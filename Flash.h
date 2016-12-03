 /*
 * LCD.h
 * Written By: Joshua Aitken
 * Date Created: 11/19/2016
 * Date Last Modified: 11/20/2016
 *
 * Requirements: -Must include Flash.h, msp.h, driverlib.h
 * 				 -Memory Location must be Flash override mailbox @ Bank 0, Sector 0
 * 				 -Alarm Trigger Log info stored in 0x000200000 - 0x000200050 (see .h file)
 * 				 -Alarm Event Log info stored in 0x000200064 - 0x0002000B4
 * 				 -Alarm event/trigger info stored as ASCII sequence of chars as "hh:mm:ss MM/DD/YY S"
 * 				 -'S' character of trigger event 19th info data element is source:
 * 				 	->'D' = Door
 * 				 	->'W' = Window
 * 				 	->'P' = Motion Sensor
 * 				 	->'T' = Fire (Temp Sensor)
 * 				 -'S' char of arm/disarm event 19th info data element is action type:
 * 				 	->'A' = Arm
 * 				 	->'D' = Disarm
 */

#ifndef FLASH_H_
#define FLASH_H_

//Macros for Flash pointer addressing
#define CALIBRATION_START 0x000200000
#define TRIGGER_EVENT1 0x000200000
#define TRIGGER_EVENT2 0x000200014
#define TRIGGER_EVENT3 0x000200028
#define TRIGGER_EVENT4 0x00020003C
#define TRIGGER_EVENT5 0x000200050

#define ARM_EVENT1 0x000200064
#define ARM_EVENT2 0x000200078
#define ARM_EVENT3 0x00020008C
#define ARM_EVENT4 0x0002000A0
#define ARM_EVENT5 0x0002000B4

void FlashSetup(void); //Sets Internal voltage level
void FlashWriteData(void* Fl_Location,char FlashData[200]);

#endif /* FLASH_H_ */
