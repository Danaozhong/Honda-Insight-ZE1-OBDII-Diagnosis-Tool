
#ifndef _OBD_BLE_SHARED_H_
#define _OBD_BLE_SHARED_H_

#include <vector>

#include "driver/ble/ble_interface.h"

enum OBDDataRequestType
{
	DATA_REQUEST_TYPE_OBD_DATA = 0,
	DATA_REQUEST_TYPE_OBD_ERROR_CODES = 1,
	DATA_REQUEST_TYPE_READYNESS_FLAGS = 2,
	DATA_REQUEST_NUM_OF_ITEMS
};

enum OBDDataRequestCycle
{
	DATA_REQUEST_SINGLE,
	DATA_REQUEST_CYCLIC
};


/*
 * State machine:
 * Three different communication states:
 * a) No communication
 * b) Device not ready for receiving data
 * c) Full communication, cyclic data transmission
 */
enum OBDConnectionState
{
	CONNECTION_STATE_DISCONNECTED,
	CONNECTION_STATE_INIT,
	CONNECTION_STATE_INIT_ENCRYPTION,
	CONNECTION_STATE_DATA_PROTOCOL_SYNC,
	CONNECTION_STATE_SET_INITIAL_OBD_DATA,
	CONNECTION_STATE_CONNECTED,
};


enum CommunicationState
{
	BLE_NO_COMMUNICATION,
	BLE_PASSIVE_COMMUNICATION,
	BLE_ACTIVE_COMMUNICATION
};

enum OBDDataTransmissionMode
{
    TRANSMISSION_MODE_ONLY_SELECTED_DATA_FAST = 0,
    TRANSMISSION_MODE_ALL_DATA_SLOW = 1,
	TRANSMISSION_MODE_NUM_OF_ELEMENTS = 2
};


namespace BLECommand
{
	const BLETransmitBuffer BLE_COMMAND_REQUEST_ENCRYPTION_KEY =  { 'A' };
	const BLETransmitBuffer BLE_COMMAND_REQUEST_OBD_DATA_HASH =  { 'B' };
	const BLETransmitBuffer BLE_COMMAND_PUBLISH_ENCRYPTION_KEY =  { 'C' };
	//static const std::vector<unsigned char> BLE_COMMAND_PUBLISH_SERVER_PUBLIC_ENCRYPTION_KEY =  { 0x11 };

	const BLETransmitBuffer BLE_COMMAND_OBD_DATA_HASH =  { 'D' };
	const BLETransmitBuffer BLE_COMMAND_ENCRYPTION_HANDSHAKE =  { 'E' };
	const BLETransmitBuffer BLE_COMMAND_SET_INTERESTING_OBD_DATA = { 'z' };
	const BLETransmitBuffer BLE_COMMAND_CLIENT_KEEP_ALIVE =  { 'F' };

	const BLETransmitBuffer BLE_COMMAND_CHANGE_OBD_TRANSMISSION_MODE =  { 'G' };
}


#endif /* _OBD_BLE_SHARED_H_ */
