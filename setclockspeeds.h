 /*
 * setclockspeeds.h
 * Written By: Chris Vanzomern
 * Date Created: 11/20/2016
 * Date Last Modified: 11/27/2016
 *
 * Requirements: -External clock sources, HFXT and LFXT are available
 * 				 -CPU can run at 48 MHz with set_main_clocks()
 * 				 -delay_ms() and delay_us() assume 48 MHz master clk
 */

#ifndef SETCLOCKSPEEDS_H_
#define SETCLOCKSPEEDS_H_

#pragma once

void set_main_clocks();
void set_wdt_clocks(void); //TODO
void set_i2s_audio_clock(void); //TODO
void delay_ms(int ms);
void delay_us(int us);
void SysTick_init(void);

#endif /* SETCLOCKSPEEDS_H_ */
