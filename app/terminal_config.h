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
#define ENABLE_LOCAL_ACS_ADDR_WRITE 0

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
// Door addresses in a network for ACS system
//
// Read from external storage on startup
// Designed to be even (address A) and odd (address B)
// Always true: ADDR_B = ADDR_A + 1

// Address getters
extern uint16_t get_reader_a_addr(void);
extern uint16_t get_reader_b_addr(void);
// Address setter
void set_reader_addr(const uint16_t first_acs_addr);

// Expected organization in external address space:
// | 0x00 | A_ADDR [7:0]
// | 0x01 | A_ADDR [9:8]
//          PADDING [15:10]

// The address actually uses less then 16 bits. See address bit width in ACS protocol.

#define PTR_READER_FIRST_ADDR 0x0 // pointer to external memory


// Setting for I2C external storage for door adresses
// (EEPROM, IO EXPANDER or device with same access)
#define STORE_I2C_BUS_FREQ   400000 // 100kHz or 400kHz
#define STORE_I2C_SLAVE_ADDR 0x50   // 7bit address

//-------------------------------------------------------------

// internal number of card readers
#define ACS_READER_A_IDX 0
#define ACS_READER_B_IDX 1

// card reader count 1/2
#define ACS_READER_COUNT 2

#define BEEP_ON_SUCCESS false
#define OK_LED_ON_SUCCESS true

// communication status led
#define ACS_COMM_STATUS_LED_PORT  0
#define ACS_COMM_STATUS_LED_PIN   6

//---------------------------------------------------------------------------------------------------------------------
// Settings for RFID reader A
//---------------------------------------------------------------------------------------------------------------------
#define ACS_READER_A_ENABLED         true

#define ACS_READER_A_DATA_PORT       3 // Port must be unique for each reader
                                       // Can be only 2 or 3.
#define ACS_READER_A_D0_PIN     2
#define ACS_READER_A_D1_PIN     1
#define ACS_READER_A_BEEP_PORT  2
#define ACS_READER_A_BEEP_PIN   7
#define ACS_READER_A_GLED_PORT  2
#define ACS_READER_A_GLED_PIN   1
#define ACS_READER_A_RLED_PORT  2
#define ACS_READER_A_RLED_PIN   0
#define ACS_READER_A_RELAY_PORT           1
#define ACS_READER_A_RELAY_PIN            10
#define ACS_READER_A_SENSOR_PORT          1
#define ACS_READER_A_SENSOR_PIN           8

#define ACS_READER_A_OPEN_TIME_MS         8000
#define ACS_READER_A_OK_GLED_TIME_MS      4000

//---------------------------------------------------------------------------------------------------------------------
// Settings for RFID reader B
//---------------------------------------------------------------------------------------------------------------------
#define ACS_READER_B_ENABLED         true

#define ACS_READER_B_DATA_PORT       2 // Port must be unique for each reader
                                       // Can be only 2 or 3.
#define ACS_READER_B_D0_PIN     8
#define ACS_READER_B_D1_PIN     6
#define ACS_READER_B_BEEP_PORT  2
#define ACS_READER_B_BEEP_PIN   10
#define ACS_READER_B_GLED_PORT  2
#define ACS_READER_B_GLED_PIN   3
#define ACS_READER_B_RLED_PORT  2
#define ACS_READER_B_RLED_PIN   2
#define ACS_READER_B_RELAY_PORT           1
#define ACS_READER_B_RELAY_PIN            11
#define ACS_READER_B_SENSOR_PORT          1
#define ACS_READER_B_SENSOR_PIN           5

#define ACS_READER_B_OPEN_TIME_MS         8000
#define ACS_READER_B_OK_GLED_TIME_MS      4000

//---------------------------------------------------------------------------------------------------------------------
// Internal setting

// if following is changed it will need code modification
#define WEIGAND_DEVICE_LIMIT 4
#define SERIAL_DEVICE_LIMIT 0
#define ACS_READER_MAXCOUNT 2
#define TERMINAL_TIMER_ID 15

#if ACS_READER_COUNT > ACS_READER_MAXCOUNT
#error "reader max count limit"
#endif

//---------------------------------------------------------------------------------------------------------------------

#define HW_WATCHDOG_TIMEOUT 1 // in seconds

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
