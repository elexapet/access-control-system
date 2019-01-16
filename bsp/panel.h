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

#pragma pack(push, 1)
typedef struct
{
  uint8_t acc_panel_on : 1;
  uint8_t acc_panel_port : 4;
  uint8_t acc_panel_d0_pin : 4;
  uint8_t acc_panel_d1_pin : 4;
  uint8_t acc_panel_beep_port : 4;
  uint8_t acc_panel_beep_pin : 4;
  uint8_t acc_panel_gled_port : 4;
  uint8_t acc_panel_gled_pin : 4;
  uint8_t relay_port : 4;
  uint8_t relay_pin : 4;
  uint16_t open_time_sec;
  StreamBufferHandle_t card_stream;
  TimerHandle_t timer;
} panel_conf_t; //TOTAL SIZE 6B
#pragma pack(pop)

extern panel_conf_t acc_panel[DOOR_ACC_PANEL_MAX_COUNT];

void panel_init(uint8_t id);

void panel_deinit(uint8_t id);

void panel_unlock(uint8_t id);


#endif /* BSP_PANEL_H_ */