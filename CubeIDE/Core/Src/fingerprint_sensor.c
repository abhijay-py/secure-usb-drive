/*
 * fingerprint_sensor.c
 *
 *  Created on: Feb 18, 2025
 *      Author: chaos
 */

#include "fingerprint_sensor.h"
#include "main.h"

#define MAX_TX_SIZE 17 // TODO: Change later
#define MAX_RX_SIZE 17 // TODO: Change later

const uint8_t COMMAND_PID = 0x01;
const uint8_t ACKNOWLEDGE_PID = 0x07;

typedef enum {
	FINGERPRINT_OK,
	FINGERPRINT_HAL_ERROR,
	FINGERPRINT_RX_ERROR
}; Fingerprint_Status;

// Calculates the checksum that is sent at the end of the UART packet
static uint16_t calculate_checksum(const uint8_t buff[], const uint16_t buff_size) {
	uint16_t sum = 0;
	for (int i = 6; i < buff_size - 2; i++) {
		sum += buff[i];
	}
	return sum;
}

// Calculate the buffer size from the size of the package contents
static inline uint16_t calculate_buff_size(const uint16_t data_length) {
	return data_length + 11;
}

// Generates a package with
// address = 0xFFFFFFFF, package identifier = pid, and package contents = data.
// This package gets stored into buff.
// data_length corresponds to the length of data, not the package length.
// If data == NULL, then the package contents will get set to 0.
static void generate_package(
	uint8_t buff[],
	const uint8_t pid,
	const uint8_t data[],
	const uint16_t data_length
) {
	// Header
	buff[0] = 0xEF;
	buff[1] = 0x01;

	// Address
	for (int i = 0; i < 4; i++) {
		buff[i + 2] = 0xFF;
	}

	// Package identifier
	buff[6] = pid;

	// Package length
	const uint16_t length = data_length + 2;
	buff[7] = (uint8_t)(length >> 8);
	buff[8] = (uint8_t)length;

	// Package contents
	for (int i = 0; i < data_length; i++) {
		buff[i + 9] = data == NULL ? 0 : data[i];
	}

	// Checksum
	const uint16_t sum = calculate_checksum(buff, calculate_buff_size(data_length));
	buff[data_length + 9] = (uint8_t)(sum >> 8);
	buff[data_length + 10] = (uint8_t)sum;
}

// Sends a command to the fingerprint sensor with
// address = 0xFFFFFFFF, package identifier = 0x01, and package contents = tx_data.
// tx_data_length corresponds to the length of tx_data, not the package length.
// Returns FINGERPRINT_OK or the type of error
static Fingerprint_Status send_command(
	const uint8_t tx_data[],
	const uint16_t tx_data_length
) {
	// Generate tx_buff
	uint8_t tx_buff[MAX_TX_SIZE];
	generate_package(tx_buff, COMMAND_PID, tx_data, tx_data_length);

	// Transmit tx_buff
	const uint16_t tx_buff_size = calculate_buff_size(tx_data_length);
	const HAL_StatusTypeDef tx_result
		= HAL_UART_Transmit_IT(&huart5, tx_buff, tx_buff_size);
	return tx_result != HAL_OK ? FINGERPRINT_HAL_ERROR : FINGERPRINT_OK;
}

// Receives an acknowledgment packet from the fingerprint sensor, and
// stores the package contents into rx_data.
// rx_data_length is the expected length of the package contents / rx_data,
// not the package length.
// Returns FINGERPRINT_OK or the type of error
static Fingerprint_Status receive_acknowledgment(
	uint8_t rx_data[],
	const uint8_t rx_data_length
) {
	// Receive UART data and put it in rx_buff
	uint8_t rx_buff[MAX_RX_SIZE];
	uint16_t rx_buff_size = calculate_buff_size(rx_data_length);
	const HAL_StatusTypeDef rx_result
		= HAL_UART_Receive(&huart5, rx_buff, rx_buff_size, 1000);

	// Generate the expected result
	uint8_t exp_rx_buff[MAX_RX_SIZE];
	// I don't know what I expect to get for the package contents so I make it NULL
	generate_package(exp_rx_buff, ACKNOWLEDGE_PID, NULL, rx_data_length);

	// Check the UART data against the expected result
	// Check everything before the package contents
	for (int i = 0; i < 9; i++) {
		if (rx_buff[i] != exp_rx_buff[i]) {
			return FINGERPRINT_RX_ERROR;
		}
	}
	// Check the checksum
	const uint16_t rx_sum = calculate_checksum(rx_buff, rx_buff_size);
	if (
		rx_buff[rx_buff_size - 2] != (uint8_t)(rx_sum >> 8)
		|| rx_buff[rx_buff_size - 1] != (uint8_t)rx_sum
	) {
		return FINGERPRINT_RX_ERROR;
	}

	// If the code gets to here then all the checks have passed,
	// so we can set rx_data equal to the package contents
	for (int i = 0; i < rx_data_length; i++) {
		rx_data[i] = rx_buff[i + 9];
	}
	return FINGERPRINT_OK;
}

// Sends a command to the fingerprint sensor with
// address = 0xFFFFFFFF, package identifier = 0x01, and package contents = tx_data.
// Then receives an acknowledgment packet from the
// fingerprint sensor with address and sets rx_data = package contents.
// tx_data_length corresponds to the length of tx_data, not the package length.
// rx_data_length corresponds to the expected length of rx_data, not the expected package length.
// Returns FINGERPRINT_OK or the type of error
static Fingerprint_Status send_and_receive_command(
	const uint8_t tx_data[],
	const uint16_t tx_data_length,
	uint8_t rx_data[],
	const uint8_t rx_data_length
) {
	if (send_command(tx_data, tx_data_length) != HAL_OK) {
		return FINGERPRINT_HAL_ERROR;
	}

	if (rx_result != HAL_OK) {
		return FINGERPRINT_HAL_ERROR;
	}




	for (int i = 0; i < rx_data_length; i++) {
		rx_data[i] = rx_buff[i + 9];
	}
}

// Sends the GetImg command in the datasheet
void get_img() {
	const uint8_t data[] = {0x01};
	send_and_receive_command(data, 1);
}

// Sends the GenChar command in the datasheet
void gen_char(const uint8_t buffer_id) {
	const uint8_t data[] = {0x02, buffer_id};
	send_and_receive_command(data, 2);
}

// Sends the Search command in the datasheet
void search(const uint8_t buffer_id, const uint16_t start_page, const uint16_t num_pages) {
	const uint8_t data[] = {
		0x04,
		buffer_id,
		(uint8_t)(start_page >> 8),
		(uint8_t)start_page,
		(uint8_t)(num_pages >> 8),
		(uint8_t)num_pages
	};
	send_and_receive_command(data, 6);
}

// Sends the RegModel command in the datasheet
void reg_model() {
	const uint8_t data[] = {0x05};
	send_and_receive_command(data, 1);
}

// Sends the Store command in the datasheet
void store(const uint8_t buffer_id, const uint16_t page_id) {
	const uint8_t data[] = {
		0x06,
		buffer_id,
		(uint8_t)(page_id >> 8),
		(uint8_t)page_id
	};
	send_and_receive_command(data, 4);
}

