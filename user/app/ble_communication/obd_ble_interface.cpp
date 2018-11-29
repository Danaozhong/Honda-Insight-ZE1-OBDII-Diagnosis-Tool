#include <cassert>
#include <algorithm>
#include <mutex>
#include "driver/ble/ble_interface.h"

#include "midware/trace/trace.h"

#include <functional>   // std::bind

#include "driver/ble/ble_interface.h"
#include "app/ble_communication/obd_ble_interface.h"

#include "../../midware/threads/cyclic_thread.hpp"
#include "../diagnosis_reader.hpp"


GenericOBDDataServer::GenericOBDDataServer(std::shared_ptr<Application::DiagnosisReader> diagnosis_reader)
	: diagnosis_reader(diagnosis_reader), server_connection_state(CONNECTION_STATE_DISCONNECTED)
{}

GenericOBDDataServer::~GenericOBDDataServer()
{}


int GenericOBDDataServer::get_protocol_version_major() const
{
	return 1;
}

int GenericOBDDataServer::get_protocol_version_minor() const
{
	return 0;
}

/* Generates a hash out of the data table to compare it with the hash of the other client */
int GenericOBDDataServer::get_obd_data_hash(char* buffer, size_t buffer_size) const
{
	snprintf(buffer, buffer_size - 1, "abe472a");
	return 0;
}

std::shared_ptr<Application::DiagnosisReader> GenericOBDDataServer::get_diagnosis_reader()
{
	return this->diagnosis_reader;
}


OBDDataBLETransmissionThread::OBDDataBLETransmissionThread(BLEOBDDataServer *ble_obd_server)
	: CyclicThread("BLEDataTransmissionThread", std::chrono::milliseconds(100)), ble_obd_server(ble_obd_server)
{}

#include <stdint.h>

void OBDDataBLETransmissionThread::run()
{
	//
	if (this->ble_obd_server != nullptr && this->ble_obd_server->get_diagnosis_reader() != nullptr)
	{
		auto changed_items = this->ble_obd_server->get_diagnosis_reader()->get_changed_obd_data();
		if(this->ble_obd_server->get_obd_transmission_mode() == TRANSMISSION_MODE_ONLY_SELECTED_DATA_FAST)
		{
			changed_items = this->ble_obd_server->get_diagnosis_reader()->get_changed_obd_data_of_interest();
		}

		this->ble_obd_server->get_diagnosis_reader()->clear_changed_obd_data();

		/* The messages are setup as follows:
		* Message length: variable
		* Message header: 4 bytes identifier + 2 bytes total content length information (excluding the 6 bytes from header)
		*
		* Each data field consists of one byte for the identifier, then the data as bytes.
		* Data fields are connected as follows:
		*
		*/

		/* Calculate the buffer size */
		uint16_t u16_tx_message_size = 6u;

		DEBUG_PRINTF("%i", changed_items.size());
		for (auto itr : changed_items)
		{
			u16_tx_message_size += 1;
			u16_tx_message_size += sizeof(itr->value_f);
		}
		std::vector<unsigned char> tx_message(u16_tx_message_size);
		tx_message[0] = 'C';
		tx_message[1] = 'O';
		tx_message[2] = 'B';
		tx_message[3] = 'D';

		auto buffer_itr = tx_message.begin() + 6;

		for (auto itr : changed_items)
		{
			uint8_t data_field_length = 0;
			unsigned char *data = NULL;

			switch (itr->type)
			{
			case OBD_VALUE_NUMERIC:
				data_field_length = sizeof(itr->value_f);
				data = (unsigned char*) &itr->value_f;
				break;
			case OBD_VALUE_BOOLEAN:
				data_field_length = sizeof(itr->value_b);
				data = (unsigned char*) &itr->value_b;
				break;
			default:
				/* Unsupported value type, transmit with length 0 */
				break;
			}

			*(buffer_itr++) = itr->identifier;
			memcpy(&(*buffer_itr), data, data_field_length);
			buffer_itr += data_field_length;
		}



		uint16_t u16_total_size = tx_message.size() - 6u;

		if (u16_total_size > 0)
		{
			tx_message[4] = reinterpret_cast<unsigned char*>(&u16_total_size)[0];
			tx_message[5] = reinterpret_cast<unsigned char*>(&u16_total_size)[1];

			/* Send the message to the client */
			DEBUG_PRINTF("OBD Packet sent, size: " + helper::to_string(u16_total_size));
			this->ble_obd_server->ble_interface.send_interface(BLE_CHARACTERISTIC_ID_SEND_OBD_DATA, tx_message);
		}
		else
		{
			DEBUG_PRINTF("Nothing to send!");
		}
	}
	else
	{
		DEBUG_PRINTF("Something is a nullptr");
	}
}


