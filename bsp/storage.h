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

#define STORE_I2C_DEV I2C0

#define STORE_DEV_BUSY_FOR 50

// Initialize I2C bus for storage
void storage_init(void);

// Read a word from address (little-endian)
bool storage_read_word_le(const uint8_t addr, uint16_t * data);

// Write a word to address (little-endian)
bool storage_write_word_le(const uint8_t addr, const uint16_t data);

// Read a byte from address
bool storage_read_byte(const uint8_t addr, uint8_t * data);

// Write a byte to address
bool storage_write_byte(const uint8_t addr, const uint8_t data);

#endif /* BSP_STORAGE_H_ */
