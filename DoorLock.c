/*
 * DoorLock.c
 * Written By: Joshua Aitken
 * Date Created: 12/1/2016
 * Date Last Modified: 12/1/2016
 *
 * Requirements: -Uses Pins 7.0-7.3
 * 				 	-> 7.0 = Blue
 * 				 	-> 7.1 = Pink
 * 				 	-> 7.2 = Yellow
 * 				 	-> 7.3 = Orange
 * 				 -Must run MCLK at 48MHz
 * 				 -Darlington Pair IC requires 5V supply
 */

#include "DoorLock.h"
#include "setclockspeeds.h"
#include "msp.h"
#include "driverlib.h"

int step; //Step counter
int LockState;
int ms_step_delay = 3;

int GetLockStatus(void);
void Stepper_Init(void);
void LockDoor(void);
void UnLockDoor(void);
void SetMotorSpeed(int secs_per_rev);
void AdjustShaftPosition(int clkwise_steps);

void Stepper_Init(void){
	//Configure Control pins
	P7SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3); //Set GPIO
	P7SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3); //Set GPIO
	P7DIR |= (BIT0 | BIT1 | BIT2 | BIT3); //Set to output
	P7OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3); //Set to low
}

void LockDoor(void){
	//TODO: Test
	step = 0; //Initialize step counter
	//Initial Sequence to align rotor
	P7OUT |= BIT0 | BIT1; //Switch on pink, blue line coils
	delay_ms(ms_step_delay); //3ms delay

	//Full Step Two phase
	while( step < NINETYDEGSTEPS){ //loop until 360 deg.
		P7OUT &= ~BIT0; //switch off blue coil
		P7OUT |= BIT2; //switch on yellow coil
		delay_ms(ms_step_delay); //3ms delay
		P7OUT &= ~BIT1; //Switch off pink line coil
		P7OUT |= BIT3; //switch on orange coil
		delay_ms(ms_step_delay); //3ms delay
		P7OUT &= ~BIT2; //Switch off yellow coil
		P7OUT |= BIT0; //switch on blue coil
		delay_ms(ms_step_delay); //3ms delay
		P7OUT &= ~BIT3; //Switch off orange coil
		P7OUT |= BIT1; //Switch on pink line coil
		delay_ms(ms_step_delay); //3ms delay
		step += 4; //increment by 4 steps
	}
	LockState = 1;
}

void UnLockDoor(void){
	//TODO: Test
	step = 0; //Initialize step counter
	//Initial Sequence to align rotor
	P7OUT |= BIT0 | BIT1; //Switch on pink, blue line coils
	delay_ms(ms_step_delay); //3ms delay

	while( step < NINETYDEGSTEPS){ //loop until 360 deg.
		P7OUT &= ~BIT1; //Switch off pink coil
		P7OUT |= BIT3; //Switch on orange coil
		delay_ms(ms_step_delay);
		P7OUT &= ~BIT0; //Switch off blue coil
		P7OUT |= BIT2; //Switch on yellow coil
		delay_ms(ms_step_delay);
		P7OUT &= ~BIT3; //Switch off orange coil
		P7OUT |= BIT1; //Switch on pink coil
		delay_ms(ms_step_delay);
		P7OUT &= ~BIT2; //Switch off yellow coil
		P7OUT |= BIT0; //Switch on blue coil
		delay_ms(ms_step_delay);
		step += 4;
	}
	LockState = 0;
}

//Sets rate of stepper: Min = 6secs
void SetMotorSpeed(int secs_per_rev){
	//TODO:
	if( secs_per_rev < 7){
		ms_step_delay = 3;
	}
	else{
		ms_step_delay = (int)((float)secs_per_rev * 1000.0 / 2048.0);
	}
}

void AdjustShaftPosition(int clkwise_steps){
	step = 0; //Initialize step counter
	//Initial Sequence to align rotor
	P7OUT |= BIT0 | BIT1; //Switch on pink, blue line coils
	delay_ms(ms_step_delay); //3ms delay

	//Full Step Two phase
	while( step < clkwise_steps){ //loop until 360 deg.
		P7OUT &= ~BIT0; //switch off blue coil
		P7OUT |= BIT2; //switch on yellow coil
		delay_ms(ms_step_delay); //3ms delay
		P7OUT &= ~BIT1; //Switch off pink line coil
		P7OUT |= BIT3; //switch on orange coil
		delay_ms(ms_step_delay); //3ms delay
		P7OUT &= ~BIT2; //Switch off yellow coil
		P7OUT |= BIT0; //switch on blue coil
		delay_ms(ms_step_delay); //3ms delay
		P7OUT &= ~BIT3; //Switch off orange coil
		P7OUT |= BIT1; //Switch on pink line coil
		delay_ms(ms_step_delay); //3ms delay
		step += 4; //increment by 4 steps
	}
	LockState = 0;
}

//Returns 1 if locked. 0 if unlocked.
int GetLockStatus(void){
	//TODO:Determine what state is locked/unlocked
	return LockState;
}
