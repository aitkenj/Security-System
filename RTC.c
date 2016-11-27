/*
 * RTC.c
 * Written By: Chris Vanzomern
 * Date Created: 11/20/2016
 * Date Last Modified: 11/20/2016
 *
 * Requirements: -Pins 6.5, 6.4 are used for I2C
 * 				 -DCO clock is maintained at 24 MHz
 * 				 -Output prints to regular terminal
 */

#include "RTC.h"

#include <stddef.h>
#include "driverlib.h"

//#include "i2c_bus.h"
#include "setclockspeeds.h"

// format string used to convert between a struct rtc_time and a string
#define RTC_FORMAT_STRING "%s, %04d-%02d-%02d %02d:%02d:%02d"

// I2C address of the RTC
#define RTC_DEVICE_ADDRESS 0x68

// data addresses of the RTC
#define RTC_MEM_CURRENT_TIME 0x00
#define RTC_MEM_ALARM_1 0x07
#define RTC_MEM_ALARM_2 0x0B
#define RTC_MEM_CONTROL 0x0E
#define RTC_MEM_STATUS 0x0F
#define RTC_MEM_AGING_OFFSET 0x10
#define RTC_MEM_TEMP 0x11

#define RTC_CTRL_EOSC BIT(7)
#define RTC_CTRL_BBSQW BIT(6)
#define RTC_CTRL_CONV BIT(5)
#define RTC_CTRL_INTCN BIT(2)
#define RTC_CTRL_A2IE BIT(1)
#define RTC_CTRL_A1IE BIT(0)

#define RTC_STAT_OSF BIT(7)
#define RTC_STAT_EN32KHZ BIT(3)
#define RTC_STAT_BSY BIT(2)
#define RTC_STAT_A2F BIT(1)
#define RTC_STAT_A1F BIT(0)

// Assume all dates are after the year 2000.
// Nobody using this is a time traveler.
#define RTC_STARTING_YEAR 2000


// The RTC uses binary-coded decimal.
// This format has no benefit on the
// system we're using, and it breaks
// even the simplest math.
// All dates and times used on the
// microcontroller should use the
// mathematically correct format,
// while anything sent to the RTC
// uses BCD.
int to_number(int packed_bcd)
{
	int num = 0;
	int decimal_place = 1;

	while(packed_bcd)
	{
		num += decimal_place * (packed_bcd % 0x10);
		packed_bcd /= 0x10;
		decimal_place *= 10;
	}

	return num;
}

int to_packed_bcd(int num)
{
	int bcd = 0;
	int hexadecimal_place = 0x1;

	while(num)
	{
		bcd += hexadecimal_place * (num % 10);
		num /= 10;
		hexadecimal_place *= 0x10;
	}

	return bcd;
}


// Day names for pretty printing.
// Element 0 is not actually used.
char const * const rtc_day_name[8] = {
		"BAD_TIME",
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday"
};


int rtc_time_is_believable(struct rtc_time *time)
{
	return (
			time->year >= RTC_STARTING_YEAR && time->year < RTC_STARTING_YEAR + 200
			&& time->month > 0 && time->month <= 12
			&& time->date > 0 && time->date <= 31
			&& time->day_of_week > 0 && time->day_of_week <= 7
			&& time->hour >= 0 && time->hour < 24
			&& time->min >= 0 && time->min < 60
			&& time->sec >= 0 && time->sec < 60
	);
}


static uint8_t rtc_registers[19] = {0};


// reads registers into shared variable
// fails (returns 0) if i2c send or receive fails
int rtc_read_registers()
{
	if(!i2c_send_single(RTC_DEVICE_ADDRESS, 0)) {return 0;}

	if(!i2c_receive(RTC_DEVICE_ADDRESS, rtc_registers, 19)) {return 0;}

	return 1;
}

// fails (returns 0) if rtc_read_registers() fails or if time is unbelievable
int rtc_gettime(struct rtc_time *result)
{
	if(!rtc_read_registers())
	{
		return 0;
		result->day_of_week = 0;
	}

	struct rtc_time tmp_result = {};

	tmp_result.sec         = to_number(rtc_registers[0]);
	tmp_result.min         = to_number(rtc_registers[1]);
	tmp_result.hour        = to_number(rtc_registers[2]);
	tmp_result.day_of_week = to_number(rtc_registers[3]);
	tmp_result.date        = to_number(rtc_registers[4]);
	tmp_result.month       = to_number(rtc_registers[5] & 0x1F);
	// RTC year + starting year + century
	tmp_result.year        = to_number(rtc_registers[6]) + RTC_STARTING_YEAR + (rtc_registers[5] >> 7);

	memcpy(result, &tmp_result, sizeof(struct rtc_time));

	if(rtc_time_is_believable(&tmp_result))
	{
		return 1;
	}
	else
	{
		result->day_of_week = 0;

		return 0;
	}
}

// fails (returns 0) if time is unbelievable or i2c_send() fails
int rtc_settime(struct rtc_time *time)
{
	if(!rtc_time_is_believable(time))
	{
		return 0;
	}

	uint8_t sent[8] = {0};

	int rtc_year = (time->year - RTC_STARTING_YEAR) % 100;
	int rtc_century = (time->year - RTC_STARTING_YEAR) / 100;

	sent[0] = RTC_MEM_CURRENT_TIME; // data location address
	sent[1] = to_packed_bcd(time->sec);
	sent[2] = to_packed_bcd(time->min);
	sent[3] = to_packed_bcd(time->hour);
	sent[4] = to_packed_bcd(time->day_of_week);
	sent[5] = to_packed_bcd(time->date);
	sent[6] = to_packed_bcd((rtc_century << 7) | time->month);
	sent[7] = to_packed_bcd(rtc_year);

	if(!i2c_send(RTC_DEVICE_ADDRESS, sent, 8)) {return 0;}

	// first read after setting is always wrong...
	rtc_read_registers();

	return 1;
}


// this function is garbage...
//Re: This function seems to work fine.
void rtc_gettemp(float* temperature)
{
	// receive control and status registers from RTC
	rtc_read_registers();

	// wait for the conversion to finish
	while(rtc_registers[RTC_MEM_STATUS] & RTC_STAT_BSY)
	{
		rtc_read_registers();
	}

	// shift the temperature bits into the correct locations
	int raw_temperature = (rtc_registers[RTC_MEM_TEMP] << 2) + (rtc_registers[RTC_MEM_TEMP + 1] >> 6);
	// subtract the sign bit for negative numbers
	raw_temperature -= (rtc_registers[RTC_MEM_TEMP] & 0x80 << 3);

	*temperature = raw_temperature * 0.25;
}


void rtc_format(struct rtc_time *time, char* text_buf, size_t length)
{
	if(rtc_time_is_believable(time))
	{
		snprintf(
				text_buf, length, RTC_FORMAT_STRING,
				rtc_day_name[time->day_of_week],
				time->year,
				time->month,
				time->date,
				time->hour,
				time->min,
				time->sec);
	}
	else
	{
		snprintf(
				text_buf, length, RTC_FORMAT_STRING,
				rtc_day_name[0],
				time->year,
				time->month,
				time->date,
				time->hour,
				time->min,
				time->sec
				);
	}
}
