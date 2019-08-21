/*
===============================================================================
 Name        : watchdog.c
 Author      : Petr Elexa
 Version     :
 Copyright   :
 Description :
===============================================================================
*/

#include "watchdog.h"
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
 * @brief	Watchdog Timer Interrupt Handler for debugging
 * @return	Nothing
 * @note	Handles watchdog timer timeout events
 */
void WDT_IRQHandler(void)
{
	uint32_t wdtStatus = Chip_WWDT_GetStatus(LPC_WWDT);

	if (wdtStatus & WWDT_WDMOD_WDTOF)
	{
	  #ifdef DEBUG_ENABLE
	  DEBUGSTR("WDT timeout\n");
    #endif //DEBUG_ENABLE
		// Restart WDT
		while(Chip_WWDT_GetStatus(LPC_WWDT) & WWDT_WDMOD_WDTOF)
		{
			Chip_WWDT_ClearStatusFlag(LPC_WWDT, WWDT_WDMOD_WDTOF);
		}
		Chip_WWDT_Start(LPC_WWDT);	/* Needs restart */
	}
}

/**
 * @brief	Init
 */
void WDT_Init(uint8_t timeout)
{
	uint32_t wdtFreq;

	/* Initialize WWDT (also enables WWDT clock) */
	Chip_WWDT_Init(LPC_WWDT);

	/* Prior to initializing the watchdog driver, the clocking for the
	   watchdog must be enabled. */
	Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_WDTOSC_PD);
	/* The watchdog oscillator will be set at a 50KHz (1Mhz / 20) clock rate. */
	Chip_Clock_SetWDTOSC(WDTLFO_OSC_1_05, 20);

	/* The WDT divides the input frequency into it by 4 */
	wdtFreq = Chip_Clock_GetWDTOSCRate() / 4;

	/* Select watchdog oscillator for WDT clock source */
	Chip_Clock_SetWDTClockSource(SYSCTL_WDTCLKSRC_WDTOSC, 1);

	/* Set watchdog feed time constant to approximately x secs */
	Chip_WWDT_SetTimeOut(LPC_WWDT, wdtFreq * timeout);

	/* Clear watchdog warning and timeout interrupts */
	Chip_WWDT_ClearStatusFlag(LPC_WWDT, WWDT_WDMOD_WDTOF);

	/* Clear and enable watchdog interrupt */
	NVIC_ClearPendingIRQ(WDT_IRQn);
	NVIC_EnableIRQ(WDT_IRQn);

#ifdef RELEASE
	/* Watchdog will cause reset on production version */
	Chip_WWDT_SetOption(LPC_WWDT, WWDT_WDMOD_WDRESET);
#endif

	/* Start watchdog */
	Chip_WWDT_Start(LPC_WWDT);
}

void WDT_Feed(void)
{
	Chip_WWDT_Feed(LPC_WWDT);
}
