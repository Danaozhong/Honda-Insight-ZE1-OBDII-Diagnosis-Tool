
/* System header files */
#include "obd_ii_cnc_uart_diagnosis_device.hpp"

#include <stdlib.h>     /* srand, rand */
#include <memory>
#include <chrono>
#include <string>
#include <cstring>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <iterator>


/* Own header files */
#include "app/shared/obd_parameters.hpp"
#include "midware/trace/trace.h"
#include "midware/threads/cyclic_thread.hpp"



#define __OBD_II_I2C_BUFFER_SIZE (100)


OBDIICnCUARTDiagnosisDevice::OBDIICnCUARTDiagnosisDevice(uint32_t u32_uart_controller_id, uint32_t u32_rx_pin, uint32_t u32_tx_pin, const uart_config_t &st_uart_configuration)
	: connected(false), m_u32_rx_pin(u32_rx_pin), m_u32_tx_pin(u32_tx_pin), m_st_uart_config(st_uart_configuration)
{
	//using namespace std::placeholders;

	OBDataListHelper::clone(obd_ii_diagnosis_data, this->ao_obd_data);


	m_en_uart_port_num = static_cast<uart_port_t>(u32_uart_controller_id);

	// Configure driver
	if (ESP_FAIL == uart_param_config(m_en_uart_port_num, &st_uart_configuration))
	{
		DEBUG_PRINTF("Setting UART configuration failed.");
	}

	if (ESP_FAIL == uart_set_pin(m_en_uart_port_num, m_u32_rx_pin, m_u32_tx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE))
	{
		DEBUG_PRINTF("Pin setting failed!");

	}


	// Initialize driver
	// Setup UART buffered IO with event queue
	const int uart_buffer_size = (1024 * 2);

	// Install UART driver using an event queue here
	if (ESP_FAIL == uart_driver_install(m_en_uart_port_num, uart_buffer_size, uart_buffer_size, 10, &m_uart_queue, 0))
	{
		DEBUG_PRINTF("UART Driver installation failed!");
	}


	//auto receive_event_handler = std::bind(&OBDIICnCI2CDiagnosisDevice::on_i2c_receive, this);

	//m_o_i2c_interface.onReceive(std::bind(&OBDIICnCI2CDiagnosisDevice::on_i2c_receive, this, _1, _2)); // register event
	//this->thread_diagnosis_reader = nullptr;

	//p_o_instance = this;
	this->thread_diagnosis_reader = new std_ex::thread(&OBDIICnCUARTDiagnosisDevice::thread_diagnosis_reader_main, this);
}

uart_port_t OBDIICnCUARTDiagnosisDevice::get_uart_port_num() const
{
	return m_en_uart_port_num;
}

OBDIICnCUARTDiagnosisDevice::~OBDIICnCUARTDiagnosisDevice()
{
	if (nullptr != this->thread_diagnosis_reader)
	{
		this->thread_diagnosis_reader->join();
		delete(this->thread_diagnosis_reader);
	}
	//p_o_instance = nullptr;
}


int OBDIICnCUARTDiagnosisDevice::connect()
{
	return 0;

}
int OBDIICnCUARTDiagnosisDevice::disconnect()
{
	return 0;
}

std::string OBDIICnCUARTDiagnosisDevice::get_release_label() const
{
	return "obd_ii_cnc_uart_diagnosis_device_v00.01.00";
}

int OBDIICnCUARTDiagnosisDevice::get_protocol_version_major() const
{
	return 1;
}

int OBDIICnCUARTDiagnosisDevice::get_protocol_version_minor() const
{
	return 0;
}

DiagnosisDeviceConnectionState OBDIICnCUARTDiagnosisDevice::get_communication_state() const
{
#if 0
	if (this->connected)
	{
		return DiagnosisDeviceConnectionState::DIAGNOSIS_DEVICE_COMMUNICATION_OK;
	}
#endif
	return DiagnosisDeviceConnectionState::DIAGNOSIS_DEVICE_COMMUNICATION_OK;
}

const OBDDataList& OBDIICnCUARTDiagnosisDevice::get_obd_data() const
{
	return this->ao_obd_data;
}

std::vector<OBDErrorCode> OBDIICnCUARTDiagnosisDevice::get_error_codes() const
{
	return this->error_codes;
}

int OBDIICnCUARTDiagnosisDevice::clear_error_code(const OBDErrorCode& code)
{
	return 0;
}

