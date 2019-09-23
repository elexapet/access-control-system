/*
 * terminal_config.h
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 */

#ifndef BSP_BOARD_CONFIG_H_
#define BSP_BOARD_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

//-------------------------------------------------------------
// General settings

#define DEVEL_BOARD

// enable caching for operation when communication is lost
// cache expiration is to be handled by master
#define CACHING_ENABLED 0

// CAN bus speed b/s
#define CAN_BAUD_RATE 125000

//-------------------------------------------------------------
// global UID from vendor (from chip)
#define IAP_READ_UID 58
extern unsigned int TERMINAL_UID[5]; // 0 - status code, 1 - least significant word

//-------------------------------------------------------------
// Door addresses in ACS system
//
// Read from external storage on startup
// Designed to be even (address A) and odd (address B)
// Always true: ADDR_B = ADDR_A + 1

// Address getters
extern uint16_t get_acs_panel_a_addr(void);
extern uint16_t get_acs_panel_b_addr(void);
// Address setter
void set_acs_panel_addr(const uint16_t first_acs_addr);

// Expected organization in external address space:
// | 0x00 | A_ADDR [7:0]
// | 0x01 | A_ADDR [9:8]
//          PADDING [15:10]

// The address actually uses less then 16 bits. See address bit width in ACS protocol.

#define PTR_ACS_PANEL_FIRST_ADDR 0x0 // pointer to external memory


// Setting for I2C external storage for door adresses
// (EEPROM, IO EXPANDER or device with same access)
#define STORE_I2C_DEV        I2C0
#define STORE_I2C_BUS_FREQ   100000 // 400kHz MAX
#define STORE_I2C_SLAVE_ADDR 0x50   // 7bit address

//-------------------------------------------------------------

// internal number of card readers
#define ACC_PANEL_A 0
#define ACC_PANEL_B 1

// card reader count 1/2
#define DOOR_ACC_PANEL_COUNT 2

#define BEEP_ON_SUCCESS false
#define OK_LED_ON_SUCCESS true

// communication status led
#define ACS_PANEL_STATUS_LED_PORT  0
#define ACS_PANEL_STATUS_LED_PIN   6

//---------------------------------------------------------------------------------------------------------------------
// Settings for panel A
//---------------------------------------------------------------------------------------------------------------------
#define DOOR_1_ACC_PANEL_ON         true

#define DOOR_1_ACC_PANEL_PORT       3 // Port must be unique for each access panel (reader)
                                      // Can be only 2 or 3.
#define DOOR_1_ACC_PANEL_D0_PIN     2
#define DOOR_1_ACC_PANEL_D1_PIN     1
#define DOOR_1_ACC_PANEL_BEEP_PORT  2
#define DOOR_1_ACC_PANEL_BEEP_PIN   7
#define DOOR_1_ACC_PANEL_GLED_PORT  2
#define DOOR_1_ACC_PANEL_GLED_PIN   1
#define DOOR_1_ACC_PANEL_RLED_PORT  2
#define DOOR_1_ACC_PANEL_RLED_PIN   0
#define DOOR_1_RELAY_PORT           1
#define DOOR_1_RELAY_PIN            10
#define DOOR_1_SENSOR_PORT          1
#define DOOR_1_SENSOR_PIN           8

#define DOOR_1_OPEN_TIME_MS         8000
#define DOOR_1_OK_GLED_TIME_MS      3000

//---------------------------------------------------------------------------------------------------------------------
// Settings for panel B
//---------------------------------------------------------------------------------------------------------------------
#define DOOR_2_ACC_PANEL_ON         true

#define DOOR_2_ACC_PANEL_PORT       2 // Port must be unique for each access panel (reader)
                                      // Can be only 2 or 3.
#define DOOR_2_ACC_PANEL_D0_PIN     8
#define DOOR_2_ACC_PANEL_D1_PIN     6
#define DOOR_2_ACC_PANEL_BEEP_PORT  2
#define DOOR_2_ACC_PANEL_BEEP_PIN   10
#define DOOR_2_ACC_PANEL_GLED_PORT  2
#define DOOR_2_ACC_PANEL_GLED_PIN   3
#define DOOR_2_ACC_PANEL_RLED_PORT  2
#define DOOR_2_ACC_PANEL_RLED_PIN   2
#define DOOR_2_RELAY_PORT           1
#define DOOR_2_RELAY_PIN            11
#define DOOR_2_SENSOR_PORT          1
#define DOOR_2_SENSOR_PIN           5

#define DOOR_2_OPEN_TIME_MS         8000
#define DOOR_2_OK_GLED_TIME_MS      3000

//---------------------------------------------------------------------------------------------------------------------
// Internal setting

// if following is changed it will need code modification
#define WEIGAND_DEVICE_LIMIT 4
#define SERIAL_DEVICE_LIMIT 0
#define DOOR_ACC_PANEL_MAXCOUNT 2
#define TERMINAL_TIMER_ID 15

#if DOOR_ACC_PANEL_COUNT > DOOR_ACC_PANEL_MAXCOUNT
#error "reader max count limit"
#endif

//---------------------------------------------------------------------------------------------------------------------

#define PANEL_WATCHDOG_TIMEOUT 2 // in seconds

//---------------------------------------------------------------------------------------------------------------------

// IO
#define LOG_HIGH 1
#define LOG_LOW 0

//---------------------------------------------------------------------------------------------------------------------


// Initialize configuration
// @return true if succeeded
bool terminal_config_init(void);

//---------------------------------------------------------------------------------------------------------------------

#endif /* BSP_BOARD_CONFIG_H_ */
