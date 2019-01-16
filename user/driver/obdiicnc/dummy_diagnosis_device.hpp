
#ifndef _OBD_II_CNC_DUMMY_DIAGNOSIS_DEVICE_HPP_
#define _OBD_II_CNC_DUMMY_DIAGNOSIS_DEVICE_HPP_

/* System header files */
#include <memory>
#include <vector>
#include "thread.hpp"

/* Own header */
#include "diagnosis_device_interface.hpp"
#include "obd_types/obd_error_code.h"


/** This is a dummy diagnosis device implementation, to be used as long as no real OBD II
 * diagnosis device is available. It just returns random OBD II values and randomly occuring
 * error codes.
 *
 * Use this dummy diagnosis device for testing transmitting data via Bluetooth BLE */
class DummyDiagnosisDevice : public DiagnosisDeviceInterface
{
public:
	DummyDiagnosisDevice();
	virtual ~DummyDiagnosisDevice();

	//virtual void start_acquisition();
	//virtual void stop_data_acquisition();

	virtual int connect();
	virtual int disconnect();

	virtual std::string get_release_label() const;

	virtual int get_protocol_version_major() const;
	virtual int get_protocol_version_minor() const;

	virtual DiagnosisDeviceConnectionState get_communication_state() const;

	virtual const OBDDataList& get_obd_data() const;

	virtual std::vector<OBDErrorCode> get_error_codes() const;

	virtual int clear_error_code(const OBDErrorCode& code);

private:
	/** Flag to indicate if the diagnosis device is connected */
	bool connected;

	/** The currently present OBD II error codes on the vehicle */
	std::vector<OBDErrorCode> error_codes;

	/** the current read OBD II data */
	OBDDataList dummy_obd_data;

	std_ex::thread* thread_diagnosis_reader;

	void update_data();

	void thread_diagnosis_reader_main();
};

#endif /* _OBD_II_CNC_DUMMY_DIAGNOSIS_DEVICE_HPP_ */
