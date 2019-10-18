/**
 *  @file
 *  @brief Brown-out detection.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#ifndef BSP_BROWNOUT_H_
#define BSP_BROWNOUT_H_

/**
* @brief Brown-out detection interrupt handler.
*
*/
void BOD_IRQHandler(void);

/**
* @brief Initialize brown-out detection.
*
*/
void BOD_Init(void);

#endif /* BSP_BROWNOUT_H_ */
