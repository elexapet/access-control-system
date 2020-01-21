/*
 * @brief NXP LPCXpresso 11C24 board file
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
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

#include "board.h"
#include "retarget.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

unsigned int TERMINAL_UID[5] = {0, 0, 0, 0, 0};


/* System oscillator rate and clock rate on the CLKIN pin */
const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

/* Translator for IOCON */
const CHIP_IOCON_PIO_T CHIP_IOCON_PIO[][12] =
{
  {
    IOCON_PIO0_0,
    IOCON_PIO0_1,
    IOCON_PIO0_2,
    IOCON_PIO0_3,
    IOCON_PIO0_4,
    IOCON_PIO0_5,
    IOCON_PIO0_6,
    IOCON_PIO0_7,
    IOCON_PIO0_8,
    IOCON_PIO0_9,
    IOCON_PIO0_10,
    IOCON_PIO0_11,
  },
  {
    IOCON_PIO1_0,
    IOCON_PIO1_1,
    IOCON_PIO1_2,
    IOCON_PIO1_3,
    IOCON_PIO1_4,
    IOCON_PIO1_5,
    IOCON_PIO1_6,
    IOCON_PIO1_7,
    IOCON_PIO1_8,
    IOCON_PIO1_9,
    IOCON_PIO1_10,
    IOCON_PIO1_11,
  },
  {
    IOCON_PIO2_0,
    IOCON_PIO2_1,
    IOCON_PIO2_2,
    IOCON_PIO2_3,
    IOCON_PIO2_4,
    IOCON_PIO2_5,
    IOCON_PIO2_6,
    IOCON_PIO2_7,
    IOCON_PIO2_8,
    IOCON_PIO2_9,
    IOCON_PIO2_10,
  #if !defined(CHIP_LPC1125)
    IOCON_PIO2_11,
  #else
    0,
  #endif
  },
  {
    IOCON_PIO3_0,
  #if !defined(CHIP_LPC1125)
    IOCON_PIO3_1,
  #else
    0,
  #endif
    IOCON_PIO3_2,
    IOCON_PIO3_3,
    IOCON_PIO3_4,
    IOCON_PIO3_5,
    0,
  }
};

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

void Board_Print_Reset_Reason(void)
{
  uint32_t status = Chip_SYSCTL_GetSystemRSTStatus();
  if (status & SYSCTL_RST_SYSRST) Board_UARTPutSTR("SW RST");
  else if (status & SYSCTL_RST_BOD) Board_UARTPutSTR("BOD RST");
  else if (status & SYSCTL_RST_WDT) Board_UARTPutSTR("WTG RST");
  else if (status & SYSCTL_RST_EXTRST) Board_UARTPutSTR("EXT RST");
  else if (status & SYSCTL_RST_POR) Board_UARTPutSTR("PWR-ON RST");
  Board_UARTPutChar('\n');
  Board_UARTPutSTR(FW_VERSION_STR);
  Board_UARTPutChar('\n');
}

/* Sends a character on the UART */
void Board_UARTPutChar(char ch)
{
#if defined(CONSOLE_UART)
	Chip_UART_SendBlocking(CONSOLE_UART, &ch, 1);
#endif
}

/* Gets a character from the UART, returns EOF if no character is ready */
int Board_UARTGetChar(void)
{
#if defined(CONSOLE_UART)
	uint8_t data;

	if (Chip_UART_Read(CONSOLE_UART, &data, 1) == 1) {
		return (int) data;
	}
#endif
	return EOF;
}

/* Outputs a string on the debug UART */
void Board_UARTPutSTR(char *str)
{
#if defined(CONSOLE_UART)
	while (*str != '\0') {
		Board_UARTPutChar(*str++);
	}
#endif
}

/* Initialize debug output via UART for board */
void Board_Console_Init(void)
{
#if defined(CONSOLE_UART)
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* TXD */

	/* Setup UART for 115.2K8N1 */
	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 115200);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);
#endif
}



