#ifndef _DIAGNOSIS_READER_APPLICATION_HPP_
#define _DIAGNOSIS_READER_APPLICATION_HPP_

/* Own header files */
#include "communication_manager.hpp"
#include "thread_repository.hpp"

namespace Application
{
	/** This is the main class holding everything together.
	 * It manages all the worker threads that are spawned during runtime,
	 * owns the diagnosis interface object, and is responsible for creating / closing the
	 * BLE interface.
	 *  */
	class DiagnosisReaderApplication
	{
	public:
		DiagnosisReaderApplication();
		~DiagnosisReaderApplication();

		void loop();
	private:
		/** The OBDII device used. */
		std::shared_ptr<DiagnosisDeviceInterface> obd_diagnosis_device;

	    /** Helper object to filter / manage the OBD II data read */
		std::shared_ptr<CommunicationManager> diagnosis_reader;

		/** The BLE interface */
		std::shared_ptr<BLEOBDDataServer> ble_server;

		/** The thread manager, containing all threads running at the moment */
		ThreadRepository thread_repository;
	};


}

#endif /* _DIAGNOSIS_READER_APPLICATION_HPP_ */