BLEOBDDataServer::BLEOBDDataServer(std::shared_ptr<Application::DiagnosisReader> diagnosis_reader)
: GenericOBDDataServer(diagnosis_reader), server_is_running(false), data_received_complete(false),
  received_data_characteristic_id(0u), thread_ble_obd_server_thread(NULL),
  thread_obd_data_transmission(NULL), thread_obd_error_codes_transmission(NULL),
  characteristic_receive_command_data_received_complete(false),
  m_en_transmission_mode(TRANSMISSION_MODE_ONLY_SELECTED_DATA_FAST)
{
	thread_ble_obd_server_thread = std::shared_ptr<OBDServerThread>(new OBDServerThread("OBDBLEServer", *this));
}

void BLEOBDDataServer::startup()
{
	if (this->server_is_running == true)
	{
		return;
	}
	this->server_is_running = true;

	/* Create the cyclic threads, but do not launch them yet */
	assert(this->thread_obd_data_transmission == NULL);
	assert(this->thread_obd_error_codes_transmission == NULL);

	/* Currently only OBD data transmission thread is supported */
	this->thread_obd_data_transmission = std::shared_ptr<OBDDataBLETransmissionThread>(new OBDDataBLETransmissionThread(this));

	/* Initialize the BLE hardware */
	this->ble_interface.init_interface();

	/* Register for the Rx callback of the OBD BLE transmission, use a lambda to encapsulate class interface and connection ID */
    auto ble_characteristic_receive_commands_rx = std::bind(&BLEOBDDataServer::receive_from_characteristic, this, BLE_CHARACTERISTIC_ID_RECEIVE_COMMANDS, std::placeholders::_1);

	this->ble_interface.get_characteristic(BLE_CHARACTERISTIC_ID_RECEIVE_COMMANDS)->get_received_data_signal()->connect(ble_characteristic_receive_commands_rx);

	/* Start the bluetooth manager thread */
	this->thread_ble_obd_server_thread->start();
}


void BLEOBDDataServer::shutdown()
{
	if (this->server_is_running == false)
	{
		return;
	}

	this->thread_ble_obd_server_thread->join();
	this->server_is_running = false;
}

void BLEOBDDataServer::set_transmission_request(OBDDataRequestType type)
{
	if (this->active_requests[type] == true)
	{
		/* Request already set */
		return;
	}

	this->active_requests[type] = true;

	/* Specific things */
	if (type == DATA_REQUEST_TYPE_OBD_DATA)
	{

	}
}

void BLEOBDDataServer::clear_transmission_request(OBDDataRequestType type)
{
	this->active_requests[type] = false;

	/* send event if necessary */
}


void BLEOBDDataServer::start_communication_threads()
{
	if (this->thread_obd_data_transmission != nullptr)
	{
		this->thread_obd_data_transmission->start();
	}

	if (this->thread_obd_error_codes_transmission != nullptr)
	{
		this->thread_obd_error_codes_transmission->start();
	}
}

void BLEOBDDataServer::join_communication_threads()
{
	/* Signalize all cyclic threads to terminate */
	if (this->thread_obd_data_transmission != nullptr)
	{
		this->thread_obd_data_transmission->join();
	}

	if (this->thread_obd_error_codes_transmission != nullptr)
	{
		this->thread_obd_error_codes_transmission->join();
	}
}

std::vector<unsigned char> BLEOBDDataServer::get_server_public_encryption_key() const
{
	return std::vector<unsigned char> {'Z', 'A', 'N' };
}

int BLEOBDDataServer::set_server_private_encryption_key(const std::vector<unsigned char> &encryption_key)
{
	/* Dummy */
	return 0;
}


int BLEOBDDataServer::client_request_encryption_key(const std::chrono::milliseconds& timeout, std::vector<unsigned char> &encryption_key)
{
	this->server_send_secured(BLE_CHARACTERISTIC_ID_SEND_COMMANDS, BLECommand::BLE_COMMAND_REQUEST_ENCRYPTION_KEY);

	/* Wait for secure handshake */
	return this->server_receive_command(BLE_CHARACTERISTIC_ID_RECEIVE_COMMANDS,
			BLECommand::BLE_COMMAND_PUBLISH_ENCRYPTION_KEY, &encryption_key, timeout);
}


