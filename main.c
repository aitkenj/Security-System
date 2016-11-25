#include <stdio.h>
#include "msp.h"
#include "driverlib.h"
#include "RTC.h"
#include "Keypad.h"
#include "Hall.h"
#include "LCD.h"
//#include "Audio.h"
//#include "Stepper.h"
//#include "PIR.h"
//#include "LED.h"
//#include "Bell.h"
#include "i2c_bus.h"


void SystemReset(void);

void main(){
	//SystemReset():
//	InitHallEffect();
//	SysTick_Init();
//	Init_Keypad();
//	LCDStartup();
//
//	uint16_t DoorStatus;
////	DoorStatus = GetDoorStatus();
//	char CurrentDate[8];
//	RTCGet_Date(CurrentDate);
//	char CurrentDay[10];
////	RTCGet_Day(CurrentDay);
//	LCDHome(0, DoorStatus, 0, NULL, CurrentDate, CurrentDay);
//
//	while(1){
//		LCDScrollDisplay(); //Code needs modifying to interrupt scroll with key input
//	}
	//Alarm Handling Code

	i2c_init(); //Also calls set_main_clocks();
	Output_Init();
	rtc_time Time1 = { .hour = 12,
					   .min = 12,
					   .sec = 12,
					   .day_of_week = 1,
					   .date = 23,
					   .month = 11,
					   .year = 2016 };
	struct rtc_time * result = &Time1;
	rtc_settime(result);
	rtc_gettime(result);
	char Timeout[] = { (result->hour)/10+48,(result->hour)%10+48,':',(result->min)/10+48,(result->min)%10+48,':',(result->sec)/10+48,(result->sec)%10+48};
	ST7735_OutString(Timeout);
}


//This sets up the watchdog timer, initializes clk, core voltage, & checks for power loss
void SystemReset(void){
	//First set clock source, speed
	//watchdog timer period and reset timer set
	//Verify if system has been reset before
	//set core voltage levels, other...
	//check for power loss & display message if occurred
}
