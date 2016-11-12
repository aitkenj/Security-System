/*
 * Hall.c
 * Written By: Joshua Aitken
 * Date Created: 10/30/2016
 * Date Last Modified: 10/30/2016
 *
 * Requirements: -Uses Pins 1.6,1.7
 */

#include <stdio.h>
#include "driverlib.h"
//#include "ST7735.h"

//Sets-up pins for H.E. sensors
void InitHallEffect(void);
extern void PORT1_IRQHandler(void);

int Status = 0;

//Sets-up pins for hall effect sensors, clock is MCLK for GPIO
void InitHallEffect(void){
	Status = P1IN & (BIT6 | BIT7);
	P1SEL0 &= ~(BIT6 | BIT6);
	P1SEL1 &= ~(BIT6 | BIT7);
	P1DIR &= ~(BIT6 | BIT7);
	P1REN |= (BIT6 | BIT7);
	__delay_cycles(10);
	Interrupt_enableMaster();
	Interrupt_enableInterrupt(INT_PORT1);
	GPIO_enableInterrupt(GPIO_PORT_P1,(GPIO_PIN6 | GPIO_PIN7));
}

//Interrupt Handler for Port 1 GPIO pins
void PORT1_IRQHandler(void){
	GPIO_clearInterruptFlag(GPIO_PORT_P1,(GPIO_PIN6 | GPIO_PIN7));
	Status = P1IN & (BIT6 | BIT7);
//		ST7735_FillScreen(0);
//		ST7735_SetCursor(0,0);
	if( Status & BIT6){
		printf("Window Open.\n");
		GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN6,GPIO_HIGH_TO_LOW_TRANSITION);
	}
	else{
		printf("Window Closed.\n");
		GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN6,GPIO_LOW_TO_HIGH_TRANSITION);
	}
	if( Status & BIT7){
		printf("Door Open.\n");
		GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN7,GPIO_HIGH_TO_LOW_TRANSITION);
	}
	else{
		printf("Door Closed.\n");
		GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN7,GPIO_LOW_TO_HIGH_TRANSITION);
	}
	GPIO_clearInterruptFlag(GPIO_PORT_P1,(GPIO_PIN6 | GPIO_PIN7));
}