int BLEOBDDataServer::set_client_public_encryption_key(const std::vector<unsigned char> &encryption_key)
{
	/* Dummy */
	return 0;
}

std::vector<unsigned char> operator+ (const std::vector<unsigned char> v1, const std::vector<unsigned char> v2)
{
	std::vector<unsigned char> cpy = v1;
	cpy.insert(cpy.end(), v1.begin(), v2.end());
	return cpy;
}

int BLEOBDDataServer::initialize_client_connection()
{
	this->server_connection_state = CONNECTION_STATE_INIT;

	/* Request encryption */
	std::vector<unsigned char> encryption_key;
	if (this->client_request_encryption_key(std::chrono::seconds(2), encryption_key) != 0)
	{
		/* Todo abort connection and wait for new connection. */
		DEBUG_PRINTF("Request of encryption key failed!");
		return -1;
	}

	DEBUG_PRINTF("Client connected, initializing encryption...");
	this->server_connection_state = CONNECTION_STATE_INIT_ENCRYPTION;
	auto server_public_encryption_key = this->get_server_public_encryption_key();
	this->server_send_command(BLE_CHARACTERISTIC_ID_SEND_COMMANDS,
			BLECommand::BLE_COMMAND_PUBLISH_ENCRYPTION_KEY, &server_public_encryption_key);

	/* Wait for secure handshake */
	if (this->server_receive_command(BLE_CHARACTERISTIC_ID_RECEIVE_COMMANDS,
			BLECommand::BLE_COMMAND_ENCRYPTION_HANDSHAKE) != 0)
	{
		/* Encryption handshake failed, abort! */
		DEBUG_PRINTF("Client connected, but encryption handshake was not received!");
		return -1;
	}

	this->server_connection_state = CONNECTION_STATE_DATA_PROTOCOL_SYNC;

	this->server_send_secured(BLE_CHARACTERISTIC_ID_SEND_COMMANDS, BLECommand::BLE_COMMAND_REQUEST_OBD_DATA_HASH);
	/* Wait for secure handshake */
	if (this->server_receive_command(BLE_CHARACTERISTIC_ID_RECEIVE_COMMANDS,
			BLECommand::BLE_COMMAND_OBD_DATA_HASH) != 0)
	{
		DEBUG_PRINTF("Receiving the client obd data hash failed!");
		return -1;
	}

	/* Successfully managed to read the OBD Data hash */
	/* TODO compare the hashes */

	DEBUG_PRINTF("Connection established!");
	this->server_connection_state = CONNECTION_STATE_CONNECTED;
	return 0;
}

OBDDataTransmissionMode BLEOBDDataServer::get_obd_transmission_mode() const
{
	return this->m_en_transmission_mode;
}

void BLEOBDDataServer::set_obd_transmission_mode(OBDDataTransmissionMode en_transmission_mode)
{
	if (m_en_transmission_mode == en_transmission_mode)
	{
		return;
	}

	m_en_transmission_mode = en_transmission_mode;
	switch (m_en_transmission_mode)
	{
	case TRANSMISSION_MODE_ONLY_SELECTED_DATA_FAST:
		this->thread_obd_data_transmission->set_interval(std::chrono::milliseconds(100));
		break;

	case TRANSMISSION_MODE_ALL_DATA_SLOW:
		this->thread_obd_data_transmission->set_interval(std::chrono::milliseconds(500));
		break;
	}
}

void BLEOBDDataServer::server_process_received_command(const BLETransmitBuffer &command, const BLETransmitBuffer *value)
{
	int i32_ret_val = -1;
	if(command == BLECommand::BLE_COMMAND_SET_INTERESTING_OBD_DATA)
	{
		/* Command is to update the list of interesting OBD data */
		if (value != nullptr)
		{
			i32_ret_val = 0;
			DEBUG_PRINTF("Set elements of interest!");
			this->diagnosis_reader->set_elements_of_interests(*value);
			std::vector<unsigned char> obd_data_of_interest;
		}
	}
	else if (command == BLECommand::BLE_COMMAND_CHANGE_OBD_TRANSMISSION_MODE)
	{
		/* Command to change the OBD data transmission mode (slow and all data, or fast and only selected data) */
		if (value != nullptr && static_cast<OBDDataTransmissionMode>((*value)[0]) < TRANSMISSION_MODE_NUM_OF_ELEMENTS)
		{
			/* The value is valid! */
			i32_ret_val = 0;
			this->set_obd_transmission_mode(static_cast<OBDDataTransmissionMode>((*value)[0]));

		}
	}


	if (i32_ret_val == -1)
	{
		DEBUG_PRINTF("Received unknown command: " + helper::to_string(static_cast<int>(command[0])));
	}
}


