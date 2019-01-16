#ifndef _APP_DIAGNOSIS_READER_HPP_
#define _APP_DIAGNOSIS_READER_HPP_

/* System header */
#include <string>
#include <vector>

#include <thread>
#include <atomic>
#include <mutex>

/* Own header files */

#include "app/ble_communication/obd_ble_interface.h"
#include "app/shared/obd_parameters.hpp"

#include "driver/ble/ble_interface.h"
#include "driver/obdiicnc/diagnosis_device_interface.hpp"

#include "obd_types/obd_data.h"
#include "obd_types/obd_error_code.h"




namespace Application
{
	/**
	 * Wrapper class for an OBDII interface, providing simplified access by filtering the
	 * OBDII data. This filtering is done to reduce the data size sent via BLE.
	 * The data is filtered by
	 * 1) only take the OBD values into account whose values have changed since the last call
	 * 2) only read the values that the client is interested in (saves again BLE bandwith)
	 *
	 * TODO Clemens This class is super ugly and not easy to understand, refactor at some point
	 */
	class CommunicationManager
	{
	public:
		static const int MINIMUM_PROTOCOL_VERSION = 1;

		CommunicationManager(std::shared_ptr<DiagnosisDeviceInterface> diagnosis_device, const OBDDataList &obd_data);
		~CommunicationManager();


		int connect();
		int disconnect();

		void reset();

		//void cycle_read_obd_data();
		void cycle_read_error_codes();

		void set_elements_of_interests(const std::vector<unsigned char> &identifier_list);

		void sig_obd_data_changed_handler(std::vector<OBDDataList::const_iterator> changed_items);

		std::vector<OBDDataList::const_iterator> get_changed_obd_data_of_interest() const;

		std::vector<OBDDataList::const_iterator> get_changed_obd_data() const;
		void clear_changed_obd_data();
		
		std::vector<OBDErrorCode> get_obd_error_codes() const;
		

	private:
		std::shared_ptr<DiagnosisDeviceInterface> diagnosis_device;


		bool is_connected;

		mutable std::mutex obd_data_list_mutex;
		/* Maintain to lists of OBD data, to calculate exactly what has changed (save bandwith)*/
		//OBDDataList obd_data_list_current;
		//OBDDataList obd_data_list_prev;

		std::vector<OBDDataList::const_iterator> ait_changed_obd_data;

		std::vector<OBDErrorCode> error_codes;
		std::vector<unsigned char> au8_elements_of_interest;
	};
} /* namespace Application */


#endif /* _APP_DIAGNOSIS_READER_H_ */
