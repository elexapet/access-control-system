/*
 * board_config.h
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 */

#ifndef BSP_BOARD_CONFIG_H_
#define BSP_BOARD_CONFIG_H_

#include "chip.h"

#define WEIGAND_TRANS_BITS 26

#define CARD_READER1
#define CARD_READER1_PORT 3
#define CARD_READER1_D0_PIN 1
#define CARD_READER1_D1_PIN 2

#define CARD_READER_BEEPER_PORT 0
#define CARD_READER_BEEPER_PIN 0
#define CARD_READER_LED_PORT 0
#define CARD_READER_LED_PIN 0

#endif /* BSP_BOARD_CONFIG_H_ */
