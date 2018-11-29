/*
 * i2c_interface.hpp
 *
 *  Created on: 28.11.2018
 *      Author: Clemens
 */

#ifndef USER_DRIVER_I2C_I2C_INTERFACE_HPP_
#define USER_DRIVER_I2C_I2C_INTERFACE_HPP_


#include <vector>

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

	I2CInterface(I2CHardwareInterface interface, I2CMode mode);

	~I2CInterface();

	void write_register(const std::vector<unsigned char> &au8_buffer);

	void read_register(const unsigned char cu8_register, char* buffer, unsigned int u32_buffer_size);

private:
	I2CMode mode;
};



#endif /* USER_DRIVER_I2C_I2C_INTERFACE_HPP_ */
