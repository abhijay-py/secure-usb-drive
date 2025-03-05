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
	FINGERPRINT_RX_ERROR
} Fingerprint_Status;

Fingerprint_Status get_img(uint8_t* confirmation_code);
Fingerprint_Status gen_char(const uint8_t buffer_id, uint8_t* confirmation_code);
Fingerprint_Status search(
	const uint8_t buffer_id, const uint16_t start_page, const uint16_t num_pages,
	uint8_t* confirmation_code, uint16_t* page_id, uint16_t* match_score
);
Fingerprint_Status reg_model(uint8_t* confirmation_code);
Fingerprint_Status store(
	const uint8_t buffer_id, const uint16_t page_id,
	uint8_t* confirmation_code
);
Fingerprint_Status write_reg(
	const uint8_t register_number, const uint8_t contents,
	uint8_t* confirmation_code
);

#endif /* INC_FINGERPRINT_SENSOR_H_ */
