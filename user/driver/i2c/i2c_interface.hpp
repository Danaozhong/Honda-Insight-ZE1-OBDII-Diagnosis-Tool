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

/** Which I2C interface to use */
enum I2CHardwareInterface
{
	I2C_FIRST,
	I2C_SECOND,
	I2C_THIRD

};

/** Master/Slave configuration */
enum I2CMode
{
	MODE_MASTER,
	MODE_SLAVE
};

/** \brief Class to access I2C communication. Only first interface implemented, still under development. */
class I2CInterface
{
public:

	I2CInterface(I2CHardwareInterface interface, I2CMode mode, uint16_t u16_address);

	~I2CInterface();




	int slave_read(uint8_t* au8_buffer, size_t &max_size, std::chrono::milliseconds u32_wait_time);

	//void write_register(const std::vector<unsigned char> &au8_buffer);

	//void read_register(const unsigned char cu8_register, char* buffer, unsigned int u32_buffer_size);

	//slave_read_buffer

private:
	I2CMode mode;

	i2c_port_t m_en_i2c_port;
	i2c_mode_t m_en_i2c_mode;
	uint16_t m_u16_address;

	/* Platform specific implementation */
	esp_err_t i2c_init();

};

namespace ArduinoI2C
{
	i2c_port_t en_translate_interface(I2CHardwareInterface en_interface);
	i2c_mode_t en_translate_mode(I2CMode en_mode);
}



#endif /* USER_DRIVER_I2C_I2C_INTERFACE_HPP_ */
