/*
===============================================================================
 Name        : watchdog.c
 Author      : Petr Elexa
 Version     :
 Copyright   :
 Description :
===============================================================================
*/

#include "board.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Watchdog Timer Interrupt Handler
 * @return	Nothing
 * @note	Handles watchdog timer warning and timeout events
 */
void WDT_IRQHandler(void)
{
	uint32_t wdtStatus = Chip_WWDT_GetStatus(LPC_WWDT);

	if (wdtStatus & WWDT_WDMOD_WDTOF) {
		//DO smth
		while(Chip_WWDT_GetStatus(LPC_WWDT) & WWDT_WDMOD_WDTOF) {
			Chip_WWDT_ClearStatusFlag(LPC_WWDT, WWDT_WDMOD_WDTOF);
		}
		Chip_WWDT_Start(LPC_WWDT);	/* Needs restart */
	}
}

/**
 * @brief	Init
 */
void WDT_Init(void)
{
	uint32_t wdtFreq;

	/* Initialize WWDT (also enables WWDT clock) */
	Chip_WWDT_Init(LPC_WWDT);

	/* Prior to initializing the watchdog driver, the clocking for the
	   watchdog must be enabled. This example uses the watchdog oscillator
	   set at a 50KHz (1Mhz / 20) clock rate. */
	Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_WDTOSC_PD);
	Chip_Clock_SetWDTOSC(WDTLFO_OSC_1_05, 20);

	/* The WDT divides the input frequency into it by 4 */
	wdtFreq = Chip_Clock_GetWDTOSCRate() / 4;

	/* Select watchdog oscillator for WDT clock source */
	Chip_Clock_SetWDTClockSource(SYSCTL_WDTCLKSRC_WDTOSC, 1);

	/* Set watchdog feed time constant to approximately 2s
	   Set watchdog warning time to 512 ticks after feed time constant
	   Set watchdog window time to 3s */
	Chip_WWDT_SetTimeOut(LPC_WWDT, wdtFreq * 2);

	/* Clear watchdog warning and timeout interrupts */
	Chip_WWDT_ClearStatusFlag(LPC_WWDT, WWDT_WDMOD_WDTOF);

	/* Clear and enable watchdog interrupt */
	NVIC_ClearPendingIRQ(WDT_IRQn);
	NVIC_EnableIRQ(WDT_IRQn);

	/* Start watchdog */
	Chip_WWDT_Start(LPC_WWDT);
}

void WDT_Feed(void)
{
	Chip_WWDT_Feed(LPC_WWDT);
}
