// 
// 
// 
/* System header */
#include <string>
#include <cstring>

#include <vector>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_log.h>

/* Foreign header */

/* Own header */
#include "driver/ble/adapt/ble_configuration.hpp"
#include "driver/ble/ble_interface.h"



void MyServerCallbacks::onConnect(BLEServer* pServer)
{
	m_bo_device_connected = true;
}

void MyServerCallbacks::onDisconnect(BLEServer* pServer)
{
	m_bo_device_connected = false;
}

bool MyServerCallbacks::is_connected() const
{
	return m_bo_device_connected;
}

void MyCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
	std::string rxValue = pCharacteristic->getValue();
	if (rxValue.length() > 0)
	{
#if 0
	if (rxValue.length() > 0) {
		print_serial("*********");
		print_serial("Received Value: ");

		for (int i = 0; i < rxValue.length(); i++) {
			print_serial(rxValue[i]);
		}

		char buffer123[200] = { 0 };
		snprintf(buffer123, 200, "stack %u, current free heap: %d, minimum ever free heap: %d", uxTaskGetStackHighWaterMark(NULL), xPortGetFreeHeapSize(), xPortGetMinimumEverFreeHeapSize());
		DEBUG_PRINTF(buffer123);
#endif
		const std::vector<unsigned char> rx_buffer(rxValue.begin(), rxValue.end());

		print_serial("Received Value: ");
		std::string value = "";
		for (int i = 0; i < rx_buffer.size(); i++)
		{

			value += " " + helper::to_string(static_cast<int>(rx_buffer[i]));

		}
		print_serial(value.c_str());

		if (rx_buffer.size() < 2)
		{
			/* Invalid data received! */
			this->current_message.clear();
			DEBUG_PRINTF("Invalid message received (less than two bytes)");
			return;
		}

		if (rx_buffer[0] != BLE_PACKET_BEGIN && rx_buffer[0] != BLE_PACKET_MIDDLE && rx_buffer[0] != BLE_PACKET_END)
		{
			/* Invalid data received! */
			this->current_message.clear();
			DEBUG_PRINTF("Invalid message received (invalid start byte)");
			return;
		}

		/* If a new message has been received, clear the buffer */
		if (rx_buffer[0] == BLE_PACKET_BEGIN)
		{
			this->current_message.clear();
		}

		/* Reconstruct the mssage */
		this->current_message.insert(this->current_message.end(), rx_buffer.begin() + 1, rx_buffer.end());

		if (rx_buffer[0] == BLE_PACKET_END)
		{
			/* Message has been received completely! */
			DEBUG_PRINTF("Message received of length " + helper::to_string(this->current_message.size()));

			/* Signalize application */
			this->SIG_RECEIVED_DATA(this->current_message);

			/* Clear the message buffer for subsequent receives */
			this->current_message.clear();
		}
	}
}


BLEInterfaceCharacteristic::BLEInterfaceCharacteristic(unsigned int index, const char uuid[BLE_UUID_LENGTH], int property, MyCallbacks* callback)
	: index(index), rx_callback(callback), p_characteristic(nullptr), property(property), uuid()
{
	strncpy(this->uuid, uuid, BLE_UUID_LENGTH - 1);
}

int BLEInterfaceCharacteristic::create_characteristic(BLEService &p_service)
{
	this->p_characteristic = p_service.createCharacteristic(uuid, property);

	if (rx_callback != nullptr) //this->characteristic == BLECharacteristic::PROPERTY_WRITE)
	{
		this->p_characteristic->setCallbacks(rx_callback);
	}
}

#include <string.h>
#include <string>
#include "midware/trace/trace.h"
#include <chrono>
#include "../../midware/threads/cyclic_thread.hpp"


