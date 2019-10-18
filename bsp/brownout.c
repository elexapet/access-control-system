/**
 *  @file
 *  @brief Brown-out detection.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
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
