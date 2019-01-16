
/* System header files */
#include "dummy_diagnosis_device.hpp"

#include <stdlib.h>     /* srand, rand */
#include <memory>
#include <chrono>
#include <thread>
#include <string>

/* Own header files */
#include "app/shared/obd_parameters.hpp"
#include "midware/trace/trace.h"
#include "midware/threads/cyclic_thread.hpp"

DiagnosisDeviceInterface::~DiagnosisDeviceInterface()
{
	/* Dummy destructor */
}

DummyDiagnosisDevice::DummyDiagnosisDevice()
	: connected(false)
{
	OBDataListHelper::clone(obd_ii_diagnosis_data, this->dummy_obd_data);

	//ThreadingHelper::set_default_stack_size(0x4000u);

	this->thread_diagnosis_reader = new std_ex::thread(&DummyDiagnosisDevice::thread_diagnosis_reader_main, this);
}

DummyDiagnosisDevice::~DummyDiagnosisDevice()
{
	if (nullptr != this->thread_diagnosis_reader)
	{
		this->thread_diagnosis_reader->join();
		delete(this->thread_diagnosis_reader);
	}
}


int DummyDiagnosisDevice::connect()
{
	if (this->connected == true)
	{
		return -1;
	}
	this->connected = true;
	return 0;

}
int DummyDiagnosisDevice::disconnect()
{
	if (this->connected == false)
	{
		return -1;
	}
	this->connected = false;
	return 0;
}

std::string DummyDiagnosisDevice::get_release_label() const
{
	return "dummydevice_v01.00.pre";
}

int DummyDiagnosisDevice::get_protocol_version_major() const
{
	return 1;
}

int DummyDiagnosisDevice::get_protocol_version_minor() const
{
	return 0;
}

DiagnosisDeviceConnectionState DummyDiagnosisDevice::get_communication_state() const
{
	if (this->connected)
	{
		return DiagnosisDeviceConnectionState::DIAGNOSIS_DEVICE_COMMUNICATION_OK;
	}
	return DiagnosisDeviceConnectionState::DIAGNOSIS_DEVICE_DISCONNECTED;
}

const OBDDataList& DummyDiagnosisDevice::get_obd_data() const
{
	return this->dummy_obd_data;
}

std::vector<OBDErrorCode> DummyDiagnosisDevice::get_error_codes() const
{
	return this->error_codes;
}

int DummyDiagnosisDevice::clear_error_code(const OBDErrorCode& code)
{
	return 0;
}


void DummyDiagnosisDevice::update_data()
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


void DummyDiagnosisDevice::thread_diagnosis_reader_main()
{
	/* This thread does the reading of the OBD data */
	while(true)
	{
		//uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		//DEBUG_PRINTF("Current stack is: " + helper::to_string(uxHighWaterMark) + " bytes.");

		this->update_data();
		std_ex::sleep_for(std::chrono::milliseconds(100));
	}
}
