
	/* System header */
	#if defined(ARDUINO) && ARDUINO >= 100
		#include "Arduino.h"
	#else
		#include "WProgram.h"
	#endif


#include <future>
#include <string>
#include <sstream>
#include <algorithm>

#include "../driver/obdiicnc/dummy_diagnosis_device.hpp"

#include "trace.h"
#include "diagnosis_reader.hpp"
#include "midware/trace/trace.h"


namespace Application
{
	/* Definition of constants, no value required */
	const int DiagnosisReader::MINIMUM_PROTOCOL_VERSION;

	DiagnosisReader::DiagnosisReader(std::shared_ptr<DiagnosisDeviceInterface> diagnosis_device, const OBDDataList &obd_data)
		: diagnosis_device(diagnosis_device), is_connected(false)
	{
		/* Create two clones of the attributes to be able to compare */
		OBDataListHelper::clone(obd_data, this->obd_data_list_current);
		OBDataListHelper::clone(obd_data, this->obd_data_list_prev);
	}


	int DiagnosisReader::connect()
	{
		if (0 != diagnosis_device->connect())
		{
			DEBUG_PRINTF("Connecting to the diagnosis tool failed!");
			return -1;
		}

		const int protocol_version = diagnosis_device->get_protocol_version_major();
		if (DiagnosisReader::MINIMUM_PROTOCOL_VERSION > protocol_version)
		{
			DEBUG_PRINTF("Protocol version mismatch, minimum required version is: " +
					helper::to_string(DiagnosisReader::MINIMUM_PROTOCOL_VERSION) + " actual: " + helper::to_string(protocol_version));
			return -2;
		}

		DEBUG_PRINTF("Connected to " + diagnosis_device->get_release_label() + ", protocol version number is " +
				helper::to_string(protocol_version));

		this->is_connected = true;
		return 0;
	}

	int DiagnosisReader::disconnect()
	{
		DEBUG_PRINTF("Disconnected from diagnosis tool!");
		diagnosis_device->disconnect();
		this->is_connected = false;
		return 0;
	}

	void DiagnosisReader::cycle_read_obd_data()
	{
		if (this->is_connected == false)
		{
			return;
			/* */
		}

		/* Lock the OBD data object */
		std::lock_guard<std::mutex> lock(this->obd_data_list_mutex);

		this->obd_data_list_prev = this->obd_data_list_current;
		/* Read from OBD tool */
		this->diagnosis_device->update_data();

		/* Receive the updates from the OBD tool */
		this->diagnosis_device->get_obd_data(this->obd_data_list_current);

		bool read_error = false;
	}

	void DiagnosisReader::cycle_read_error_codes()
	{
		this->error_codes = this->diagnosis_device->get_error_codes();
	}


	void DiagnosisReader::set_elements_of_interests(const std::vector<unsigned char> &identifier_list)
	{
		/* Lock the element of interest object */
		std::lock_guard<std::mutex> lock(this->obd_data_list_mutex);
		this->au8_elements_of_interest = identifier_list;
	}

	std::vector<OBDDataList::const_iterator> DiagnosisReader::get_changed_obd_data_of_interest() const
	{
		/* Lock the OBD data object */
		std::lock_guard<std::mutex> lock(this->obd_data_list_mutex);

		std::vector<OBDDataList::const_iterator> results;
		for (int i = 0; i != this->obd_data_list_current.size(); ++i)
		{

			if (std::find(this->au8_elements_of_interest.begin(), this->au8_elements_of_interest.end(), this->obd_data_list_current[i].identifier) != this->au8_elements_of_interest.end()
			&& this->obd_data_list_current[i].value_f != this->obd_data_list_prev[i].value_f)
			{
				results.push_back(this->obd_data_list_current.begin() + i);
			}
		}
		return results;

	}

	std::vector<OBDDataList::const_iterator> DiagnosisReader::get_changed_obd_data() const
	{
		/* Lock the OBD data object */
		std::lock_guard<std::mutex> lock(this->obd_data_list_mutex);

		std::vector<OBDDataList::const_iterator> results;
		for (int i = 0; i != this->obd_data_list_current.size(); ++i)
		{
			if (this->obd_data_list_current[i].value_f != this->obd_data_list_prev[i].value_f)
			{
				results.push_back(this->obd_data_list_current.begin() + i);
			}
		}
		return results;
	}

	void DiagnosisReader::clear_changed_obd_data()
	{
		/* Lock the OBD data object */
		std::lock_guard<std::mutex> lock(this->obd_data_list_mutex);

		this->obd_data_list_prev = this->obd_data_list_current;
	}



	std::vector<OBDErrorCode> DiagnosisReader::get_obd_error_codes() const
	{
		return this->error_codes;
	}
} /* namespace Application */
