/**
 *  @file
 *  @brief HW watchdog.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#ifndef BSP_WATCHDOG_H_
#define BSP_WATCHDOG_H_

#include <stdint.h>
#include "board.h"

/**
* @brief Watchdog timer interrupt handler.
*
*        Handles watchdog timer timeout events.
*        Will be called only in debug mode.
*
*/
void WDT_IRQHandler(void);

/**
* @brief Initialize Watchdog timer.
*
*        After init watchdog must be reloaded by @ref WDT_Feed.
*
* @param timeout ... time to reset or interrupt from watchdog timer.
*
*/
void WDT_Init(uint8_t timeout);

/**
* @brief Feed watchdog timer.
*
*/
static inline void WDT_Feed(void)
{
	Chip_WWDT_Feed(LPC_WWDT);
}

#endif /* BSP_WATCHDOG_H_ */