int BLEOBDDataServer::server_send_secured(int characteristic_id, const std::vector<unsigned char> &buffer)
{
	if (this->server_connection_state >= CONNECTION_STATE_INIT_ENCRYPTION)
	{
		/* TODO Encrypt data */
	}
	else
	{
		/* Nothing to do, send data non encrypted */
	}
	this->ble_interface.send_interface(characteristic_id, buffer);
	return 0;
}

int BLEOBDDataServer::server_send_command(int characteristic_id, const std::vector<unsigned char> command, std::vector<unsigned char> *value)
{
	std::vector<unsigned char> data = command;
	if (value != nullptr && value->size() > 0u)
	{
		data.insert(data.end(), (*value).begin(), (*value).end());
	}
	return this->server_send_secured(characteristic_id, data);
}


int BLEOBDDataServer::server_receive_command(int characteristic_id, const std::vector<unsigned char> command, std::vector<unsigned char> *value, const std::chrono::milliseconds timeout)
{
	std::vector<unsigned char> actually_received_command;
	std::vector<unsigned char> actually_received_value;

	if(0 == server_receive_any_command(characteristic_id, actually_received_command, &actually_received_value, timeout))
	{
		if (actually_received_command[0] == command[0])
		{
			if (nullptr != value)
			{
				*value = actually_received_value;
			}
			return 0;
		}
	}

	/* Nothing received (timeout), or incorrect command received */
	return -1;
}

int BLEOBDDataServer::server_receive_any_command(int characteristic_id, std::vector<unsigned char> &command, std::vector<unsigned char> *value, const std::chrono::milliseconds timeout)
{
	//std::unique_lock<std::mutex> lck(characteristic_receive_command_data_receive_mutex);
	/* Wait to receive data */
	const auto ts_begin = std::chrono::system_clock::now();
	while(data_received_complete == false)
	{
		TaskHelper::sleep_for(std::chrono::milliseconds(50));
		auto ts_now = std::chrono::system_clock::now();
		if (ts_now - ts_begin > timeout)
		{
			break;
		}
	}

	//if (characteristic_receive_data_cv.wait_for(lck, timeout, [this] () -> bool {return data_received_complete == true;}))
	if(data_received_complete == true)
	{
		DEBUG_PRINTF("Reply received!!");
		command = received_command;

		//std::string command_as_string(command.begin(), command.end());
		if (nullptr != value)
		{
			*value = received_value;
		}
		data_received_complete = false;
		return 0;
	}
	else
	{
		/* Data not received (timeout). */
		DEBUG_PRINTF("Timeout");
		return -1;
	}
}

void BLEOBDDataServer::receive_from_characteristic(int characteristic_id, const std::vector<unsigned char> &received_buffer)
{
	char buffer123[200] = { 0 };
	snprintf(buffer123, 200, "BLE task: stack %u, current free heap: %d, minimum ever free heap: %d", uxTaskGetStackHighWaterMark(NULL), xPortGetFreeHeapSize(), xPortGetMinimumEverFreeHeapSize());
	DEBUG_PRINTF(buffer123);

	if (received_buffer.size() < 1)
	{
		return;
	}
	if(data_received_complete == true)
	{
		DEBUG_PRINTF("Still got data!");
		return;

	}

	received_command = { received_buffer[0] };
	received_value = {};

	if (received_buffer.size() > 1)
	{
		received_value = std::vector<unsigned char>(received_buffer.begin() + 1, received_buffer.end());
	}
	/* Signalise waiting thread that data has been received */
	//std::unique_lock<std::mutex> lck(characteristic_receive_command_data_receive_mutex);
	data_received_complete = true;
	//characteristic_receive_data_cv.notify_all();
}


OBDServerThread::OBDServerThread(const std::string &name, BLEOBDDataServer &ble_obd_server)
	: Thread(name), ble_obd_server(ble_obd_server)
{}


