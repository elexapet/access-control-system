/*
 * @brief Common board API functions
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#ifndef __BOARD_API_H_
#define __BOARD_API_H_

#include "lpc_types.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup BOARD_COMMON_API BOARD: Common board functions
 * @ingroup BOARD_Common
 * This file contains common board definitions that are shared across
 * boards and devices. All of these functions do not need to be
 * impemented for a specific board, but if they are implemented, they
 * should use this API standard.
 * @{
 */

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif


// Available LEDs on board
typedef enum
{
  BOARD_LED_RESERVED = 0,
#ifdef DEVEL_BOARD
  BOARD_LED_DBG_R,
  BOARD_LED_DBG_G,
  BOARD_LED_DBG_B,
#endif
  BOARD_LED_STATUS,
} board_led_t;

/**
 * @brief	Set up and initialize hardware prior to call to main()
 * @return	None
 * @note	Board_SystemInit() is called prior to the application and sets up system
 * clocking, memory, and any resources needed prior to the application
 * starting.
 */
void Board_SystemInit(void);

/**
 * @brief	Set up and initialize all required blocks and functions related to the board hardware.
 * @return	None
 */
void Board_Init(void);

/**
 * @brief	Initializes board UART for output, required for printf redirection
 * @return	None
 */
void Board_Console_Init(void);


void Board_Print_Reset_Reason(void);

/**
 * @brief	Sends a single character on the UART, required for printf redirection
 * @param	ch	: character to send
 * @return	None
 */
void Board_UARTPutChar(char ch);

/**
 * @brief	Get a single character from the UART, required for scanf input
 * @return	EOF if not character was received, or character value
 */
int Board_UARTGetChar(void);

/**
 * @brief	Prints a string to the UART
 * @param	str	: Terminated string to output
 * @return	None
 */
void Board_UARTPutSTR(char *str);

/**
 * @brief	Sets the state of a board LED to on or off
 * @param	LEDNumber	: LED number to set state for
 * @param	State		: true for on, false for off
 * @return	None
 */
void Board_LED_Set(board_led_t led_number, bool on);

/**
 * @brief	Returns the current state of a board LED
 * @param	LEDNumber	: LED number to set state for
 * @return	true if the LED is on, otherwise false
 */
bool Board_LED_Test(board_led_t led_number);

/**
 * @brief	Toggles the current state of a board LED
 * @param	LEDNumber	: LED number to change state for
 * @return	None
 */
void Board_LED_Toggle(board_led_t led_number);

/**
 * @brief Function prototype for a MS delay function. Board layers or example code may
 *        define this function as needed.
 */
typedef void (*p_msDelay_func_t)(uint32_t);

/* The DEBUG* functions are selected based on system configuration.
   Code that uses the DEBUG* functions will have their I/O routed to
   the UART, semihosting, or nowhere. */
#if defined(DEBUG_ENABLE)
#if defined(DEBUG_SEMIHOSTING)
#define DEBUGINIT()
#define DEBUGOUT(...) printf(__VA_ARGS__)
#define DEBUGSTR(str) printf(str)
#define DEBUGIN() (int) EOF

#else
#define DEBUGINIT() Board_Console_Init()
#define DEBUGOUT(...) printf(__VA_ARGS__)
#define DEBUGSTR(str) Board_UARTPutSTR(str)
#define DEBUGIN() Board_UARTGetChar()
#endif /* defined(DEBUG_SEMIHOSTING) */

#else
#define DEBUGINIT() Board_Console_Init()
#define DEBUGOUT(...)
#define DEBUGSTR(str)
#define DEBUGIN() (int) EOF
#endif /* defined(DEBUG_ENABLE) */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_API_H_ */
