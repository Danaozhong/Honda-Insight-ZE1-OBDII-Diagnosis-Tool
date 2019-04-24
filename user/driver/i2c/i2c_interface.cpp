#include "i2c_interface.hpp"

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include <Wire.h>


#define DATA_LENGTH 512                  /*!< Data buffer length of test buffer */
#define RW_TEST_LENGTH 128               /*!< Data length for r/w test, [0,DATA_LENGTH] */
#define DELAY_TIME_BETWEEN_ITEMS_MS 1000 /*!< delay time between different test items */

#define I2C_SLAVE_SCL_IO 12               /*!< gpio number for i2c slave clock */
#define I2C_SLAVE_SDA_IO 13               /*!< gpio number for i2c slave data */
#define I2C_SLAVE_NUM I2C_NUMBER(CONFIG_I2C_SLAVE_PORT_NUM) /*!< I2C port number for slave dev */
#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave rx buffer size */


#define I2C_FIRST_GPIO_SDA GPIO_NUM_21
#define I2C_FIRST_GPIO_SCL GPIO_NUM_22

#define I2C_SECOND_GPIO_SDA GPIO_NUM_32
#define I2C_SECOND_GPIO_SCL GPIO_NUM_33


#define I2C_EXAMPLE_MASTER_SCL_IO          GPIO_NUM_5       // gpio number for I2C master clock
#define I2C_EXAMPLE_MASTER_SDA_IO          GPIO_NUM_4       // gpio number for I2C master data
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0        // I2C port number for master dev
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0                // I2C master do not need buffer
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0                // I2C master do not need buffer
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000           	// I2C master clock frequency 100000


//#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
//#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_MASTER_PORT_NUM) /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

#define BH1750_SENSOR_ADDR CONFIG_BH1750_ADDR   /*!< slave address for BH1750 sensor */
#define BH1750_CMD_START CONFIG_BH1750_OPMODE   /*!< Operation mode */
//#define ESP_SLAVE_ADDR CONFIG_I2C_SLAVE_ADDRESS /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1 /*!< I2C nack value */


I2CInterface::I2CInterface(I2CMode mode, uint16_t u16_address)
	: m_u32_clock_frequency(100000), m_mode(mode), po_read_thread(nullptr), pv_current_command_handle(nullptr),
	  u32_pin_scl(0u), u32_pin_sda(0u)
{

	this->m_u16_address = u16_address;

}

I2CInterface::~I2CInterface()
{
	this->bo_stop_thread = true;
	if (this->po_read_thread != nullptr)
	{
		this->po_read_thread->join();
	}

}

bool I2CInterface::begin(uint32_t sda, uint32_t scl, uint32_t frequency)
{
	this->u32_pin_scl = scl;
	this->u32_pin_sda= sda;

	if (MODE_MASTER == this->m_mode)
	{
		// Initialize as MASTER
		this->i32_i2c_init_master();

	}
	else
	{
		// Initialize as SLAVE
		this->i32_i2c_init_slave();

		// start the read thread
		this->bo_stop_thread = false;

		this->po_read_thread = std::unique_ptr<std_ex::thread>(new std_ex::thread(&I2CInterface::v_slave_read_thread_main, this));
	}
}

void I2CInterface::onRequest( i2c_user_on_request p_func )
{
	// TODO
}

void I2CInterface::onReceive( i32_user_on_receive p_func )
{
	this->pf_receive_callback = p_func;
}

void I2CInterface::beginTransmission(uint16_t u16_address)
{
	if (pv_current_command_handle != nullptr)
	{
		return;
	}
	if (MODE_MASTER == this->m_mode)
	{
		// Write as master
	    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	    i2c_master_start(cmd);
	    i2c_master_write_byte(cmd, (u16_address << 1) | WRITE_BIT, ACK_CHECK_EN);
	    pv_current_command_handle = cmd;
	}
	else
	{

	}
}

uint8_t I2CInterface::endTransmission(bool sendStop)
{
	if (MODE_MASTER == this->m_mode)
	{
	    i2c_master_stop(pv_current_command_handle);
	    esp_err_t ret = i2c_master_cmd_begin(this->en_get_interface(), pv_current_command_handle, 1000 / portTICK_RATE_MS);
	    i2c_cmd_link_delete(pv_current_command_handle);
	    pv_current_command_handle = nullptr;
	}
	else
	{
		// Write as slave
	}
}


