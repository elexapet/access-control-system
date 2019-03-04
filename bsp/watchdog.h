/*
 * watchdog.h
 *
 *  Created on: 23. 9. 2018
 *      Author: Petr
 */

#ifndef BSP_WATCHDOG_H_
#define BSP_WATCHDOG_H_

void WDT_IRQHandler(void);

void WDT_Init(void);
void WDT_Feed(void);

#endif /* BSP_WATCHDOG_H_ */
