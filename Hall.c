/*
 * Hall.c
 * Written By: Joshua Aitken
 * Date Created: 10/30/2016
 * Date Last Modified: 10/30/2016
 *
 * Requirements: -Uses Pins 1.6,1.7
 * 				 -Must include driverlib.hs
 * 				 -Setup must have non-dimpled side of magnet facing front of sensor
 */

#include <stdio.h>
#include "driverlib.h"
//#include "ST7735.h"

//Sets-up pins for H.E. sensors
void InitHallEffect(void);
void EnableDoorAlarm(void);
void EnableWindowAlarm(void);
void DisableWindowAlarm(void);
void DisableDoorAlarm(void);
int GetWindowStatus(void);
int GetDoorStatus(void);
extern void PORT1_IRQHandler(void);

int Status = 0;

//Sets-up pins for hall effect sensors, clock is MCLK for GPIO
void InitHallEffect(void){
	//Status = P1IN & (BIT6 | BIT7);
	P1SEL0 &= ~(BIT6 | BIT6);
	P1SEL1 &= ~(BIT6 | BIT7);
	P1DIR &= ~(BIT6 | BIT7);
	P1REN |= (BIT6 | BIT7);
	__delay_cycles(10);
}

//Allows change in door state to call IVR
void EnableDoorAlarm(void){
	Interrupt_enableMaster();
	Interrupt_enableInterrupt(INT_PORT1);
	GPIO_enableInterrupt(GPIO_PORT_P1,GPIO_PIN7);
	GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN7,GPIO_LOW_TO_HIGH_TRANSITION);
}

//Allows change in door state to call IVR
void EnableWindowAlarm(void){
	Interrupt_enableMaster();
	Interrupt_enableInterrupt(INT_PORT1);
	GPIO_enableInterrupt(GPIO_PORT_P1,GPIO_PIN6);
	GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN6,GPIO_LOW_TO_HIGH_TRANSITION);
}

//Disables Alarm Trigger By Window Opening
void DisableWindowAlarm(void){
	GPIO_disableInterrupt(GPIO_PORT_P1,GPIO_PIN6);
}

//Disables Alarm Trigger By Door Opening
void DisableDoorAlarm(void){
	GPIO_disableInterrupt(GPIO_PORT_P1,GPIO_PIN7);
}

//Return 1 if door open, 0 if closed
int GetDoorStatus(void){
	if( P1IN & BIT7){
		return 1; //Open
	}
	return 0; //Closed
}

//Return 1 if window open, 0 if closed
int GetWindowStatus(void){
	if( P1IN & BIT6){
			return 1; //Open
	}
	return 0; //Closed
}

//Interrupt Handler for Port 1 GPIO pins
void PORT1_IRQHandler(void){
	GPIO_clearInterruptFlag(GPIO_PORT_P1,(GPIO_PIN6 | GPIO_PIN7));
	Status = P1IN & (BIT6 | BIT7);
	if( Status & BIT6){ //Window Opened
		//TODO: Alarm Activation
	}
	if( Status & BIT7){ //Door Opened
		//TODO: Alarm Activation
	}
	GPIO_clearInterruptFlag(GPIO_PORT_P1,(GPIO_PIN6 | GPIO_PIN7));
}
