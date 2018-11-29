#include "i2c_interface.hpp"

#include "driver/i2c.h"



I2CInterface::I2CInterface(I2CHardwareInterface interface, I2CMode mode)
{
	this->mode = mode;
}

I2CInterface::~I2CInterface()
{}

void I2CInterface::write_register(const std::vector<unsigned char> &au8_buffer)
{
}

void I2CInterface::read_register(const unsigned char cu8_register, char* buffer, unsigned int u32_buffer_size)

{
	if (this->mode == MODE_MASTER)
	{


	}
	else
	{


	}

}
