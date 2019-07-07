/*
 * This access panel implementation uses Weigand26 card reader protocol.
 */

#ifndef BSP_PANEL_H_
#define BSP_PANEL_H_

#include <stdint.h>
#include "board.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "timers.h"
#include "weigand.h"

typedef enum
{
  PANEL_MODE_DEF = 0,
  PANEL_MODE_LOCKED,
  PANEL_MODE_LEARN
} panel_mode_t;

typedef struct
{
  TimerHandle_t timer_open;
  TimerHandle_t timer_ok;
  uint16_t open_time_sec;
  uint16_t gled_time_sec;
  panel_mode_t mode;
  uint8_t enabled;
} panel_conf_t;

typedef struct
{
  uint8_t acc_panel_port;
  uint8_t acc_panel_d0_pin;
  uint8_t acc_panel_d1_pin;
  uint8_t acc_panel_beep_port;
  uint8_t acc_panel_beep_pin;
  uint8_t acc_panel_gled_port;
  uint8_t acc_panel_gled_pin;
  uint8_t acc_panel_rled_port;
  uint8_t acc_panel_rled_pin;
  uint8_t relay_port;
  uint8_t relay_pin;
  uint8_t sensor_port;
  uint8_t sensor_pin;
} panel_wiring_t;


extern panel_conf_t panel_conf[DOOR_ACC_PANEL_COUNT];

void panel_init(uint8_t id);

void panel_deinit(uint8_t id);

// user_id is 24-bit number
int8_t panel_get_request_from_buffer(uint32_t * user_id);

void panel_unlock(uint8_t id, bool with_beep, bool with_ok_led);


#endif /* BSP_PANEL_H_ */
