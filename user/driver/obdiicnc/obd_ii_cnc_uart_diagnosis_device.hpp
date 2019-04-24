
#ifndef _OBD_II_CNC_UART_DIAGNOSIS_DEVICE_HPP_
#define _OBD_II_CNC_UART_DIAGNOSIS_DEVICE_HPP_

/* System header files */
#include <memory>
#include <vector>
#include "thread.hpp"
#include <driver/uart.h>
#include <mutex>

/* Other modules */

/* Own header */
#include "diagnosis_device_interface.hpp"
#include "obd_types/obd_error_code.h"


/** This is a dummy diagnosis device implementation, to be used as long as no real OBD II
 * diagnosis device is available. It just returns random OBD II values and randomly occuring
 * error codes.
 *
 * Use this dummy diagnosis device for testing transmitting data via Bluetooth BLE */
class OBDIICnCUARTDiagnosisDevice : public DiagnosisDeviceInterface
{
public:
	OBDIICnCUARTDiagnosisDevice(uint32_t u32_uart_controller_id, uint32_t u32_rx_pin, uint32_t u32_tx_pin, const uart_config_t &st_uart_configuration);
	virtual ~OBDIICnCUARTDiagnosisDevice();

	virtual int connect();
	virtual int disconnect();

	virtual std::string get_release_label() const;

	virtual int get_protocol_version_major() const;
	virtual int get_protocol_version_minor() const;

	virtual DiagnosisDeviceConnectionState get_communication_state() const;

	virtual const OBDDataList& get_obd_data() const;

	virtual std::vector<OBDErrorCode> get_error_codes() const;

	virtual int clear_error_code(const OBDErrorCode& code);

	void process_message(uint8_t *buffer, uint32_t u32_buffer_size);
private:
	const uart_config_t m_st_uart_config;
	const uint32_t m_u32_rx_pin;
	const uint32_t m_u32_tx_pin;

	uart_port_t m_en_uart_port_num;

	uart_port_t get_uart_port_num() const;

	QueueHandle_t m_uart_queue;

	/** Flag to indicate if the diagnosis device is connected */
	bool connected;

	/** The currently present OBD II error codes on the vehicle */
	std::vector<OBDErrorCode> error_codes;

	/** the current read OBD II data */
	OBDDataList ao_obd_data;

	std_ex::thread* thread_diagnosis_reader;

	std::string m_str_previous_content;

	void thread_diagnosis_reader_main();

	int32_t parse_uart_data_line(const std::string &line);
	void parse_received_uart_data(const std::vector<uint8_t> &cau8_buffer);

	std::mutex m_o_obd_data_mutex;


};

#endif /* _OBD_II_CNC_I2C_DIAGNOSIS_DEVICE_HPP_ */
