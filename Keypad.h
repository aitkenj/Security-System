/*
 * Keypad.h
 * Written By: Joshua Aitken
 * Date Created: 10/30/2016
 * Date Last Modified: 10/30/2016
 *
 * Requirements: -Uses Pins 5.0,5.1,5.2,2.3,2.4,2.6,2.7
 * 				 -Input Pins (2.3,2.4,2.6,2.7) connect to 10k external pull-up Rs
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

#include "driverlib.h"
#include <stdio.h>

extern char Pin[10];

//Sets up GPIO pins for keypad
void Init_Keypad(void);

//Reads input port pins into array and validates ones-touch
uint8_t Debounce_keys(void);

//Reads No_entires keypad inputs into Pin array
void Key_Entry(uint8_t No_Entries, uint8_t AcceptNonDigits);



#endif /* KEYPAD_H_ */
