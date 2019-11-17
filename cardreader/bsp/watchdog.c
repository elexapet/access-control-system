/**
 *  @file
 *  @brief HW watchdog.
 *
 *  @author Petr Elexa
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

#include "watchdog.h"
#include "board.h"


/*****************************************************************************
 * Public functions
 ****************************************************************************/

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
