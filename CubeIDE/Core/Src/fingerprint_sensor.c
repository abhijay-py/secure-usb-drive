/*
 * fingerprint_sensor.c
 *
 *  Created on: Feb 18, 2025
 *      Author: chaos
 */

#include "fingerprint_sensor.h"
#include "main.h"

#define PACKET_SIZE 8 // This file only supports sending / receiving 8 byte packets

// Calculates the checksum that is sent as the 7th byte of the UART packet
static uint16_t calculate_checksum(const uint8_t buff[]) {
	return buff[2] ^ buff[3] ^ buff[4] ^ buff[5] ^ buff[6];
}

// Generates an 8 byte package and stores it inside buff (see page 3 of the datasheet).
static void generate_package(
	uint8_t buff[],
	const uint8_t cmd,
	const uint8_t p1, const uint8_t p2, const uint8_t p3
) {
	buff[0] = 0xF5;
	buff[1] = cmd;
	buff[2] = p1;
	buff[3] = p2;
	buff[4] = p3;
	buff[5] = 0;
	buff[6] = calculate_checksum(buff);
	buff[7] = 0xF5;
}

// Sends an 8 byte command to the fingerprint sensor.
// Returns FINGERPRINT_OK or the type of error
static Fingerprint_Status send_command(
	const uint8_t cmd,
	const uint8_t p1, const uint8_t p2, const uint8_t p3
) {
	// Generate tx_buff
	uint8_t tx_buff[PACKET_SIZE];
	generate_package(tx_buff, cmd, p1, p2, p3);

	// Transmit tx_buff
	const uint16_t tx_buff_size = calculate_buff_size(tx_data_length);
	const HAL_StatusTypeDef tx_result
		= HAL_UART_Transmit(&huart5, tx_buff, PACKET_SIZE, 1000);
	return tx_result != HAL_OK ? FINGERPRINT_HAL_ERROR : FINGERPRINT_OK;
}

// Receives an 8 byte acknowledgment packet from the fingerprint sensor, and
// stores the return values into the q1, q2, and q3 return parameters.
// Returns FINGERPRINT_OK or the type of error
static Fingerprint_Status receive_acknowledgment(
	const uint8_t cmd,
	uint8_t* q1, uint8_t* q2, uint8_t* q3
) {
	// Receive UART data and put it in rx_buff
	uint8_t rx_buff[PACKET_SIZE];
	const HAL_StatusTypeDef rx_result
		= HAL_UART_Receive(&huart5, rx_buff, PACKET_SIZE, 1000);
	if (rx_result != HAL_OK) {
		return FINGERPRINT_HAL_ERROR;
	}

	// Generate the expected result
	uint8_t exp_rx_buff[PACKET_SIZE];
	// I don't know what I expect to get for q1, q2, and q3, so I set them all to 0.
	generate_package(exp_rx_buff, cmd, 0, 0, 0);

	// Check the UART data against the expected result
	// Check the first, last, cmd, and zero bytes
	if (
		exp_rx_buff[0] != rx_buff[0]
		|| exp_rx_buff[1] != rx_buff[1]
		|| exp_rx_buff[5] != rx_buff[5]
		|| exp_rx_buff[7] != rx_buff[7]
	) {
		return FINGERPRINT_RX_ERROR;
	}

	// Check the checksum
	if (rx_buff[6] != calculate_checksum(rx_buff)) {
		return FINGERPRINT_RX_ERROR;
	}

	// If the code gets to here then all the checks have passed,
	// so we can set q1, q2, and q3 accordingly.
	if (q1 != NULL) {
		*q1 = rx_buff[2];
	}
	if (q2 != NULL) {
		*q2 = rx_buff[3];
	}
	if (q3 != NULL) {
		*q3 = rx_buff[4];
	}

	return FINGERPRINT_OK;
}

// Sends an 8 byte command to the fingerprint sensor.
// Then receives an 8 byte acknowledgment packet from the
// fingerprint sensor and sets q1, q2, and q3 accordingly.
// Returns FINGERPRINT_OK or the type of error
static Fingerprint_Status send_and_receive_command(
	const uint8_t cmd,
	const uint8_t p1, const uint8_t p2, const uint8_t p3,
	uint8_t* q1, uint8_t* q2, uint8_t* q3
) {
	Fingerprint_Status tx_result = send_command(cmd, p1, p2, p3);
	if (tx_result != FINGERPRINT_OK) {
		return tx_result;
	}

	return receive_acknowledgment(cmd, q1, q2, q3);
}

// Sends the delete specified user (2.4) command in the datasheet.
// ack_type is called q3 in the datasheet.
Fingerprint_Status delete_specified_user(uint16_t user_id, uint8_t* ack_type) {
	return send_and_receive_command(
		0x04,
		(uint8_t)(user_id >> 8), (uint8_t)user_id, 0,
		NULL, NULL, ack_type
	);
}

// Sends the compare 1:1 (2.7) command in the datasheet.
// ack_type is called q3 in the datasheet.
Fingerprint_Status compare_1_1(uint16_t user_id, uint8_t* ack_type) {
	return send_and_receive_command(
		0x0B,
		(uint8_t)(user_id >> 8), (uint8_t)user_id, 0,
		NULL, NULL, ack_type
	);
}

// Sends the add_fingerprint (2.7) command in the datasheet.
// press_num is 1 if it's the first fingerprint input,
// 2 if it's the second fingerprint input, and 3 if it's the third fingerprint input.
// ack_type is called q3 in the datasheet.
Fingerprint_Status add_fingerprint(
	unsigned int press_num,
	uint16_t user_id, uint8_t user_privilege,
	uint8_t* ack_type
) {
	if (press_num < 1 || press_num > 3) {
		return FINGERPRINT_INVALID_PARAM;
	}
	return send_and_receive_command(
		press_num,
		(uint8_t)(user_id >> 8), (uint8_t)user_id, user_privilege,
		NULL, NULL, ack_type
	);
}