int BLEInterfaceCharacteristic::send(const std::vector<unsigned char> &buffer)
{
	/* We can only send up to 20 bytes at once, hene we use the first byte to combine signals.
	 * If size < 20 bytes, the message is send with the end byte
	 * If size > 20 bytes, the first message is send with the start byte
	 * second message with the middle byte
	 * last message with the stop byte
	 */

	/* Start the first packet with the starting byte */
	unsigned char send_buffer[BLE_MAX_PACKET_SIZE] = { BLE_PACKET_BEGIN, 0 };

	auto send_itr = buffer.begin();
	while(send_itr != buffer.end())
	{
		size_t remaining_buffer_size = buffer.end() - send_itr;
		size_t current_packet_size = std::min(static_cast<size_t>(BLE_MAX_PACKET_SIZE - 1), remaining_buffer_size);

		if (remaining_buffer_size < BLE_MAX_PACKET_SIZE)
		{
			/* Last 20 bytes reached, send the packet end information */
			send_buffer[0] = BLE_PACKET_END;
		}
		memcpy(send_buffer + 1, &(*send_itr), current_packet_size);
		send_itr += current_packet_size;

		this->p_characteristic->setValue(static_cast<uint8_t*>(send_buffer), current_packet_size + 1);
		this->p_characteristic->notify(); // Send the value to the app!
		DEBUG_PRINTF("Sent another packet!");

		/* Prepare the next packet */
		send_buffer[0] = BLE_PACKET_MIDDLE;
	}
}


int BLEInterfaceCharacteristic::get_index() const { return this->index; }
const char* BLEInterfaceCharacteristic::get_uuid() const { return this->uuid; }


boost::signals2::signal<std::vector<unsigned char>>* BLEInterfaceCharacteristic::get_received_data_signal() const
{
	if (this->rx_callback == nullptr)
	{
		return nullptr;
	}
	return (&this->rx_callback->SIG_RECEIVED_DATA);
}




void BLEInterface::init_interface()
{
	interface_public_string = BLE_DEVICE_SSID;
	BLEDevice::init(interface_public_string); // Give it a name

	BLEServer *pServer = BLEDevice::createServer();
	server_callback = new MyServerCallbacks();
	pServer->setCallbacks(server_callback);

	print_serial("Creating the BLE services...");
	// Create the BLE Service
	BLEService *p_service = pServer->createService(SERVICE_UUID);

	for(auto itr = ble_interface_characteristics.begin(); itr != ble_interface_characteristics.end(); ++itr)
	{
		print_serial("Create one...");
		itr->create_characteristic(*p_service);
	}

	print_serial("Starting service...");
	// Start the service
	p_service->start();

	// Start advertising
	pServer->getAdvertising()->start();
	print_serial("Waiting a client connection to notify...");
}


void BLEInterface::send_interface(int index, const std::vector<unsigned char> &data)
{
	if (false == is_connected())
	{
		return;
	}

	auto characteristic = this->get_characteristic(index);
	characteristic->send(data);
}

bool BLEInterface::is_connected() const
{
	if (this->server_callback == NULL)
	{
		/* TODO make a proper error handler */
		return false;
	}
	return this->server_callback->is_connected();
}

void BLEInterface::disconnect() const
{
	// todo
}

BLEInterfaceCharacteristic* BLEInterface::get_characteristic(int id)
{
	for(auto itr = ble_interface_characteristics.begin(); itr != ble_interface_characteristics.end(); ++itr)
	{
		if (itr->get_index() == id)
		{
			return &(*itr);
		}
	}
	return nullptr;
}



#if 0
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

void init_bluetooth()
{
	// Create the BLE Device
	BLEDevice::init("OBDIIC&C"); // Give it a name


	// Create the BLE Server
	BLEServer *pServer = BLEDevice::createServer();
	pServer->setCallbacks(new MyServerCallbacks());

	// Create the BLE Service
	BLEService *pService = pServer->createService(SERVICE_UUID);

	// Create a BLE Characteristic
	pCharacteristic = pService->createCharacteristic(
		CHARACTERISTIC_UUID_TX,
		BLECharacteristic::PROPERTY_NOTIFY
		);

	pCharacteristic->addDescriptor(new BLE2902());

	BLECharacteristic *pCharacteristic = pService->createCharacteristic(
		CHARACTERISTIC_UUID_RX,
		BLECharacteristic::PROPERTY_WRITE
		);

	pCharacteristic->setCallbacks(new MyCallbacks());

	// Start the service
	pService->start();

	// Start advertising
	pServer->getAdvertising()->start();
	Serial.println("Waiting a client connection to notify...");
}
#endif
