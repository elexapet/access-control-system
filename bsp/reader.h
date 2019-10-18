/**
 *  @file
 *  @brief RFID card reader driver.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#ifndef BSP_READER_H_
#define BSP_READER_H_

#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "timers.h"
#include "weigand.h"

typedef enum
{
  READER_MODE_DEF = 0,
  READER_MODE_LOCKED,
  READER_MODE_LEARN
} reader_mode_t;

typedef struct
{
  TimerHandle_t timer_open;
  TimerHandle_t timer_ok;
  uint16_t open_time_sec;
  uint16_t gled_time_sec;
  uint8_t enabled;
  uint8_t door_open;
} reader_conf_t;

typedef struct
{
  uint8_t data_port;
  uint8_t d0_pin;
  uint8_t d1_pin;
  uint8_t beep_port;
  uint8_t beep_pin;
  uint8_t gled_port;
  uint8_t gled_pin;
  uint8_t rled_port;
  uint8_t rled_pin;
  uint8_t relay_port;
  uint8_t relay_pin;
  uint8_t sensor_port;
  uint8_t sensor_pin;
} reader_wiring_t;

/**
 * @brief Configuration of readers
 */
extern reader_conf_t reader_conf[ACS_READER_COUNT];

/**
 * @brief Initialize reader driver.
 *
 */
void reader_init(uint8_t idx);

/**
 * @brief Disable reader driver.
 *
 * @param idx ... reader index
 */
void reader_deinit(uint8_t idx);


/**
 * @brief Disable reader driver.
 *
 * @param user_id ... 24-bit number
 * @param idx ... reader index
 * @param ... time_to_wait_ms ... time to wait for request
 */
int8_t reader_get_request_from_buffer(uint32_t * user_id, uint16_t time_to_wait_ms);

/**
 * @brief Unlock door belonging to reader.
 *
 * @param idx ... reader index
 * @param with_beep ... true for sound signal
 * @param with_ok_led ... true for visual signal
 */
void reader_unlock(uint8_t idx, bool with_beep, bool with_ok_led);

/**
 * @brief Check door status.
 *
 * @param idx ... reader index
 *
 * @return true if door is open
 */
bool reader_is_door_open(uint8_t reader_idx);

#endif /* BSP_READER_H_ */
