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

#define ACC_PANEL_0 0
#define ACC_PANEL_1 1
#define ACC_PANEL_2 2

typedef struct
{
  TimerHandle_t timer;
  uint16_t open_time_sec;
  uint8_t acc_panel_port : 4;
  uint8_t acc_panel_d0_pin : 4;
  uint8_t acc_panel_d1_pin : 4;
  uint8_t acc_panel_beep_port : 4;
  uint8_t acc_panel_beep_pin : 4;
  uint8_t acc_panel_gled_port : 4;
  uint8_t acc_panel_gled_pin : 4;
  uint8_t relay_port : 4;
  uint8_t relay_pin : 4;
  uint8_t acc_panel_on : 1;
} panel_conf_t; //TOTAL SIZE 6B

extern panel_conf_t acc_panel[DOOR_ACC_PANEL_MAX_COUNT];

void panel_init(uint8_t id);

void panel_deinit(uint8_t id);

// user_id is 24-bit number
int8_t panel_get_request_from_buffer(uint32_t * user_id);

void panel_unlock(uint8_t id, bool with_beep, bool with_ok_led);


#endif /* BSP_PANEL_H_ */
