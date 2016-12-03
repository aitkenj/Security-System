/*
 * DoorLock.h
 * Written By: Joshua Aitken
 * Date Created: 12/1/2016
 * Date Last Modified: 12/1/2016
 *
 * Requirements: -Uses Pins 7.0-7.3
 */

#ifndef DOORLOCK_H_
#define DOORLOCK_H_

#define THREESIXTYDEGSTEPS 2048 //Steps in 360 deg.
#define ONEEIGHTDEGSTEPS 1024 //Steps in 180 deg.
#define NINETYDEGSTEPS 512 //Steps in 90 deg.

int GetLockStatus(void);
void Stepper_Init(void);
void LockDoor(void);
void UnLockDoor(void);
void SetMotorSpeed(int secs_per_rev);
void AdjustShaftPosition(int clkwise_steps);

#endif /* DOORLOCK_H_ */
