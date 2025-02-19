/*
 * fingerprint_sensor.h
 *
 *  Created on: Feb 18, 2025
 *      Author: chaos
 */

#ifndef INC_FINGERPRINT_SENSOR_H_
#define INC_FINGERPRINT_SENSOR_H_

void get_img();
void gen_char(const uint8_t buffer_id);
void search(const uint8_t buffer_id, const uint16_t start_page, const uint16_t num_pages);
void reg_model();
void store(const uint8_t buffer_id, const uint16_t page_id);

#endif /* INC_FINGERPRINT_SENSOR_H_ */