//int I2CInterface::write(uint16_t u16_address, const uint8_t* au8_buffer, size_t buffer_size)
int32_t I2CInterface::write(const uint8_t* cau8Data, int32_t i32BufferLength)
{
	if (pv_current_command_handle == nullptr)
	{
		return -1;
	}
	i2c_master_write(pv_current_command_handle, const_cast<uint8_t*>(cau8Data), i32BufferLength, ACK_CHECK_EN);
	return 0;
}


int32_t I2CInterface::write(const uint8_t* cau8Data)
{
	return this->write(cau8Data, strlen(reinterpret_cast<const char*>(cau8Data)));
}


esp_err_t I2CInterface::i32_i2c_init_master()
{
    //int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    if (0 != ArduinoI2C::do_pin_configuration(this->u32_pin_scl, this->u32_pin_sda, conf))
    {
    	return -1;
    }
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = m_u32_clock_frequency;
    i2c_param_config(en_get_interface(), &conf);
    return i2c_driver_install(en_get_interface(), conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}
esp_err_t I2CInterface::i32_i2c_init_slave()
{
	esp_err_t i32_result = -1;
	i2c_config_t i2c_configuration = i2c_config_t();

	i2c_configuration.sda_pullup_en = GPIO_PULLUP_ENABLE;
	i2c_configuration.scl_pullup_en = GPIO_PULLUP_ENABLE;
	i2c_configuration.mode = ArduinoI2C::en_translate_mode(m_mode);
	i2c_configuration.slave.addr_10bit_en = 0;
	i2c_configuration.slave.slave_addr = m_u16_address;
	i2c_param_config(en_get_interface(), &i2c_configuration);
	return i2c_driver_install(en_get_interface(), i2c_configuration.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}


int32_t I2CInterface::i32_i2c_slave_read(uint8_t* au8_buffer, uint32_t &u32_max_size,  std::chrono::milliseconds u32_wait_time)
{
	// Todo check that we are indeed slave
	int ret_val = i2c_slave_read_buffer(this->en_get_interface(), au8_buffer, u32_max_size, u32_wait_time.count() / portTICK_RATE_MS);
	if (ret_val < 0)
	{
		// error case
		return ret_val;
	}

	// good case, return the numbers of bytes read
	u32_max_size = static_cast<int32_t>(ret_val);
	return 0;
}

i2c_port_t I2CInterface::en_get_interface() const
{
	if (this->u32_pin_scl == I2C_FIRST_GPIO_SCL)
	{
		return I2C_NUM_0;
	}
	return I2C_NUM_1;
}


void I2CInterface::v_slave_read_thread_main()
{
	uint8_t au8_read_buffer[I2C_BUFFER_SIZE] = { 0 };

	while(bo_stop_thread == false)
	{
		uint32_t uint32_buffer_size = I2C_BUFFER_SIZE;
		if (0 == this->i32_i2c_slave_read(au8_read_buffer, uint32_buffer_size, std::chrono::milliseconds(10)))
		{
			if (uint32_buffer_size > 0)
			{
				// Successfully received data, call the callback.
				if (pf_receive_callback != nullptr)
				{
					pf_receive_callback(au8_read_buffer, uint32_buffer_size);
				}
			}

		}
	}
}



i2c_mode_t ArduinoI2C::en_translate_mode(I2CMode en_mode)
{
	if (en_mode == MODE_MASTER)
	{
		return I2C_MODE_MASTER;
	}
	return I2C_MODE_SLAVE;
}



int32_t ArduinoI2C::do_pin_configuration(uint32_t u32PinSCL, uint32_t u32PinSDA, i2c_config_t &st_i2c_config)
{
	int32_t i32_result = -1;
	switch (u32PinSCL)
	{
	case 22:
		st_i2c_config.sda_io_num = I2C_FIRST_GPIO_SDA;
		st_i2c_config.scl_io_num = I2C_FIRST_GPIO_SCL;
		i32_result = 0;
		break;
	case 33:
		st_i2c_config.sda_io_num = I2C_SECOND_GPIO_SDA;
		st_i2c_config.scl_io_num = I2C_SECOND_GPIO_SCL;
		i32_result = 0;
		break;
	default:
		/* This channel is not supported */
		break;
	}
	return i32_result;
}

