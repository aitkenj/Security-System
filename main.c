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
#include "DoorLock.h"

void SystemReset(void);

void main(){
	//SystemReset():
	i2c_init();
//	InitHallEffect();
//	SysTick_init();
////	Init_Keypad();
	Output_Init();
	EnableDoorAlarm();
////
////	uint16_t DoorStatus;
//////	DoorStatus = GetDoorStatus();
////	char CurrentDate[8];
////	RTCGet_Date(CurrentDate);
////	char CurrentDay[10];
//////	RTCGet_Day(CurrentDay);
////	LCDHome(0, DoorStatus, 0, NULL, CurrentDate, CurrentDay);
////
////	while(1){
////		LCDScrollDisplay(); //Code needs modifying to interrupt scroll with key input
////	}
//	//Alarm Handling Code
//	ST7735_FillScreen(0);
//	LCDScrollDisplay();
	ViewAlarmArmConfig();
	delay_ms(3000);
	char Prompt[] = "Enter current pin:";
	ViewSetPincode(3,Prompt);
	delay_ms(3000);
	ViewAdjustVolume();
}


//This sets up the watchdog timer, initializes clk, core voltage, & checks for power loss
void SystemReset(void){
	//First set clock source, speed
	//watchdog timer period and reset timer set
	//Verify if system has been reset before
	//set core voltage levels, other...
	//check for power loss & display message if occurred
}
