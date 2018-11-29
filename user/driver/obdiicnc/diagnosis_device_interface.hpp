
#ifndef _OBD_II_CNC_GENERIC_INTERFACE_H_
#define _OBD_II_CNC_GENERIC_INTERFACE_H_

/* System header files */
#include <vector>

/* Own header files */
#include "obd_types/obd_data.h"
#include "obd_types/obd_error_code.h"

/** Enumeration defining the connection states for an OBD II diagnosis device */
enum DiagnosisDeviceConnectionState
{
	DIAGNOSIS_DEVICE_COMMUNICATION_OK,	/** Communication to the diagnosis device is successfull */
	DIAGNOSIS_DEVICE_DISCONNECTED,		/** No diagnosis device connected! */
	DIAGNOSIS_DEVICE_INITIALIZING,		/** Evaluating the diagnosis device abilities */
	DIAGNOSIS_DEVICE_NO_SIGNAL,			/** Diagnosis device does not read any data (e.g. not connected to vehicle) */
};

/**
 * Interface defining the base interface of an OBD II diagnosis device.
 */
class DiagnosisDeviceInterface
{
public:
	/** Destructor */
	virtual ~DiagnosisDeviceInterface() = 0;

	/** Connect to the OBD II diagnosis device. */
	virtual int connect();

	/** Disconnect from the OBD II diagnosis device. */
	virtual int disconnect();

	/** Get the release label of the diagnosis reader */
	virtual std::string get_release_label() const;

	/** Get a version of the communication protocol */
	virtual int get_protocol_version_major() const;
	virtual int get_protocol_version_minor() const;

	/** Returns the current state of the OBD II device */
	virtual DiagnosisDeviceConnectionState get_communication_state() const;

	/** performs a read cycle */
	virtual void update_data();

	/** Returns the OBD II data read out with update_data() */
	virtual int get_obd_data(OBDDataList &data_array);

	/** Returns the OBD II error codes */
	virtual std::vector<OBDErrorCode> get_error_codes();

	/** Clears an OBD II error code */
	virtual int clear_error_code(const OBDErrorCode& code);
};

#endif /* _OBD_II_CNC_GENERIC_INTERFACE_H_ */
