/*
 * fingerprint_sensor.c
 *
 *  Created on: Feb 18, 2025
 *      Author: chaos
 */

#include "fingerprint_sensor.h"
#include "main.h"

#define MAX_SIZE 12 // TODO: Change later

// Sends a command to the fingerprint sensor with Address = 0xFFFFFFFF,
// 0x01 package identifier, package contents = data.
// data_length corresponds to the length of data, not the package length.
static void send_command(const uint8_t data[], const uint16_t data_length) {
	uint8_t tx_buff[MAX_SIZE];

	// Header
	tx_buff[0] = 0xEF;
	tx_buff[1] = 0x01;

	// Address
	for (int i = 0; i < 4; i++) {
		tx_buff[i+2] = 0xFF;
	}

	uint16_t sum = 0;

	// Package identifier
	tx_buff[6] = 0x01;
	sum += 1;

	// Package length
	uint16_t length = data_length + 2;
	tx_buff[7] = (uint8_t)(length >> 8);
	tx_buff[8] = (uint8_t)length;
	sum += length;

	// Package contents
	for (int i = 0; i < data_length; i++) {
		tx_buff[i + 9] = data[i];
		sum += data[i];
	}

	// Checksum
	tx_buff[data_length + 9] = (uint8_t)(sum >> 8);
	tx_buff[data_length + 10] = (uint8_t)sum;

	HAL_UART_Transmit(&huart5, tx_buff, data_length + 11, 1000);
}

// See the GetImg command in the datasheet
void get_img() {
	const uint8_t data[] = {0x01};
	send_command(data, 1);
}

// See the GenChar command in the datasheet
void gen_char(const uint8_t buffer_id) {
	const uint8_t data[] = {0x02, buffer_id};
	send_command(data, 2);
}

// See the Search command in the datasheet
void search(const uint8_t buffer_id, const uint16_t start_page, const uint16_t num_pages) {
	const uint8_t data[] = {
		0x04,
		buffer_id,
		(uint8_t)(start_page >> 8),
		(uint8_t)start_page,
		(uint8_t)(num_pages >> 8),
		(uint8_t)num_pages
	};
	send_command(data, 6);
}

// See the RegModel command in the datasheet
void reg_model() {
	const uint8_t data[] = {0x05};
	send_command(data, 1);
}

// See the Store command in the datasheet
void store(const uint8_t buffer_id, const uint16_t page_id) {
	const uint8_t data[] = {
		0x06,
		buffer_id,
		(uint8_t)(page_id >> 8),
		(uint8_t)page_id
	};
	send_command(data, 4);
}
