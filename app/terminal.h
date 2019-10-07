/*
 * card_reader.h
 *
 *  Created on: 14. 10. 2018
 *      Author: Petr
 */

#ifndef BSP_TERMINAL_H_
#define BSP_TERMINAL_H_

#include <reader.h>
#include <stdint.h>

void terminal_init(void);

void terminal_reconfigure(reader_conf_t * reader_cfg, uint8_t id);

#endif /* BSP_TERMINAL_H_ */
