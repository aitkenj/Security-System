/*
 * RTC.h
 * Written By: Chris Vanzomern
 * Date Created: 11/20/2016
 * Date Last Modified: 11/20/2016
 *
 * Requirements: -Pins 6.5, 6.4 are used for I2C
 * 				 -DCO clock is maintained at 24 MHz
 * 				 -Output prints to regular terminal
 */

#ifndef RTC_H_
#define RTC_H_

#include "driverlib.h"
#include <stdio.h>
#include <stddef.h>

#pragma once

typedef struct rtc_time
{
	int sec;
	int min;
	int hour;
	int day_of_week;
	int date;
	int month;
	int year;
} rtc_time;

extern char const * const rtc_day_name[8];

int rtc_gettime(struct rtc_time *result);

int rtc_settime(struct rtc_time *time);

void rtc_gettemp(float *result);

void rtc_format(struct rtc_time *time, char *result, size_t length);

void EnableFireSensor(void);

void DisableFireSensor(void);

int IsFireSensorEnabled(void);

#endif /* RTC_H_ */
