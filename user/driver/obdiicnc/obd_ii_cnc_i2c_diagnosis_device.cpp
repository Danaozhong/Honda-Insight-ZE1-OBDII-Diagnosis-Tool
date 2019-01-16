
/* System header files */
#include "obd_ii_cnc_i2c_diagnosis_device.hpp"

#include <stdlib.h>     /* srand, rand */
#include <memory>
#include <chrono>
#include <string>


/* Own header files */
#include "app/shared/obd_parameters.hpp"
#include "midware/trace/trace.h"
#include "midware/threads/cyclic_thread.hpp"



#define __OBD_II_I2C_BUFFER_SIZE (100)


OBDIICnCI2CDiagnosisDevice::OBDIICnCI2CDiagnosisDevice()
	: connected(false), m_o_i2c_interface(I2C_FIRST, MODE_SLAVE, 0xE)
{
	OBDataListHelper::clone(obd_ii_diagnosis_data, this->dummy_obd_data);

	//ThreadingHelper::set_default_stack_size(0x4000u);

	this->thread_diagnosis_reader = new std_ex::thread(&OBDIICnCI2CDiagnosisDevice::thread_diagnosis_reader_main, this);
}

OBDIICnCI2CDiagnosisDevice::~OBDIICnCI2CDiagnosisDevice()
{
	if (nullptr != this->thread_diagnosis_reader)
	{
		this->thread_diagnosis_reader->join();
		delete(this->thread_diagnosis_reader);
	}
}


int OBDIICnCI2CDiagnosisDevice::connect()
{
	return 0;

}
int OBDIICnCI2CDiagnosisDevice::disconnect()
{
	return 0;
}

std::string OBDIICnCI2CDiagnosisDevice::get_release_label() const
{
	return "obd_ii_cnc_i2c_diagnosis_device_v00.01.00";
}

int OBDIICnCI2CDiagnosisDevice::get_protocol_version_major() const
{
	return 1;
}

int OBDIICnCI2CDiagnosisDevice::get_protocol_version_minor() const
{
	return 0;
}

DiagnosisDeviceConnectionState OBDIICnCI2CDiagnosisDevice::get_communication_state() const
{
#if 0
	if (this->connected)
	{
		return DiagnosisDeviceConnectionState::DIAGNOSIS_DEVICE_COMMUNICATION_OK;
	}
#endif
	return DiagnosisDeviceConnectionState::DIAGNOSIS_DEVICE_COMMUNICATION_OK;
}

const OBDDataList& OBDIICnCI2CDiagnosisDevice::get_obd_data() const
{
	return this->dummy_obd_data;
}

std::vector<OBDErrorCode> OBDIICnCI2CDiagnosisDevice::get_error_codes() const
{
	return this->error_codes;
}

int OBDIICnCI2CDiagnosisDevice::clear_error_code(const OBDErrorCode& code)
{
	return 0;
}

#if 0
void OBDIICnCI2CDiagnosisDevice::update_data()
{
	std::vector<OBDDataList::const_iterator> a_changed_obd_data;

	// TODO thread protection

	/* Pretend to read some data */
	for (auto itr = this->dummy_obd_data.begin(); itr != this->dummy_obd_data.end(); ++itr)
	{
		if (rand() % 4 != 0)
		{
			continue;
		}

		float total_range = itr->max - itr->min;

		float delta_value = static_cast<float>(rand() % 100) * total_range / 1000.0f;

		if (rand() % 2 == 1)
		{
			itr->value_f += delta_value;
		}
		else
		{
			itr->value_f -= delta_value;
		}

		/* Accept boundaries */
		itr->value_f = std::min(itr->value_f, itr->max);
		itr->value_f = std::max(itr->value_f, itr->min);

		a_changed_obd_data.push_back(itr);
	}

	if (a_changed_obd_data.size() != 0)
	{
		// OBD values have changed, send signal to application!
		this->sig_obd_data_received(a_changed_obd_data);

	}

	// TODO Send a signal here as fell for the OBD error codes.
	if (rand() % 100 != 0)
	{
		// Add an error code;
		int i32_error_code = rand() % 1000;
		//this->error_codes.push_back(OBDErrorCode(i32_error_code, "Dummy error code"));
	}
}
#endif


void OBDIICnCI2CDiagnosisDevice::process_message(uint8_t *buffer, size_t buffer_size)
{
	if (buffer_size == 0)
	{
		DEBUG_PRINTF("Received invalid message: length 0");
		return;
	}

	const size_t payload_length = buffer_size - 1u;

	auto process_obd_data_message = [&]() -> int
		{
			// packet payload length should be x*6 bytes
			const size_t num_of_packets = payload_length / 6;
			if (payload_length % 6 != 0)
			{
				DEBUG_PRINTF("Data size does not fix x*6 bytes!");
				return -1;
			}

			uint8_t* buffer_ptr = buffer + 1;

			for (size_t i = 0; i != num_of_packets; ++i)
			{
				uint16_t u16_obd_code = *reinterpret_cast<uint16_t*>(buffer_ptr);
				uint32_t u32_obd_value = *reinterpret_cast<uint16_t*>(buffer_ptr + 2);
				buffer_ptr += 6;
				DEBUG_PRINTF("Received value for OBD code " + std_ex::to_string(u16_obd_code) + ", value is: " + std_ex::to_string(u32_obd_value));
			}
			return 0;
		};


	switch(static_cast<I2C_HEADER_BYTE>(buffer[0]))
	{
	case HEADER_STARTUP:
		DEBUG_PRINTF("Startup header received!");
		break;
	case HEADER_SHUTDOWN:
		DEBUG_PRINTF("Shutdown header received!");
		break;
	case HEADER_OBD_DATA:
		process_obd_data_message();
		break;
	default:
		DEBUG_PRINTF("Invalid header received: " + std_ex::to_string(buffer[0]));
		break;

	}
}

void OBDIICnCI2CDiagnosisDevice::thread_diagnosis_reader_main()
{
	/* This thread does the reading of the OBD data */
	uint8_t buffer[__OBD_II_I2C_BUFFER_SIZE] = { 0 };
	while(true)
	{
		//uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		//DEBUG_PRINTF("Current stack is: " + helper::to_string(uxHighWaterMark) + " bytes.");
		size_t buffer_size = __OBD_II_I2C_BUFFER_SIZE;

		if (0 == m_o_i2c_interface.slave_read(buffer, buffer_size, std::chrono::milliseconds(10)))
		{
			// Successfully received data

		}

		//this->update_data();
		//std_ex::sleep_for(std::chrono::milliseconds(100));
	}
}
