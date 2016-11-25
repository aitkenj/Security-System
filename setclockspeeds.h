 /*
 * setclockspeeds.h
 * Written By: Chris Vanzomern
 * Date Created: 11/20/2016
 * Date Last Modified: 11/20/2016
 *
 * Requirements:
 */

#ifndef SETCLOCKSPEEDS_H_
#define SETCLOCKSPEEDS_H_

#pragma once

void set_main_clocks();
void delay_ms(int ms);
void delay_us(int us);
void SysTick_init(void);

#endif /* SETCLOCKSPEEDS_H_ */
