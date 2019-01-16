/*
 * thread.hpp
 *
 *  Created on: 29.11.2018
 *      Author: Clemens
 */


// TODO Clemens: replace by using ACE or boost

#ifndef USER_MIDWARE_THREADS_THREAD_HPP_
#define USER_MIDWARE_THREADS_THREAD_HPP_

#include <atomic>
#include <string>
#include <thread>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace ThreadingHelper
{
	int set_priority(std::thread &thread, int priority);
	int set_stack_size(std::thread &thread, size_t stack_size);
	//int set_default_stack_size(std::thread &thread, size_t stack_size);
}


namespace std_ex
{
	class thread
	{
	public:
	    // Abstract base class for types that wrap arbitrary functors to be
	    // invoked in the new thread of execution.
	    struct _State
	    {
	      virtual ~_State();
	      virtual void _M_run() = 0;
	    };
	    using _State_ptr = std::unique_ptr<_State>;


	    template<typename _Callable, typename... _Args>
	      explicit
	      thread(_Callable&& __f, _Args&&... __args)
	      {
	        _M_start_thread(_S_make_state(
		      std::__bind_simple(std::forward<_Callable>(__f),
					 std::forward<_Args>(__args)...)));
	      }

		~thread();

	    void join();

	private:
		template<typename _Callable>
		struct _State_impl : public _State
		{
			_Callable		_M_func;

			_State_impl(_Callable&& __f) : _M_func(std::forward<_Callable>(__f))
			{}

			void _M_run() { _M_func(); }
		};

		//std::atomic<bool> terminate;
		TaskHandle_t m_task_handle;
		std::string m_task_name;
		size_t m_u_task_priority;
		size_t m_u_stack_size;

	private:
		void _M_start_thread(_State_ptr);

		template<typename _Callable>
		static _State_ptr _S_make_state(_Callable&& __f)
		{
			using _Impl = _State_impl<_Callable>;
			return _State_ptr(new _Impl(std::forward<_Callable>(__f)));
		}

	};
}

// UGLY FROM HERE BELOW - use std::thread in the future
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

#if 0
/* if std::thread does not work, use ugly ACE, still better than directly call the OS.... */
class ACE_Task_Base
{
public:
	ACE_Task_Base(const std::string &name);

	virtual ~ACE_Task_Base();

	virtual int start();

	virtual int activate(long flags = THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
			int n_threads = 1,
			int force_active = 0,
			long priority = ACE_DEFAULT_THREAD_PRIORITY,
			int grp_id = -1,
			ACE_Task_Base* task = 0,
			ACE_hthread_t thread_handles[] = 0,
			void * stack[] = 0,
			size_t stack_size[] = 0,
			ACE_thread_t thread_ids[] = 0,
			const char * thr_name[] = 0
		);


#endif
#endif /* USER_MIDWARE_THREADS_THREAD_HPP_ */
