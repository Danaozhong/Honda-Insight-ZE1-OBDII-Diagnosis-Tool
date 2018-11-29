// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _SIMPLE_BL2E2_H_
#define _SIMPLE_BL2E2_H_

/* System header */
#include <atomic>
#include <string>
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "../../midware/threads/cyclic_thread.hpp"
#include "driver/ble/ble_interface.h"
/* Foreign header */
#include "app/shared/obd_ble_shared.hpp"
#include "app/shared/obd_parameters.hpp"
#include "app/ble_communication/ble_request_queue.hpp"



class OBDDataCommunicatorStateMachine
{
public:
	/* TODO Put some signals here to transition between the states */
};



#include <memory>
namespace Application
{
class DiagnosisReader;
}


class GenericOBDDataServer
{
public:
	GenericOBDDataServer(std::shared_ptr<Application::DiagnosisReader> diagnosis_reader);

	virtual ~GenericOBDDataServer();

	virtual void startup() = 0;
	virtual void shutdown() = 0;

	//virtual void on_client_connect(const std::string &client_id);
	//virtual void on_client_disconnect(const std::string &client_id);

	virtual int get_protocol_version_major() const;
	virtual int get_protocol_version_minor() const;

	/* Generates a hash out of the data table to compare it with the hash of the other client */
	virtual int get_obd_data_hash(char *buffer, size_t buffer_size) const;


	virtual void set_transmission_request(OBDDataRequestType type) = 0;
	virtual void clear_transmission_request(OBDDataRequestType type) = 0;

	std::shared_ptr<Application::DiagnosisReader> get_diagnosis_reader();
	/*
	 * Init
	 * Version Number
	 * Encryption Key
	 * Request CyclicTransmission[Object]
	 * RequestSingeTransmission[Object]
	 */
	/* The current communication state of the server */
	OBDConnectionState server_connection_state;
protected:
	/* Reference to the diagnosis reader used for interfacing the OBD diagnosis tool */
	std::shared_ptr<Application::DiagnosisReader> diagnosis_reader;

	/* Prevent this class from being copied to prevent slicing */
	GenericOBDDataServer(const GenericOBDDataServer&);
	//operator==(const GenericOBDDataServer&);
};



class BLEOBDDataServer;
// Todo make OBDServerThread and BLEOBDDataServer one class.
class OBDServerThread : public Thread
{
public:
	friend BLEOBDDataServer;
	OBDServerThread(const std::string &name, BLEOBDDataServer &ble_obd_server);
	virtual void run();
private:
	BLEOBDDataServer &ble_obd_server;
};

class OBDDataBLETransmissionThread : public CyclicThread
{
public:
	friend BLEOBDDataServer;
	OBDDataBLETransmissionThread(BLEOBDDataServer *ble_obd_server);
	virtual void run();


	BLEOBDDataServer *ble_obd_server;
};



class BLEOBDDataServer : public GenericOBDDataServer, public BLESendRequestQueue
{
public:
	friend OBDServerThread;

	BLEOBDDataServer(std::shared_ptr<Application::DiagnosisReader> diagnosis_reader);

	virtual void startup();
	virtual void shutdown();

	virtual void set_transmission_request(OBDDataRequestType type);
	virtual void clear_transmission_request(OBDDataRequestType type);


	void start_communication_threads();
	void join_communication_threads();

	virtual std::vector<unsigned char> get_server_public_encryption_key() const;
	virtual int set_server_private_encryption_key(const std::vector<unsigned char> &encryption_key);


	virtual int client_request_encryption_key(const std::chrono::milliseconds& timeout, std::vector<unsigned char> &encryption_key);


	virtual int set_client_public_encryption_key(const std::vector<unsigned char> &encryption_key);

	int initialize_client_connection();

	BLEInterface ble_interface;

public: // TODO Make private at some point
	OBDDataTransmissionMode get_obd_transmission_mode() const;
	void set_obd_transmission_mode(OBDDataTransmissionMode en_transmission_mode);
private:

	void server_process_received_command(const BLETransmitBuffer &command, const BLETransmitBuffer *value = nullptr);

	/* Automatically sends encrypted or unencrypted, depending on connection state */
	int server_send_secured(int characteristic_id, const std::vector<unsigned char> &buffer);

	int server_send_command(int characteristic_id, const std::vector<unsigned char> command, std::vector<unsigned char> *value = NULL);

	int server_receive_command(int characteristic_id, const std::vector<unsigned char> command, std::vector<unsigned char> *value = NULL, const std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));

	int server_receive_any_command(int characteristic_id, std::vector<unsigned char> &command, std::vector<unsigned char> *value = NULL, const std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));


	/* Receive function for OBD commands */
	//std::vector<unsigned char> characteristic_receive_command_received_data;
	//Todo this should be arrays */
	bool characteristic_receive_command_data_received_complete;
	std::mutex characteristic_receive_command_data_receive_mutex;
	std::condition_variable characteristic_receive_data_cv;
	unsigned int received_data_characteristic_id;
	std::vector<unsigned char> received_command;
	std::vector<unsigned char> received_value;
	std::atomic<bool> data_received_complete;

	void receive_from_characteristic(int characteristic_id, const std::vector<unsigned char> &buffer);


	bool server_is_running;

	/* Collection of all BLE communication threads that are currently active */
	//ThreadRepository transmission_thread_repository;
	bool active_requests[DATA_REQUEST_NUM_OF_ITEMS];

	std::shared_ptr<OBDServerThread> thread_ble_obd_server_thread;
	std::shared_ptr<OBDDataBLETransmissionThread> thread_obd_data_transmission;
	std::shared_ptr<CyclicThread> thread_obd_error_codes_transmission;

	OBDDataTransmissionMode m_en_transmission_mode;

private: // Todo remove



};

#if 0
/* Callback to receive commands from the client */
class CommandReceiverCallback : public BLECharacteristicCallbacks
{
public:
	void onWrite(BLECharacteristic *pCharacteristic);

	void set_ble_obd_data_server(BLEOBDDataServer *ble_obd_data_server);

private:
	BLEOBDDataServer* ble_obd_data_server;
};
#endif


#if 0
#if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BLUEDROID_ENABLED)

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bt.h"

#include "Arduino.h"

struct ble_gap_adv_params_s;

class OBDIICCBluetooth {
    public:

		OBDIICCBluetooth(void);
		~OBDIICCBluetooth(void);

        /**
         * Start BLE Advertising
         *
         * @param[in] localName  local name to advertise
         *
         * @return true on success
         *
         */
        bool begin(String localName=String());

        /**
         * Stop BLE Advertising
         *
         * @return none
         */
        void end(void);

    private:
        String local_name;
    private:

};

#endif
#endif

#endif
