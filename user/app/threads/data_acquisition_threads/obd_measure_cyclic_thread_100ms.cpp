#include <chrono>

#include "obd_measure_cyclic_thread_100ms.hpp"



OBD_MeasureCyclicThread_100ms::OBD_MeasureCyclicThread_100ms(Application::DiagnosisReader &diagnosis_reader)
	: CyclicThread("OBDataAcquisition100msCyclic", std::chrono::milliseconds(50)), ref_diagnosis_reader(diagnosis_reader)
{
}

void OBD_MeasureCyclicThread_100ms::run(void)
{
	ref_diagnosis_reader.cycle_read_obd_data();
}
