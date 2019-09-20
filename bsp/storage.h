/*
 * storage.h
 *
 *  Created on: 20. 9. 2019
 *      Author: Petr
 */

#ifndef BSP_STORAGE_H_
#define BSP_STORAGE_H_

#include "board.h"
#include <stdint.h>
#include <stdbool.h>

void storage_init(I2C_ID_T dev_id, uint32_t freq);

bool storage_read_word_le(const uint8_t addr, uint16_t * data);

bool storage_write_word_le(const uint8_t addr, const uint16_t data);

bool storage_read_byte(const uint8_t addr, uint8_t * data);

bool storage_write_byte(const uint8_t addr, const uint8_t data);

#endif /* BSP_STORAGE_H_ */
