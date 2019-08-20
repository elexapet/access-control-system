/*
 * terminal_config.c
 *
 *      Author: Petr
 */

#include "terminal_config.h"

// panel addresses reserved in flash
__attribute__ ((used,section(".panel_addr")))
const uint16_t ACC_PANEL_A_ADDR = 4;

__attribute__ ((used,section(".panel_addr")))
const uint16_t ACC_PANEL_B_ADDR = 5;
