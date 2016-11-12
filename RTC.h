/*
 * RTC.h
 * Written By: Joshua Aitken
 * Date Created: 10/30/2016
 * Date Last Modified: 10/30/2016
 *
 * Requirements: -Pins 6.5, 6.4 are used for I2C
 * 				 -DCO clock is maintained at 24 MHz
 * 				 -Output prints to regular terminal
 */

#ifndef RTC_H_
#define RTC_H_

#include "driverlib.h"
#include <stdio.h>

 //Sets up I2C and assigns RTC slave address
void I2CInit(void);

//Writes time input into RTC registers
void RTCWrite(uint8_t Min, uint8_t Hour, uint8_t Date, uint8_t Month);

//Reads time from RTC registers into RTC_registers
void RTCReadTime(void);

//Converts current time in RTC_registers from BCD to decimal & displays
void RTCGet_Time(char Time[]);

//Converts date in RTC_registers to weekday and numerical display
void RTCGet_Date(char Date[]);

//Reads and displays temp in Celsius
float RTCGet_Temp(void);

#endif /* RTC_H_ */
