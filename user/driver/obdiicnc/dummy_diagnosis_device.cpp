
/* System header files */
#include "dummy_diagnosis_device.hpp"

#include <stdlib.h>     /* srand, rand */
#include <memory>

/* Own header files */
#include "app/shared/obd_parameters.hpp"
#include "midware/trace/trace.h"

DiagnosisDeviceInterface::~DiagnosisDeviceInterface()
{
	/* Dummy destructor */
}

DummyDiagnosisDevice::DummyDiagnosisDevice()
{
	OBDataListHelper::clone(obd_ii_diagnosis_data, this->dummy_obd_data);
}

DummyDiagnosisDevice::~DummyDiagnosisDevice()
{
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

void DummyDiagnosisDevice::update_data()
{
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

	}

	if (rand() % 100 != 0)
	{
		// Add an error code;
		int i32_error_code = rand() % 1000;
		//this->error_codes.push_back(OBDErrorCode(i32_error_code, "Dummy error code"));
	}
}

int DummyDiagnosisDevice::get_obd_data(OBDDataList &data_array)
{
	int index = 0;
	data_array.clear();

	for (auto itr = dummy_obd_data.begin(); itr != dummy_obd_data.end(); ++itr)
	{
		data_array.push_back(*itr);
		//*itr = this->dummy_obd_data[index];
		//++index;
	}
	return 0;
}

std::vector<OBDErrorCode> DummyDiagnosisDevice::get_error_codes()
{
	return this->error_codes;
}

int DummyDiagnosisDevice::clear_error_code(const OBDErrorCode& code)
{
	return 0;
}
