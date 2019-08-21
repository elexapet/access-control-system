/*
 * brownout.c
 *
 *  Created on: 21. 8. 2019
 *      Author: Petr
 */

#include "brownout.h"
#include "board.h"


void BOD_IRQHandler(void)
{
  return;
}

void BOD_Init(void)
{
  Chip_SYSCTL_SetBODLevels(SYSCTL_BODRSTLVL_2_63V, SYSCTL_BODINTVAL_2_80V);
  Chip_SYSCTL_EnableBODReset();
}
