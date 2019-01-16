#include "obd_measure_cyclic_thread_1000ms.hpp"

#include <chrono>





OBD_MeasureCyclicThread_1000ms::OBD_MeasureCyclicThread_1000ms(Application::CommunicationManager &diagnosis_reader)
	: CyclicThread("OBDataAcquisition1000msCyclic", std::chrono::milliseconds(1000)), ref_diagnosis_reader(diagnosis_reader)
{
}

void OBD_MeasureCyclicThread_1000ms::run()
{
	ref_diagnosis_reader.cycle_read_error_codes();
}
