

#include "obd_ii_cnc_i2c_dummy_sender.hpp"
#include "cyclic_thread.hpp"

OBDIICnCI2CDummySender::OBDIICnCI2CDummySender()
	: m_o_i2c_interface(0xC)
{
	m_o_i2c_interface.begin(16, 17, 100000); // SDA is pin 16, SCL pin 17, 100kHz
	this->thread_diagnosis_reader = new std_ex::thread(&OBDIICnCI2CDummySender::thread_dummy_sender_main, this);
}

OBDIICnCI2CDummySender::~OBDIICnCI2CDummySender()
{
	bo_terminate = true;

	this->thread_diagnosis_reader->join();
	delete this->thread_diagnosis_reader;
}

void OBDIICnCI2CDummySender::thread_dummy_sender_main()
{
	while(this-bo_terminate == false)
	{
		uint8_t u8Header = 0x30;
		uint16_t u16OBDIICode = 22u;
		uint32_t u32Value = 170u;

		uint8_t au8Buffer[7] = { 0 };
		memcpy(au8Buffer, &u8Header, 1);
		memcpy(au8Buffer + 1, &u16OBDIICode, 2);
		memcpy(au8Buffer + 3, &u32Value, 4);

		m_o_i2c_interface.beginTransmission(0xE); // Transmit to device 0xE
		m_o_i2c_interface.write(au8Buffer, 7);
		m_o_i2c_interface.endTransmission();

		DEBUG_PRINTF("Data sent!");
		std_ex::sleep_for(std::chrono::milliseconds(1000));
	}
}
