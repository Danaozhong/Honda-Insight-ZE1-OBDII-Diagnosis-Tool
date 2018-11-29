/*
 * ble_request_queue.cpp
 *
 *  Created on: 21.03.2018
 *      Author: Clemens
 */

#include "ble_request_queue.hpp"
#include <condition_variable>

#include <atomic>

#include "../../midware/threads/cyclic_thread.hpp"

BLESendRequestQueue::BLESendRequestQueue() {
	// TODO Auto-generated constructor stub

}

BLESendRequestQueue::~BLESendRequestQueue() {
	// TODO Auto-generated destructor stub
}


int BLESendRequestQueue::enqueue_request(const std::shared_ptr<BLESendRequestQueueEntry> &request)
{
	std::lock_guard<std::mutex> lck(this->m_ble_requeust_queue_mutex);
	this->m_ble_request_queue.emplace_back(request);
	return 0;

}
/* Helper functions to create requests */
int BLESendRequestQueue::send_command(const BLETransmitBuffer &command, const BLETransmitBuffer *value)
{
	auto send_request = std::shared_ptr<BLESendRequestQueueEntry>(new BLESendRequestQueueEntry);
	send_request->request_type = BLE_REQUEST_SEND_ONLY;
	send_request->send_command = command;
	send_request->send_value = {};
	if (value != nullptr)
	{
		/* Must be a copy, as the original buffer might already be deleted before the command is executed on the server side */
		send_request->send_value = *value;
	}
	return this->enqueue_request(send_request);
}

int BLESendRequestQueue::send_command_and_wait_for_reply(const BLETransmitBuffer &send_command, const BLETransmitBuffer *send_value, BLETransmitBuffer &received_command, BLETransmitBuffer *received_value, const std::chrono::milliseconds timeout)
{
	auto send_request = std::shared_ptr<BLESendRequestQueueEntry>(new BLESendRequestQueueEntry);
	send_request->request_type = BLE_REQUEST_SEND_AND_RECEIVE;
	send_request->send_command = send_command;
	send_request->send_value = {};
	if (send_value != nullptr)
	{
		send_request->send_value = *send_value;
	}

	send_request->timeout = timeout;

	/* Connect a local function to the signal which is sent when a reply is received. Later w will wait for this signal to
	 * be received.
	 */
	this->m_bo_current_request_reply_received = false;
    auto ble_reply_received = std::bind(&BLESendRequestQueue::server_current_request_reply_received_signal_handler, this);
	send_request->SIG_BLE_REQUEST_REPLY_RECEIVED.connect(ble_reply_received);

	this->enqueue_request(send_request);

	/* TODO This is still very ugly, write a proper wait for() */
	auto  begin_wait_time = std::chrono::system_clock::now();
	auto  current_time = begin_wait_time;
	while (current_time - begin_wait_time < timeout)
	{
		if (this->m_bo_current_request_reply_received == true)
		{
			/* Reply has been received in time! */
			return 0;
		}
		TaskHelper::sleep_for(std::chrono::milliseconds(20));
		current_time = std::chrono::system_clock::now();
	}

	/* Timeout while waiting to receive the reply */
	return -1;

}

void BLESendRequestQueue::server_current_request_reply_received_signal_handler(void)
{
	this->m_bo_current_request_reply_received = true;
}
