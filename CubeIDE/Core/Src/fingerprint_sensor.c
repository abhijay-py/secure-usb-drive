/*
 * fingerprint_sensor.c
 *
 *  Created on: Feb 18, 2025
 *      Author: chaos
 */

#include "fingerprint_sensor.h"
#include "main.h"

#define MAX_SIZE 12 // TODO: Change later
uint8_t tx_buff[MAX_SIZE];

static void send_command(const uint8_t data[], const uint16_t data_length) {
	// Header
	tx_buff[0] = 0xEF;
	tx_buff[1] = 0x01;

	// Adder
	for (int i = 0; i < 4; i++) {
		tx_buff[i+2] = 0xFF;
	}

	uint16_t sum = 0;

	// Package identifier
	tx_buff[6] = 0x01;
	sum += pid;

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

void get_img() {
	send_command({0x01}, 1);
}

