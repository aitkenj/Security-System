 /*
 * LCD.h
 * Written By: Joshua Aitken
 * Date Created: 11/19/2016
 * Date Last Modified: 11/20/2016
 *
 * Requirements:
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
