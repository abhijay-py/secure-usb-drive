/*
 * fingerprint_sensor.h
 *
 *  Created on: Feb 18, 2025
 *      Author: chaos
 */

#ifndef INC_FINGERPRINT_SENSOR_H_
#define INC_FINGERPRINT_SENSOR_H_

#include <stdint.h>

typedef enum {
	FINGERPRINT_OK,
	FINGERPRINT_HAL_ERROR,
	FINGERPRINT_RX_ERROR,
	FINGERPRINT_INVALID_PARAM
} Fingerprint_Status;

const uint8_t ACK_SUCCESS = 0x00;

Fingerprint_Status delete_specified_user(uint16_t user_id, uint8_t* ack_type);
Fingerprint_Status compare_1_1(uint16_t user_id, uint8_t* ack_type);
Fingerprint_Status add_fingerprint(
	unsigned int press_num,
	uint16_t user_id, uint8_t user_privilege,
	uint8_t* ack_type
);

#endif /* INC_FINGERPRINT_SENSOR_H_ */
