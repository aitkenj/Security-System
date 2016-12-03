/*
 * Hall.h
 * Written By: Joshua Aitken
 * Date Created: 10/30/2016
 * Date Last Modified: 10/30/2016
 *
 * Requirements: -Uses Pins 1.6,1.7
 * 				 -Must include driverlib.hs
 * 				 -Setup must have non-dimpled side of magnet facing front of sensor
 */

#ifndef HALL_H_
#define HALL_H_

#include <stdio.h>
#include "driverlib.h"

//Sets-up pins for H.E. sensors
void InitHallEffect(void);
void EnableDoorAlarm(void);
void EnableWindowAlarm(void);
void DisableWindowAlarm(void);
void DisableDoorAlarm(void);
int GetWindowStatus(void);
int GetDoorStatus(void);
extern void PORT1_IRQHandler(void);


#endif /* HALL_H_ */
