/*
 * thread.hpp
 *
 *  Created on: 29.11.2018
 *      Author: Clemens
 */

#ifndef USER_MIDWARE_THREADS_THREAD_HPP_
#define USER_MIDWARE_THREADS_THREAD_HPP_

#include <atomic>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/** C++ wrapper for the RTOS tasks */
class Thread
{
public:
	Thread(const std::string &name);

	virtual ~Thread();

	virtual int start();
	int start_task(size_t stack_size, size_t priority, void (*fp)(void*));
	virtual int stop();
	virtual int join();
	virtual void run() = 0;
protected:
	std::string name;
	std::atomic<bool> terminate;
	volatile TaskHandle_t task_handle;
};


#endif /* USER_MIDWARE_THREADS_THREAD_HPP_ */
