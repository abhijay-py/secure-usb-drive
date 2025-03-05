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


#define ACK_SUCCESS 0x00 // Operation successfully
#define ACK_FAIL 0x01 // Operation failed
#define ACK_FULL 0x04 // Fingerprint database is full
#define ACK_NOUSER 0x05 // No such user
#define ACK_USER_EXIST 0x06 // User already exists
#define ACK_FIN_EXIST 0x07 // Fingerprint already exists
#define ACK_TIMEOUT 0x08 // Acquisition timeout

Fingerprint_Status delete_specified_user(uint16_t user_id, uint8_t* ack_type);
Fingerprint_Status compare_1_1(uint16_t user_id, uint8_t* ack_type);
Fingerprint_Status add_fingerprint(
	unsigned int press_num,
	uint16_t user_id, uint8_t user_privilege,
	uint8_t* ack_type
);
Fingerprint_Status set_timeout(uint8_t timeout_value, uint8_t* ack_type);

#endif /* INC_FINGERPRINT_SENSOR_H_ */
