
#include "diagnosis_reader_application.hpp"
#include "dummy_diagnosis_device.hpp"
#include "obd_ii_cnc_uart_diagnosis_device.hpp"
#include "trace.h"


#include "app/threads/data_acquisition_threads/obd_measure_cyclic_thread_100ms.hpp"
#include "app/threads/data_acquisition_threads/obd_measure_cyclic_thread_1000ms.hpp"


namespace Application
{
	DiagnosisReaderApplication::DiagnosisReaderApplication()
	:diagnosis_reader(nullptr), ble_server(nullptr)
	{
		/* Create a OBD II reader (currently, a dummy OBD II interface is used) */

		uart_config_t st_uart_configuration = {
		    .baud_rate = 38400,
		    .data_bits = UART_DATA_8_BITS,
		    .parity = UART_PARITY_DISABLE,
		    .stop_bits = UART_STOP_BITS_1,
		    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		    .rx_flow_ctrl_thresh = 0,
		};

		this->obd_diagnosis_device = std::make_shared<OBDIICnCUARTDiagnosisDevice>(1, 22, 23, st_uart_configuration);

		if (nullptr == this->obd_diagnosis_device)
		{
			FATAL("Creation of the OBD Diagnosis device failed!");
		}


		this->diagnosis_reader = std::shared_ptr<CommunicationManager>(new CommunicationManager(obd_diagnosis_device, obd_ii_diagnosis_data));

		if(nullptr == this->diagnosis_reader)
		{
			FATAL("Creation of the diagnosis reader interface failed!");
		}

		diagnosis_reader->connect();


		/* Start the bluetooth BLE interface */
		DEBUG_PRINTF("Starting Bluetooth!");
		this->ble_server = std::shared_ptr<BLEOBDDataServer>(new BLEOBDDataServer(this->diagnosis_reader));
		this->ble_server->startup();

		/* Start the data acquisition threads */



		DEBUG_PRINTF("Creating OBD Data Acquisition Threads!");
		//std::shared_ptr<Thread> thread_cyclic_100ms = std::shared_ptr<OBD_MeasureCyclicThread_100ms>(new OBD_MeasureCyclicThread_100ms(*this->diagnosis_reader));
		std::shared_ptr<Thread> thread_cyclic_1000ms = std::shared_ptr<OBD_MeasureCyclicThread_1000ms>(new OBD_MeasureCyclicThread_1000ms(*this->diagnosis_reader));

		//thread_repository.add_thread(thread_cyclic_100ms);
		thread_repository.add_thread(thread_cyclic_1000ms);

		thread_repository.start_all_threads();
		DEBUG_PRINTF("OBD Data Acquisition threads started!");


		// TODO ugly patch begin - manually set the OBD II items that are of interest
		std::vector<unsigned char> au8_elements_of_interest = {0x0A, 0x41, 0x0D, 0x11, 0x0E, 0x4D, 0x0C, 0x51 };

		this->diagnosis_reader->set_elements_of_interests(au8_elements_of_interest);
		// TODO ugly patch end

	}

	DiagnosisReaderApplication::~DiagnosisReaderApplication()
	{
		DEBUG_PRINTF("Destructor of Diagnosis Reader Application is called!");
		thread_repository.join_all_threads();

		this->ble_server->shutdown();
	}

#if 0 /* Old sample code */
	float txValue = 0;
	const int readPin = 32; // Use GPIO number. See ESP32 board pinouts
	void DiagnosisReaderApplication::ble_loop()
	{
		/* Old sample code */
		//if (deviceConnected)
		//{
			// Fabricate some arbitrary junk for now...
			txValue = analogRead(readPin) / 3.456; // This could be an actual sensor reading!

			// Let's convert the value to a char array:
			char txString[8]; // make sure this is big enuffz
			dtostrf(txValue, 1, 2, txString); // float_val, min_width, digits_after_decimal, char_buffer

			//    pCharacteristic->setValue(&txValue, 1); // To send the integer value
			//    pCharacteristic->setValue("Hello!"); // Sending a test message
			//ble_interface_characteristics[0]->setValue(txString);
			//ble_interface->send_interface(2, txString);
			//pCharacteristic->notify(); // Send the value to the app!
			print_serial("*** Sent Value: ");
			print_serial(txString);
			print_serial(" ***");
		//}
		delay(1000);
	}

#endif
	void DiagnosisReaderApplication::loop()
	{
		/* This is the main thread, nothing to do */
		//ble_loop();
		delay(100);
	}
}
