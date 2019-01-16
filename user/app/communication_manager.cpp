
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
#include "communication_manager.hpp"
#include "midware/trace/trace.h"


namespace Application
{
	/* Definition of constants, no value required */
	const int CommunicationManager::MINIMUM_PROTOCOL_VERSION;

	CommunicationManager::CommunicationManager(std::shared_ptr<DiagnosisDeviceInterface> diagnosis_device, const OBDDataList &obd_data)
		: diagnosis_device(diagnosis_device), is_connected(false)
	{
		/* Register to the OBD diagnosis tool signal handlers, when data has changed */
	    auto local_event_handler = std::bind(&CommunicationManager::sig_obd_data_changed_handler, this, std::placeholders::_1);
		diagnosis_device->sig_obd_data_received.connect(local_event_handler);
	}

	CommunicationManager::~CommunicationManager()
	{
		// Todo remove from signal handler
	}

	int CommunicationManager::connect()
	{
		if (0 != diagnosis_device->connect())
		{
			DEBUG_PRINTF("Connecting to the diagnosis tool failed!");
			return -1;
		}

		const int protocol_version = diagnosis_device->get_protocol_version_major();
		if (CommunicationManager::MINIMUM_PROTOCOL_VERSION > protocol_version)
		{
			DEBUG_PRINTF("Protocol version mismatch, minimum required version is: " +
					std_ex::to_string(CommunicationManager::MINIMUM_PROTOCOL_VERSION) + " actual: " + std_ex::to_string(protocol_version));
			return -2;
		}

		DEBUG_PRINTF("Connected to " + diagnosis_device->get_release_label() + ", protocol version number is " +
				std_ex::to_string(protocol_version));

		this->is_connected = true;
		return 0;
	}

	int CommunicationManager::disconnect()
	{
		DEBUG_PRINTF("Disconnected from diagnosis tool!");
		diagnosis_device->disconnect();
		this->is_connected = false;
		return 0;
	}

#if 0
	void CommunicationManager::cycle_read_obd_data()
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
#endif
	void CommunicationManager::cycle_read_error_codes()
	{
		this->error_codes = this->diagnosis_device->get_error_codes();
	}


	void CommunicationManager::set_elements_of_interests(const std::vector<unsigned char> &identifier_list)
	{
		/* Lock the element of interest object */
		std::lock_guard<std::mutex> lock(this->obd_data_list_mutex);
		this->au8_elements_of_interest = identifier_list;
	}

	void CommunicationManager::sig_obd_data_changed_handler(std::vector<OBDDataList::const_iterator> ait_changed_items)
	{
		std::lock_guard<std::mutex> lock(this->obd_data_list_mutex);
		for (auto itr = ait_changed_items.begin(); itr != ait_changed_items.end(); ++itr)
		{
			if (std::find(this->ait_changed_obd_data.begin(), this->ait_changed_obd_data.end(), *itr) == this->ait_changed_obd_data.end())
			{
				this->ait_changed_obd_data.push_back(*itr);

			}
		}
	}


	std::vector<OBDDataList::const_iterator> CommunicationManager::get_changed_obd_data_of_interest() const
	{
		/* Lock the OBD data object */
		std::lock_guard<std::mutex> lock(this->obd_data_list_mutex);
		std::vector<OBDDataList::const_iterator> results;
		for (auto obd_data_element_itr = this->ait_changed_obd_data.begin(); obd_data_element_itr != this->ait_changed_obd_data.end(); ++obd_data_element_itr)
		{

			if (std::find(this->au8_elements_of_interest.begin(), this->au8_elements_of_interest.end(), (*obd_data_element_itr)->identifier) != this->au8_elements_of_interest.end())
			{
				DEBUG_PRINTF("Iterator found!");
				results.push_back(*obd_data_element_itr);
			}
		}
		return results;

	}

	std::vector<OBDDataList::const_iterator> CommunicationManager::get_changed_obd_data() const
	{
		/* Lock the OBD data object */
		std::lock_guard<std::mutex> lock(this->obd_data_list_mutex);
		return this->ait_changed_obd_data;
	}

	void CommunicationManager::clear_changed_obd_data()
	{
		/* Lock the OBD data object */
		std::lock_guard<std::mutex> lock(this->obd_data_list_mutex);
		this->ait_changed_obd_data.clear();
	}

	std::vector<OBDErrorCode> CommunicationManager::get_obd_error_codes() const
	{
		return this->error_codes;
	}
} /* namespace Application */
