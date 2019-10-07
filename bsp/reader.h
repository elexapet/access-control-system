/*
 * This RFID reader implementation uses Weigand26 card reader protocol.
 */

#ifndef BSP_READER_H_
#define BSP_READER_H_

#include <stdint.h>
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
  reader_mode_t mode;
  uint8_t enabled;
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


extern reader_conf_t reader_conf[ACS_READER_COUNT];

void reader_init(uint8_t id);

void reader_deinit(uint8_t id);

// user_id is 24-bit number
int8_t reader_get_request_from_buffer(uint32_t * user_id);

void reader_unlock(uint8_t id, bool with_beep, bool with_ok_led);


#endif /* BSP_READER_H_ */
