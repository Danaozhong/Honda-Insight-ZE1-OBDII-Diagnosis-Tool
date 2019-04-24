
#ifndef _OBD_II_CNC_I2C_DIAGNOSIS_DEVICE_HPP_
#define _OBD_II_CNC_I2C_DIAGNOSIS_DEVICE_HPP_

/* System header files */
#include <memory>
#include <vector>
#include "thread.hpp"
#include <Wire.h>

/* Other modules */
#include "i2c_interface.hpp"

/* Own header */
#include "diagnosis_device_interface.hpp"
#include "obd_types/obd_error_code.h"


/** This is a dummy diagnosis device implementation, to be used as long as no real OBD II
 * diagnosis device is available. It just returns random OBD II values and randomly occuring
 * error codes.
 *
 * Use this dummy diagnosis device for testing transmitting data via Bluetooth BLE */
class OBDIICnCI2CDiagnosisDevice : public DiagnosisDeviceInterface
{
public:
	OBDIICnCI2CDiagnosisDevice();
	virtual ~OBDIICnCI2CDiagnosisDevice();

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

	void process_message(uint8_t *buffer, uint32_t u32_buffer_size);
private:
	/** Flag to indicate if the diagnosis device is connected */
	bool connected;

	I2CInterface m_o_i2c_interface;

	/** The currently present OBD II error codes on the vehicle */
	std::vector<OBDErrorCode> error_codes;

	/** the current read OBD II data */
	OBDDataList dummy_obd_data;

	std_ex::thread* thread_diagnosis_reader;



	void thread_diagnosis_reader_main();


	void on_i2c_receive(const uint8_t *cau8_buffer, int32_t i32_num_of_bytes);

	//void process_message(uint8_t *au8_buffer, uint32_t u32_buffer_size);

	enum I2C_HEADER_BYTE {
		HEADER_STARTUP = 0x10,
		HEADER_SHUTDOWN = 0x20,
		HEADER_OBD_DATA = 0x30,
		HEADER_OBD_CONNECTION_STATUS_REPORT = 0x40
	};
};

#endif /* _OBD_II_CNC_I2C_DIAGNOSIS_DEVICE_HPP_ */
