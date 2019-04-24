/*
 * i2c_interface.hpp
 *
 *  Created on: 28.11.2018
 *      Author: Clemens
 */


// Try using boost::asio for this

#ifndef USER_DRIVER_I2C_I2C_INTERFACE_HPP_
#define USER_DRIVER_I2C_I2C_INTERFACE_HPP_

#include "stdint.h"
#include <vector>
#include "driver/i2c.h"
#include <chrono>
#include <functional>
#include <thread.hpp>


/** Master/Slave configuration */
enum I2CMode
{
	MODE_MASTER,
	MODE_SLAVE
};

typedef std::function<void(void)> i2c_user_on_request;
typedef std::function<void(const uint8_t*, int32_t)> i32_user_on_receive;


#define I2C_BUFFER_SIZE 500


/** \brief Class to access I2C communication. Only first interface implemented, still under development. */
class I2CInterface
{
public:

	I2CInterface(I2CMode mode, uint16_t u16_address);

	~I2CInterface();


	bool begin(uint32_t sda=-1, uint32_t scl=-1, uint32_t frequency=0);

	void onRequest( i2c_user_on_request p_func );
	void onReceive( i32_user_on_receive p_func );


	//int slave_read(uint8_t* au8_buffer, size_t &max_size, std::chrono::milliseconds u32_wait_time);


	void beginTransmission(uint16_t u16_address);

    uint8_t endTransmission(bool sendStop);

    int32_t write(const uint8_t* cau8Data, int32_t i32BufferLength);

    int32_t write(const uint8_t* cau8Data);

private:
	const I2CMode m_mode;
	const uint32_t m_u32_clock_frequency;
	//i2c_mode_t m_en_i2c_mode;
	uint16_t m_u16_address;

	i32_user_on_receive pf_receive_callback;

	/* Platform specific implementation */
	int32_t u32_pin_scl;
	int32_t u32_pin_sda;

	esp_err_t i32_i2c_init_master();
	esp_err_t i32_i2c_init_slave();

	int32_t i32_i2c_slave_read(uint8_t* au8_buffer, uint32_t &u32_max_size,  std::chrono::milliseconds u32_wait_time);

	i2c_port_t en_get_interface() const;

	i2c_cmd_handle_t pv_current_command_handle;


	// read thread
	std::unique_ptr<std_ex::thread> po_read_thread;

	std::atomic<bool> bo_stop_thread;
	void v_slave_read_thread_main();
};

namespace ArduinoI2C
{
	i2c_mode_t en_translate_mode(I2CMode en_mode);
	int32_t do_pin_configuration(uint32_t u32PinSCL, uint32_t u32PinSD, i2c_config_t &st_i2c_config);
}



#endif /* USER_DRIVER_I2C_I2C_INTERFACE_HPP_ */
