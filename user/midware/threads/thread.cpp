/*
 * thread.cpp
 *
 *  Created on: 29.11.2018
 *      Author: Clemens
 */
#include "thread.hpp"
#include "trace.h"


#include "Arduino.h" /* For delay() */


Thread::Thread(const std::string &name)
: name(name), terminate(false), task_handle(nullptr)
{}

Thread::~Thread()
{
	if (this->task_handle != nullptr)
	{
		this->join();
	}
}

int Thread::start()
{
	DEBUG_PRINTF("Task " + this->name + " started!");

	/* TODO create a table to define the task priorities and sizes */
	this->start_task(4096, 1, [](void* o){ static_cast<Thread*>(o)->run(); });
}

int Thread::start_task(size_t stack_size, size_t priority, void (*fp)(void*))
{
	TaskHandle_t task_handle;
	  xTaskCreate(
		  fp,           			/* Task function. */
		  name.c_str(),        				/* name of task. */
		  stack_size,               /* Stack size of task */
	      this,                     /* parameter of the task */
		  priority,                /* priority of the task */
	      &task_handle);           /* Task handle to keep track of created task */
	return 0;
}


int Thread::stop()
{
	DEBUG_PRINTF("Sending thread " + this->name + " to stop!");
	this->terminate = true;
	return 0;
}


int Thread::join()
{
	if (this->terminate == false)
	{
		this->stop();
	}
	while (this->task_handle != nullptr)
	{
		delay(100);
	}
	DEBUG_PRINTF("Thread " + this->name + " stopped!");
	return 0;
}




