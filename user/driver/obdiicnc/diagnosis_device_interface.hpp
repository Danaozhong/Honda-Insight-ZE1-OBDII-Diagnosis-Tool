
#ifndef _OBD_II_CNC_GENERIC_INTERFACE_H_
#define _OBD_II_CNC_GENERIC_INTERFACE_H_

/* System header files */
#include <vector>
#include "event_handler.h"

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

	/** Get the release label of the diagnosis reader */
	virtual std::string get_release_label() const = 0;

	/** Get a version of the communication protocol */
	virtual int get_protocol_version_major() const = 0;
	virtual int get_protocol_version_minor() const = 0;

	/** Returns the current state of the OBD II device */
	virtual DiagnosisDeviceConnectionState get_communication_state() const = 0;


	/* Signal triggered when OBD data is received */
	boost::signals2::signal<std::vector<OBDDataList::const_iterator>> sig_obd_data_received;

	/* Signal triggered when OBD error code is received */
	boost::signals2::signal<std::vector<uint16_t>> sig_obd_error_code_received;


	/** Connect to the OBD II diagnosis device. */
	virtual int connect() = 0;

	/** Disconnect from the OBD II diagnosis device. */
	virtual int disconnect() = 0;


	/** performs a read cycle */
	//virtual void update_data();

	/** Returns the OBD II data */
	virtual const OBDDataList& get_obd_data() const = 0;

	/** Returns the OBD II error codes */
	virtual std::vector<OBDErrorCode> get_error_codes() const = 0;

	/** Clears an OBD II error code */
	virtual int clear_error_code(const OBDErrorCode& code) = 0;
};

#endif /* _OBD_II_CNC_GENERIC_INTERFACE_H_ */
