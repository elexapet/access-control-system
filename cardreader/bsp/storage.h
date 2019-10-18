/**
 *  @file
 *  @brief Storage implementation.
 *
 *         Supports I2C EEPROMs and I/O expanders.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#ifndef BSP_STORAGE_H_
#define BSP_STORAGE_H_

#include "board.h"
#include <stdint.h>
#include <stdbool.h>

// I2C device to use
#define STORE_I2C_DEV I2C0


/**
 * @brief Initialize I2C bus for storage.
 *
 */
void storage_init(void);

/**
 * @brief Read a word from address (little-endian).
 *
 * @return true if succeeded
 */
bool storage_read_word_le(const uint8_t addr, uint16_t * data);

/**
 * @brief Write a word to address (little-endian).
 *
 * @return true if succeeded
 */
bool storage_write_word_le(const uint8_t addr, const uint16_t data);

/**
 * @brief Read a byte from address.
 *
 * @return true if succeeded
 */
bool storage_read_byte(const uint8_t addr, uint8_t * data);

/**
 * @brief Write a byte to address.
 *
 * @return true if succeeded
 */
bool storage_write_byte(const uint8_t addr, const uint8_t data);


#endif /* BSP_STORAGE_H_ */
