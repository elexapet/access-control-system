/**
 *  @file
 *  @brief Terminal client configuration (hardware and software).
 * *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#ifndef TERM_CONFIG_H_
#define TERM_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

//#define DEVEL_BOARD // LPCXpresso11C24 development board

//-------------------------------------------------------------
// General settings.
//-------------------------------------------------------------

// Enable write of ACS address to external storage on startup.
#define ENABLE_LOCAL_ACS_ADDR_WRITE 0

// Enable caching for operation when communication is lost.
// Cache expiration is to be handled by master (Not implemented).
#define CACHING_ENABLED 0

// CAN bus speed b/s - affects maximum data cable length.
// Suggested option for common use are 100kbit/s and 125kbit/s.
#define CAN_BAUD_RATE 125000

// Setting for I2C external storage for door adresses
// (EEPROM, IO EXPANDER or device with same access).
#define STORE_I2C_BUS_FREQ   400000 // 100kHz or 400kHz
#define STORE_I2C_SLAVE_ADDR 0x50   // 7bit address

// Internal index of card readers (can be only swapped).
#define ACS_READER_A_IDX 0
#define ACS_READER_B_IDX 1

#define BEEP_ON_SUCCESS true
#define OK_LED_ON_SUCCESS true

// Door sensor types (reed switch).
#define SENSOR_IS_NO 0  // Normally open (Form A).
#define SENSOR_IS_NC 1  // Normally closed (Form B).

// If DOOR_SENSOR_TYPE is defined enables door sensor.
#define DOOR_SENSOR_TYPE SENSOR_IS_NO

// Communication status led.
#define ACS_COMM_STATUS_LED_PORT  0
#define ACS_COMM_STATUS_LED_PIN   6

//-------------------------------------------------------------
// Global UID from vendor (from chip).
//-------------------------------------------------------------
#define IAP_READ_UID 58
extern unsigned int TERMINAL_UID[5]; // 0 - status code, 1 - least significant word

//-------------------------------------------------------------
// Door addresses in a network for ACS system.
//-------------------------------------------------------------
//
// Read from external storage on startup
// Designed to be even (address A) and odd (address B)
// Always true: ADDR_B = ADDR_A + 1

/**
* @brief Get network address of door A.
*
*/
extern uint16_t get_reader_a_addr(void);
/**
* @brief Get network address of door B.
*
*/
extern uint16_t get_reader_b_addr(void);
/**
 * @brief Address setter.
 *
 * @param acs_addr ... ACS network address (odd or even).
 */
void set_reader_addr(const uint16_t acs_addr);

// Expected organization in external address space:
// | 0x00 | A_ADDR [7:0]
// | 0x01 | A_ADDR [9:8]
//          PADDING [15:10]

// The address actually uses less then 16 bits. See address bit width in ACS protocol.

#define PTR_READER_FIRST_ADDR 0x0 // pointer to external memory
#define STORE_DEV_BUSY_FOR 50 // Number of read commands to try before EEPROM timeout

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
#define ACS_READER_A_SENSOR_PORT          1 // Can be only 0 or 1.
#define ACS_READER_A_SENSOR_PIN           8

#define ACS_READER_A_OPEN_TIME_MS         5000
#define ACS_READER_A_OK_GLED_TIME_MS      500

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
#define ACS_READER_B_SENSOR_PORT          1 // Can be only 0 or 1.
#define ACS_READER_B_SENSOR_PIN           5

#define ACS_READER_B_OPEN_TIME_MS         5000
#define ACS_READER_B_OK_GLED_TIME_MS      500


//---------------------------------------------------------------------------------------------------------------------
// Internal setting
//---------------------------------------------------------------------------------------------------------------------

// Not user modifiable.
#define WEIGAND_DEVICE_LIMIT 4
#define SERIAL_DEVICE_LIMIT 0
#define ACS_READER_MAXCOUNT 2
#define TERMINAL_TIMER_ID 15

//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

#define HW_WATCHDOG_TIMEOUT 1 // in seconds

//---------------------------------------------------------------------------------------------------------------------
// IO
//---------------------------------------------------------------------------------------------------------------------

#define LOG_HIGH 1
#define LOG_LOW 0

//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

/**
*
* @brief Initialize configuration for terminal.
*
*        Reads address from storage.
* @note Fails if I2C bus is in invalid state
* @return true if succeeded
*/
bool terminal_config_init(void);

//---------------------------------------------------------------------------------------------------------------------

#endif /* TERM_CONFIG_H_ */