void OBDServerThread::run()
{
	ble_obd_server.server_connection_state = CONNECTION_STATE_DISCONNECTED;

	while(true)
	{
		/* Wait for client to connected */
		while(ble_obd_server.ble_interface.is_connected() == false)
		{
			/* Todo replace by something like an event handler */
			TaskHelper::sleep_for(std::chrono::seconds(2));
			DEBUG_PRINTF("Waiting for BLE client to connect...");
		}

		TaskHelper::sleep_for(std::chrono::seconds(1));
		DEBUG_PRINTF("BLE client connected, initializing connection...");
		if(0 != ble_obd_server.initialize_client_connection())
		{
			/* Initialization failed failed */
			ble_obd_server.ble_interface.disconnect();
			ble_obd_server.server_connection_state = CONNECTION_STATE_DISCONNECTED;
			continue;
		}

		/* Spawn the worker threads */
		ble_obd_server.start_communication_threads();

		/* Wait for commands / keep alive */
		bool connection_alive = true;

		auto last_rx_timestamp = std::chrono::system_clock::now();
		auto last_tx_timestamp = std::chrono::system_clock::now();

		while(connection_alive)
		{
			/* Wait for commands / or keep alives */
			BLETransmitBuffer received_command;
			BLETransmitBuffer received_value;
			if(0 == ble_obd_server.server_receive_any_command(BLE_CHARACTERISTIC_ID_RECEIVE_COMMANDS,
					received_command, &received_value, std::chrono::milliseconds(100)))
			{
				last_rx_timestamp = std::chrono::system_clock::now();

				/* check frame content */
				ble_obd_server.server_process_received_command(received_command, &received_value);

				/* TODO Check packet content */
			}

			/* Check if there is something in the queue */
			{
				std::lock_guard<std::mutex> lck(this->ble_obd_server.m_ble_requeust_queue_mutex);
				while (this->ble_obd_server.m_ble_request_queue.size() > 0)
				{
					auto current_request = this->ble_obd_server.m_ble_request_queue.front();
					ble_obd_server.m_ble_request_queue.pop_front();

					int ret_val = -1;
					if (0 != ble_obd_server.server_send_command(BLE_CHARACTERISTIC_ID_SEND_COMMANDS,
							current_request->send_command, &current_request->send_value))
					{
						/* Sending the signal failed, continue with next queue item */
						continue;
					}
					current_request->SIG_BLE_REQUEST_SENT(0);

					/* Check if a reply is requested */
					if (current_request->request_type == BLE_REQUEST_SEND_AND_RECEIVE)
					{
						if(0 == ble_obd_server.server_receive_any_command(BLE_CHARACTERISTIC_ID_RECEIVE_COMMANDS,
								current_request->received_command, &current_request->received_value, current_request->timeout))
						{
							/* Successfully received an answer via BLE, signalize to client */
							current_request->SIG_BLE_REQUEST_REPLY_RECEIVED(0);

							/* Update timestamp that we have received something */
							last_rx_timestamp = std::chrono::system_clock::now();
						}
					}
				}
			}

			/* Ensure we did not run into timeout */
			if (std::chrono::system_clock::now() - last_rx_timestamp > std::chrono::seconds(10))
			{
				/*Nothing received within 10 seconds, abort! */
				connection_alive = false;
			}

            /* Check if it necessary to send heartbeat */
            if (std::chrono::system_clock::now() - last_tx_timestamp > std::chrono::seconds(5))
            {
                /* send a heartbeat and reset the counter */
                std::vector<unsigned char> heartbeat_data = { 'V', 'B', 'A' };
                DEBUG_PRINTF("send hb");
                if (0 != ble_obd_server.server_send_command(
                        BLE_CHARACTERISTIC_ID_SEND_COMMANDS,
                        BLECommand::BLE_COMMAND_CLIENT_KEEP_ALIVE, &heartbeat_data))
                {
                    /* Sending heartbeat failed, close connection. */
                    DEBUG_PRINTF("Error: Sending heartbeat failed, exiting!");
                    //connection_alive = false;
                }
                last_tx_timestamp = std::chrono::system_clock::now();
            }
		}

		/* Terminate all connection threads */
		ble_obd_server.server_connection_state = CONNECTION_STATE_DISCONNECTED;
		ble_obd_server.join_communication_threads();
	}
	/* Never terminates */
}

