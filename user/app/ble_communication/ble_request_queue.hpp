/*
 * ble_request_queue.hpp
 *
 *  Created on: 21.03.2018
 *      Author: Clemens
 */

#ifndef USER_APP_BLE_COMMUNICATION_BLE_REQUEST_QUEUE_HPP_
#define USER_APP_BLE_COMMUNICATION_BLE_REQUEST_QUEUE_HPP_

/* System header */
#include <chrono>
#include <memory>
#include <deque>
#include <mutex>
#include <atomic>

/* Foreign header */
#include "driver/ble/ble_interface.h"
#include "midware/events/event_handler.h"

enum BLERequestType
{
	BLE_REQUEST_SEND_ONLY,
	BLE_REQUEST_SEND_AND_RECEIVE

};
/** TODO Make this a virtual class */
class BLESendRequestQueueEntry
{
public:
	BLERequestType request_type;

	/** Signal triggered when the request is sent */
	boost::signals2::signal<int> SIG_BLE_REQUEST_SENT;

	/**  The data that should be transmitted when this request is being sent */
	BLETransmitBuffer send_command;
	BLETransmitBuffer send_value;

	/** RX only - Signal triggered when the reply is received */
	boost::signals2::signal<int> SIG_BLE_REQUEST_REPLY_RECEIVED;

	BLETransmitBuffer received_command;
	BLETransmitBuffer received_value;

	/** RX only - timeout for receiving reply */
	std::chrono::milliseconds timeout;
};



/** Class to handle command queues being sent via an BLE interface, thread safe */
class BLESendRequestQueue
{
public:
	BLESendRequestQueue();
	virtual ~BLESendRequestQueue();

	int enqueue_request(const std::shared_ptr<BLESendRequestQueueEntry> &request);

	/* Helper functions to create requests */
	int send_command(const BLETransmitBuffer &command, const BLETransmitBuffer *value = nullptr);

	int send_command_and_wait_for_reply(const BLETransmitBuffer &send_command, const BLETransmitBuffer *send_value, BLETransmitBuffer &received_command, BLETransmitBuffer *received_value, const std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));

protected:

	/* called from server side when the reply  for a request has been received */
	void server_current_request_reply_received_signal_handler(void);

	std::deque<std::shared_ptr<BLESendRequestQueueEntry>> m_ble_request_queue;
	std::mutex m_ble_requeust_queue_mutex;

	std::atomic<bool> m_bo_current_request_reply_received;
};

#endif /* USER_APP_BLE_COMMUNICATION_BLE_REQUEST_QUEUE_HPP_ */
