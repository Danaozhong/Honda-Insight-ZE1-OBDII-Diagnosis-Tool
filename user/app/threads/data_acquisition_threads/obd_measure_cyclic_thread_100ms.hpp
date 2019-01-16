/*
 * obd_measure_cylic_thread_1000ms.hpp
 *
 *  Created on: 29.11.2018
 *      Author: Clemens
 */

#ifndef USER_APP_THREADS_DATA_ACQUISITION_THREADS_OBD_MEASURE_CYCLIC_THREAD_100MS_HPP_
#define USER_APP_THREADS_DATA_ACQUISITION_THREADS_OBD_MEASURE_CYCLIC_THREAD_100MS_HPP_

#include "../../../midware/threads/cyclic_thread.hpp"
#include "../../communication_manager.hpp"

class OBD_MeasureCyclicThread_100ms : public CyclicThread
{
public:
	OBD_MeasureCyclicThread_100ms(Application::CommunicationManager &diagnosis_reader);
	virtual void run();
protected:
	Application::CommunicationManager &ref_diagnosis_reader;
};


#endif /* USER_APP_THREADS_DATA_ACQUISITION_THREADS_OBD_MEASURE_CYCLIC_THREAD_100MS_HPP_ */
