#include "i2c_interface.hpp"

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"


#define DATA_LENGTH 512                  /*!< Data buffer length of test buffer */
#define RW_TEST_LENGTH 128               /*!< Data length for r/w test, [0,DATA_LENGTH] */
#define DELAY_TIME_BETWEEN_ITEMS_MS 1000 /*!< delay time between different test items */

#define I2C_SLAVE_SCL_IO 12               /*!< gpio number for i2c slave clock */
#define I2C_SLAVE_SDA_IO 13               /*!< gpio number for i2c slave data */
#define I2C_SLAVE_NUM I2C_NUMBER(CONFIG_I2C_SLAVE_PORT_NUM) /*!< I2C port number for slave dev */
#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave rx buffer size */

#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA               /*!< gpio number for I2C master data  */
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


I2CInterface::I2CInterface(I2CHardwareInterface interface, I2CMode mode, uint16_t u16_address)
{
	this->mode = mode;
}

I2CInterface::~I2CInterface()
{}



int I2CInterface::slave_read(uint8_t* au8_buffer, size_t &max_size,  std::chrono::milliseconds u32_wait_time)
{
	// Todo check that we are indeed slave
	int ret_val = i2c_slave_read_buffer(m_en_i2c_port, au8_buffer, max_size, u32_wait_time.count() / portTICK_RATE_MS);
	if (ret_val < 0)
	{
		// error case
		return ret_val;
	}

	// good case, return the numbers of bytes read
	max_size = static_cast<size_t>(ret_val);
	return 0;
}


esp_err_t I2CInterface::i2c_init()

{
	esp_err_t result;

	i2c_config_t i2c_configuration;
	i2c_configuration.sda_io_num = GPIO_NUM_32;
	i2c_configuration.sda_pullup_en = GPIO_PULLUP_ENABLE;
	i2c_configuration.scl_io_num = GPIO_NUM_33;
	i2c_configuration.scl_pullup_en = GPIO_PULLUP_ENABLE;
	i2c_configuration.mode = m_en_i2c_mode;
	i2c_configuration.slave.addr_10bit_en = 0;
	i2c_configuration.slave.slave_addr = m_u16_address;
	i2c_param_config(m_en_i2c_port, &i2c_configuration);
	return i2c_driver_install(m_en_i2c_port, i2c_configuration.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}


i2c_port_t ArduinoI2C::en_translate_interface(I2CHardwareInterface en_interface)
{
	if (en_interface == I2C_FIRST)
	{
		return I2C_NUM_0;
	}
	return I2C_NUM_1;
}

i2c_mode_t ArduinoI2C::en_translate_mode(I2CMode en_mode)
{
	if (en_mode == MODE_MASTER)
	{
		return I2C_MODE_MASTER;
	}
	return I2C_MODE_SLAVE;
}