/* Initializes board LED(s) */
static void Board_LED_Init(void)
{
#ifdef DEVEL_BOARD
	Chip_IOCON_PinMux(LPC_IOCON, IOCON_PIO0_7, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 7);
	Board_LED_Set(BOARD_LED_DBG_R, false);
	Chip_IOCON_PinMux(LPC_IOCON, IOCON_PIO0_8, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 8);
	Board_LED_Set(BOARD_LED_DBG_G, false);
	Chip_IOCON_PinMux(LPC_IOCON, IOCON_PIO0_9, IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 9);
	Board_LED_Set(BOARD_LED_DBG_B, false);
#endif
	Chip_IOCON_PinMux(LPC_IOCON,
	                  CHIP_IOCON_PIO[ACS_COMM_STATUS_LED_PORT][ACS_COMM_STATUS_LED_PIN],
	                  IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, ACS_COMM_STATUS_LED_PORT, ACS_COMM_STATUS_LED_PIN);
  Board_LED_Set(BOARD_LED_STATUS, false);
}

/* Sets the state of a board LED to on or off */
void Board_LED_Set(board_led_t led_number, bool on)
{
  switch (led_number)
  {
#ifdef DEVEL_BOARD
    case BOARD_LED_DBG_R:
      Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, !on);
      return;
    case BOARD_LED_DBG_G:
      Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, !on);
      return;
    case BOARD_LED_DBG_B:
      Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, !on);
      return;
#endif
    case BOARD_LED_STATUS:
      Chip_GPIO_SetPinState(LPC_GPIO, ACS_COMM_STATUS_LED_PORT, ACS_COMM_STATUS_LED_PIN, !on);
      return;
    default:
      return;
  }
}

/* Returns the current state of a board LED */
bool Board_LED_Test(board_led_t led_number)
{
  switch (led_number)
  {
#ifdef DEVEL_BOARD
    case BOARD_LED_DBG_R:
      return !Chip_GPIO_GetPinState(LPC_GPIO, 0, 7);
    case BOARD_LED_DBG_G:
      return !Chip_GPIO_GetPinState(LPC_GPIO, 0, 8);
    case BOARD_LED_DBG_B:
      return !Chip_GPIO_GetPinState(LPC_GPIO, 0, 9);
#endif
    case BOARD_LED_STATUS:
      return !Chip_GPIO_GetPinState(LPC_GPIO, ACS_COMM_STATUS_LED_PORT, ACS_COMM_STATUS_LED_PIN);
    default:
      return 0;
  }
}

void Board_LED_Toggle(board_led_t led_number)
{
  switch (led_number)
  {
#ifdef DEVEL_BOARD
    case BOARD_LED_DBG_R:
      Chip_GPIO_SetPinToggle(LPC_GPIO, 0, 7);
      return;
    case BOARD_LED_DBG_G:
      Chip_GPIO_SetPinToggle(LPC_GPIO, 0, 8);
      return;
    case BOARD_LED_DBG_B:
      Chip_GPIO_SetPinToggle(LPC_GPIO, 0, 9);
      return;
#endif
    case BOARD_LED_STATUS:
      Chip_GPIO_SetPinToggle(LPC_GPIO, ACS_COMM_STATUS_LED_PORT, ACS_COMM_STATUS_LED_PIN);
      return;
    default:
      return;
  }
}

/* Set up and initialize all required blocks and functions related to the
   board hardware */
void Board_Init(void)
{
  // Read clock settings and update SystemCoreClock variable
  SystemCoreClockUpdate();

	/* Sets up DEBUG UART */
	DEBUGINIT();

	/* Initialize GPIO */
	Chip_GPIO_Init(LPC_GPIO);

	/* Initialize LEDs */
	Board_LED_Init();

  // Enable INTs for all GPIO ports
  NVIC_EnableIRQ(EINT0_IRQn);
  NVIC_EnableIRQ(EINT1_IRQn);
  NVIC_EnableIRQ(EINT2_IRQn);
  NVIC_EnableIRQ(EINT3_IRQn);

  //Read UID
  unsigned int cmd_param[5] = {IAP_READ_UID, 0, 0, 0, 0};
  iap_entry(cmd_param, TERMINAL_UID);
}
