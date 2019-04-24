
#ifndef _OBD_II_CNC_I2C_DUMMY_SENDER_HPP_
#define _OBD_II_CNC_I2C_DUMMY_SENDER_HPP_

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
class OBDIICnCI2CDummySender
{
public:
	OBDIICnCI2CDummySender();
	virtual ~OBDIICnCI2CDummySender();

private:

	TwoWire m_o_i2c_interface;

	std_ex::thread* thread_diagnosis_reader;

	std::atomic<bool> bo_terminate;

	void thread_dummy_sender_main();

	enum I2C_HEADER_BYTE {
		HEADER_STARTUP = 0x10,
		HEADER_SHUTDOWN = 0x20,
		HEADER_OBD_DATA = 0x30,
		HEADER_OBD_CONNECTION_STATUS_REPORT = 0x40
	};
};

#endif /* _OBD_II_CNC_I2C_DUMMY_SENDER_HPP_ */
