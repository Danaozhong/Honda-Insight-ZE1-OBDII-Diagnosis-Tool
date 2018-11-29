/*
 * This file contains the BLE configuration, that is supposed to be shared between this
 * application and the App running on the smart phone.
 */
#ifndef _BLE_CONFIGURATION_HPP_
#define _BLE_CONFIGURATION_HPP_

/* System header */
#include <string>
#include <vector>

/** This is something related to BLE, don't exactly know what it does */
extern const std::string SERVICE_UUID;

class BLEInterfaceCharacteristic;

typedef std::vector<BLEInterfaceCharacteristic> BLECharacteristicList;

/** A list of all the characteristics used in this BLE connection */
extern BLECharacteristicList ble_interface_characteristics;

/** BLE characteristic for receiving commands from the BLE counterpart (e.g. smartphone) */
extern const unsigned int BLE_CHARACTERISTIC_ID_RECEIVE_COMMANDS;

/** BLE characteristic for sending commands to the the BLE counterpart (e.g. smartphone) */
extern const unsigned int BLE_CHARACTERISTIC_ID_SEND_COMMANDS;

/** BLE characteristic only used to cyclically transmit OBD II data (e.g. voltage, amps, soc...) */
extern const unsigned int BLE_CHARACTERISTIC_ID_SEND_OBD_DATA;

/** BLE characteristic only used to cyclically transmit the read OBD error codes */
extern const unsigned int BLE_CHARACTERISTIC_ID_SEND_OBD_ERROR_CODES;

#define BLE_UUID_LENGTH (40u)

/** Defines the maximum length in bytes of each BLE transfer (by default 20). Each message will be
 * split up into x * BLE_MAX_PACKET_SIZE messages.
 */
#define BLE_MAX_PACKET_SIZE (20u)

/** Key to identify where a BLE message starts */
extern const unsigned char BLE_PACKET_BEGIN;

/** Key to identify where a BLE message is split up */
extern const unsigned char BLE_PACKET_MIDDLE;

/** Key to identify where a BLE message is sent */
extern const unsigned char BLE_PACKET_END;

/** The SSID (name) of the BLE interface. This will show up in your smartphone when you try to connect */
extern const std::string BLE_DEVICE_SSID;

#endif