int32_t OBDIICnCUARTDiagnosisDevice::parse_uart_data_line(const std::string &line)
{

	const uint32_t cu32_num_of_values_per_data_line = 8u;

	// Values are all comma seperated
	std::vector<std::string> a_str_csv;
	std::stringstream ss(line);
	std::string str_line;
	while(std::getline(ss, str_line, ','))
	{
		a_str_csv.push_back(str_line);
	}

	if (a_str_csv.size() != cu32_num_of_values_per_data_line * 2u + 2u)
	{
		DEBUG_PRINTF("Line was discarded (size mismatch: " + std_ex::to_string(a_str_csv.size()));
		return -1;
	}

	if (a_str_csv[0] != "DATA" || a_str_csv[1] != "TIME")
	{
		DEBUG_PRINTF("First few fields invalid!");
		return -2;
	}

	std::vector<std::pair<uint8_t, double>> ao_obd_values(8);

	/*
	 * Two entries belong together, the first one MAY carry a sign (not necessarily), the
	 one contains the numeric value. Cast everything to float
	 */

	for (uint32_t i = 0; i < cu32_num_of_values_per_data_line; ++i)
	{
		std::string std_value = a_str_csv[2 + i*2] + a_str_csv[2 + i*2 + 1];

		// TODO switch to C++11
		ao_obd_values[i].second = atof(std_value.c_str());
		//ao_obd_values[i].second = std::stod(std_value);
	}



	if (ao_obd_values[0].second > 120.0f)
	{
		// Bvo, Ect, Bam, Spd, Bte, Fin, Soc, Afr
		ao_obd_values[0].first = 0x0A;
		ao_obd_values[1].first = 0x41;
		ao_obd_values[2].first = 0x0D;
		ao_obd_values[3].first = 0x11;
		ao_obd_values[4].first = 0x0E;
		ao_obd_values[5].first = 0x4D;
		ao_obd_values[6].first = 0x0C;
		ao_obd_values[7].first = 0x51;
	}
	else
	{
		// just take some random values
		ao_obd_values[0].first = 0x22;
		ao_obd_values[1].first = 0x23;
		ao_obd_values[2].first = 0x24;
		ao_obd_values[3].first = 0x25;
		ao_obd_values[4].first = 0x26;
		ao_obd_values[5].first = 0x27;
		ao_obd_values[6].first = 0x28;
		ao_obd_values[7].first = 0x29;
	}

	std::vector<OBDDataList::const_iterator> acitr;
	{
		// Lock the mutex list and update the values
		std::lock_guard<std::mutex> lk(m_o_obd_data_mutex);
	    for (auto i : ao_obd_values)
	    {
	    	auto itr = OBDataListHelper::get_element_by_identifier(this->ao_obd_data, i.first);
	    	if (itr == this->ao_obd_data.end())
	    	{
	    		DEBUG_PRINTF("ERROR Value not found: " + std_ex::to_string(i.first));
	    		continue;
	    	}

	    	DEBUG_PRINTF("Successfully updated " + std::string(itr->description) + " from value " + std_ex::to_string(itr->value_f) + " to " + std_ex::to_string(i.second));
	    	itr->value_f = static_cast<float>(i.second);


	    	OBDDataList::const_iterator citr = itr;
	    	acitr.push_back(citr);
	    }
	}
    // Send signal that data has changed
	this->sig_obd_data_received(acitr);
}

void OBDIICnCUARTDiagnosisDevice::parse_received_uart_data(const std::vector<uint8_t> &cau8_buffer)
{
	if (cau8_buffer.size() == 0 || NULL != cau8_buffer.back())
	{
		DEBUG_PRINTF("Passed buffer is not NULL-terminated, ignoring!");
		return;
	}
	m_str_previous_content += std::string(cau8_buffer.begin(), cau8_buffer.begin() + strlen(reinterpret_cast<const char*>(cau8_buffer.data())));

	// Split at newline (0xD) characters
	std::istringstream ss(m_str_previous_content);
	std::string str_line;
	std::vector<std::string> a_str_items;


	while(std::getline(ss, str_line, static_cast<char>(0xD)))
	{
		a_str_items.push_back(str_line);
	}

	if (a_str_items.size() > 1)
	{
		// TODO try to work with ranges here
		for (auto itr = a_str_items.begin(); itr != a_str_items.end() - 1; ++itr)
		{
			parse_uart_data_line(*itr);
		}
	}
	// Keep the remaining what is yet unparsed for future use.
	m_str_previous_content = a_str_items.back();

	// Discard remaining data if it gets too large
	if (m_str_previous_content.size() > 100)
	{
		DEBUG_PRINTF("Data was discarded: " + m_str_previous_content);
		m_str_previous_content.clear();
	}
}

void OBDIICnCUARTDiagnosisDevice::thread_diagnosis_reader_main()
{
	/* This thread does the reading of the OBD data */
	std::vector<uint8_t> data(128);

	while(true)
	{


		uint32_t u32_length = 0;
		int32_t i32_ret_code = 0;

		// Peek to retrieve the buffer length
		if (ESP_OK == uart_get_buffered_data_len(get_uart_port_num(), &u32_length))
		{
			// Make sure we don't write over the buffer
			u32_length = std::min(u32_length, data.size() - 1);

			//DEBUG_PRINTF("Peek length on UART is " + std_ex::to_string(u32_length));
			// actually receive
			std::fill(data.begin(), data.end(), 0);
			i32_ret_code = uart_read_bytes(get_uart_port_num(), data.data(), u32_length, 100);
		}

		if (i32_ret_code > 0)
		{
			// Make sure the string is null-terminated
			data.back() = '\0';
			//DEBUG_PRINTF(std::string(data.begin(), data.end()));
			parse_received_uart_data(data);
		}

		//char* test_str = "This is a test string.\n";
		//uart_write_bytes(get_uart_port_num(), (const char*)test_str, strlen(test_str));

#if 0
		uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		DEBUG_PRINTF("Current stack is: " + std_ex::to_string(uxHighWaterMark) + " bytes.");

		uint32_t u32FreeHeapSize = xPortGetFreeHeapSize();
		DEBUG_PRINTF("Current free heap is: " + std_ex::to_string(u32FreeHeapSize) + " bytes.");
#endif
		//heap_caps_get_free_size(MALLOC_CAP_8BIT)

		// Wait a bit to do load balancing
		std_ex::sleep_for(std::chrono::milliseconds(50));


		//this->update_data();
		//std_ex::sleep_for(std::chrono::milliseconds(100));
	}
}
