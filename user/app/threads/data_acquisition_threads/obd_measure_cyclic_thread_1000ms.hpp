/*
 * obd_measure_cylic_thread_1000ms.hpp
 *
 *  Created on: 29.11.2018
 *      Author: Clemens
 */

#ifndef USER_APP_THREADS_DATA_ACQUISITION_THREADS_OBD_MEASURE_CYCLIC_THREAD_1000MS_HPP_
#define USER_APP_THREADS_DATA_ACQUISITION_THREADS_OBD_MEASURE_CYCLIC_THREAD_1000MS_HPP_

#include "../../../midware/threads/cyclic_thread.hpp"
#include "app/diagnosis_reader.hpp"

class OBD_MeasureCyclicThread_1000ms : public CyclicThread
{
public:
	OBD_MeasureCyclicThread_1000ms(Application::DiagnosisReader &diagnosis_reader);
	virtual void run();
protected:
	Application::DiagnosisReader &ref_diagnosis_reader;
};


#endif /* USER_APP_THREADS_DATA_ACQUISITION_THREADS_OBD_MEASURE_CYCLIC_THREAD_1000MS_HPP_ */
