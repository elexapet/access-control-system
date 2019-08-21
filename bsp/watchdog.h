/*
 * watchdog.h
 *
 *  Created on: 23. 9. 2018
 *      Author: Petr
 */

#ifndef BSP_WATCHDOG_H_
#define BSP_WATCHDOG_H_

#include <stdint.h>

void WDT_IRQHandler(void);

void WDT_Init(uint8_t timeout);
void WDT_Feed(void);

#endif /* BSP_WATCHDOG_H_ */
