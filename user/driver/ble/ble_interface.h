// BluetoothInterface.h

#ifndef _BLE_INTERFACE_H_
#define _BLE_INTERFACE_H_

/* System header files */
#include <vector>
#undef min
#undef max
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

/* Foreign header files */
#include "midware/events/event_handler.h"

/* Own header files */
#include "driver/ble/adapt/ble_configuration.hpp"

/** buffer data structure */
typedef std::vector<unsigned char> BLETransmitBuffer;

/* Some ARDUINO library stuff */
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

/* Forward class declation */
class BLECharacteristic;

/**
 * Class to handle the callback when a BLE device connects / disconnects
 */
class MyServerCallbacks : public BLEServerCallbacks
{
public:
	/** Are we connected to a client? */
	bool is_connected() const;

private:
	/** Called when a client (e.h. smartphone) connects to us. */
	void onConnect(BLEServer* pServer);

	/** Called when the client disconnects again. */
	void onDisconnect(BLEServer* pServer);

	/** Flag to remember the connection state */
	bool m_bo_device_connected;

};

/** Callback triggered when a BLE Characteristics receives data */
class MyCallbacks : public BLECharacteristicCallbacks
{
public:
    /** signal to be sent when the data was received completely */
	boost::signals2::signal<std::vector<unsigned char>> SIG_RECEIVED_DATA;

	void onWrite(BLECharacteristic *pCharacteristic);
private:
	std::vector<unsigned char> current_message;
};


/** Interface class to be implement a BLE characteristic (may be client or server) */
class BLEInterfaceCharacteristicBase
{
public:

	virtual ~BLEInterfaceCharacteristicBase() {};

	/** Sends data via the BLE characteristic */
    virtual int send(const std::vector<unsigned char> &buffer) = 0;

    virtual int get_index() const = 0;

    /** Each BLE interface must have an unique UUID */
    virtual const char* get_uuid() const = 0;

    /** Use this signal provided by the characteristic to register on an Rx event */
    virtual boost::signals2::signal<std::vector<unsigned char>>* get_received_data_signal() const = 0;
};

/** Class to implement a standard BLE characteristic */
class BLEInterfaceCharacteristic : public BLEInterfaceCharacteristicBase
{
public:
	BLEInterfaceCharacteristic(unsigned int index, const char uuid[BLE_UUID_LENGTH], int property, MyCallbacks* callback = NULL);

	int create_characteristic(BLEService &pService);

	virtual int send(const std::vector<unsigned char> &buffer);

	virtual int get_index() const;

	virtual const char* get_uuid() const;

	virtual boost::signals2::signal<std::vector<unsigned char>>* get_received_data_signal() const;
private:
	unsigned int index;
	char uuid[BLE_UUID_LENGTH];
	int property;
	MyCallbacks* rx_callback;
	BLECharacteristic *p_characteristic;
};

/** Class to manage a BLE interface. */
class BLEInterface
{
public:
	void init_interface();
	void send_interface(int characteristic_index, const std::vector<unsigned char> &data);

	bool is_connected() const;

	void disconnect() const;

	BLEInterfaceCharacteristic* get_characteristic(int id);
private:
	MyServerCallbacks *server_callback;

	std::string interface_public_string;
};


#endif /* _BLE_INTERFACE_H_ */

