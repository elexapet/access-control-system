/*
 * board_config.h
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 */

#ifndef BSP_BOARD_CONFIG_H_
#define BSP_BOARD_CONFIG_H_

#include "chip.h"


#define DEVEL_BOARD

// Use global UID from chip as terminal unique ID
#define IAP_READ_UID 58
extern unsigned int TERMINAL_UID[4]; // 0 is least significant word

//---------------------------------------------------------------------------------------------------------------------
// Values for door 0
//---------------------------------------------------------------------------------------------------------------------
#define DOOR_0_ACC_PANEL_ON         false

#define DOOR_0_ACC_PANEL_PORT       1 // Port must be unique for each access panel (reader)
#define DOOR_0_ACC_PANEL_D0_PIN     1
#define DOOR_0_ACC_PANEL_D1_PIN     2
#define DOOR_0_ACC_PANEL_BEEP_PORT  0
#define DOOR_0_ACC_PANEL_BEEP_PIN   0
#define DOOR_0_ACC_PANEL_GLED_PORT  0
#define DOOR_0_ACC_PANEL_GLED_PIN   0

#define DOOR_0_RELAY_PORT           0
#define DOOR_0_RELAY_PIN            0
#define DOOR_0_OPEN_TIME_MS         3000

//---------------------------------------------------------------------------------------------------------------------
// Values for door 1
//---------------------------------------------------------------------------------------------------------------------
#define DOOR_1_ACC_PANEL_ON         true

#define DOOR_1_ACC_PANEL_PORT       2 // Port must be unique for each access panel (reader)
#define DOOR_1_ACC_PANEL_D0_PIN     7
#define DOOR_1_ACC_PANEL_D1_PIN     8
#define DOOR_1_ACC_PANEL_BEEP_PORT  2
#define DOOR_1_ACC_PANEL_BEEP_PIN   3
#define DOOR_1_ACC_PANEL_GLED_PORT  2
#define DOOR_1_ACC_PANEL_GLED_PIN   6

#define DOOR_1_RELAY_PORT           2
#define DOOR_1_RELAY_PIN            10
#define DOOR_1_OPEN_TIME_MS         2000

//---------------------------------------------------------------------------------------------------------------------
// Values for door 2
//---------------------------------------------------------------------------------------------------------------------
#define DOOR_2_ACC_PANEL_ON         true

#define DOOR_2_ACC_PANEL_PORT       3 // Port must be unique for each access panel (reader)
#define DOOR_2_ACC_PANEL_D0_PIN     1
#define DOOR_2_ACC_PANEL_D1_PIN     2
#define DOOR_2_ACC_PANEL_BEEP_PORT  2
#define DOOR_2_ACC_PANEL_BEEP_PIN   0
#define DOOR_2_ACC_PANEL_GLED_PORT  2
#define DOOR_2_ACC_PANEL_GLED_PIN   1

#define DOOR_2_RELAY_PORT           2
#define DOOR_2_RELAY_PIN            2
#define DOOR_2_OPEN_TIME_MS         2000
//---------------------------------------------------------------------------------------------------------------------

#define BEEP_ON_SUCCESS true
#define OK_LED_ON_SUCCESS true

#define WEIGAND_DEVICE_LIMIT 4
#define SERIAL_DEVICE_LIMIT 0
#define DOOR_ACC_PANEL_MAX_COUNT (WEIGAND_DEVICE_LIMIT + SERIAL_DEVICE_LIMIT)

// IO
#define LOG_HIGH 1
#define LOG_LOW 0

#endif /* BSP_BOARD_CONFIG_H_ */
