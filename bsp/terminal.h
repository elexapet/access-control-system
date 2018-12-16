/*
 * card_reader.h
 *
 *  Created on: 14. 10. 2018
 *      Author: Petr
 */

#ifndef BSP_TERMINAL_H_
#define BSP_TERMINAL_H_

#include <stdint.h>

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
  uint8_t open_time_sec : 8;
} panel_conf_t; //TOTAL SIZE 6B
#pragma pack(pop)

void terminal_init(void);

void terminal_reconfigure(panel_conf_t * acc_panel, uint8_t id);

#endif /* BSP_TERMINAL_H_ */
