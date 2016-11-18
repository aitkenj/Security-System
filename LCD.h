/*
 * LCD.h
 * Written By: Joshua Aitken
 * Date Created: 11/14/2016
 * Date Last Modified: 11/14/2016
 *
 * Requirements:
 */

#ifndef LCD_H_
#define LCD_H_

//Initializes LCD I/O pins, state of screen and splash screen display
void LCDStartup(void);

//Sets ups systick timer pins and sets clk source
void SysTick_init(void);

//Creates delay for any value of ms with SysTick timer
void SysTick_delay(uint16_t Delay_ms);

//Sets MCLK to use HSCLK external oscillator
void SetupClock48MHz(void);

//Displays Home Screen Information in Basic state with alarm/door status indicated
void LCDHome(uint16_t AlarmStatus, uint16_t DoorStatus, uint16_t AlarmEvent, char AlarmSource[], char Date[], char Day[]);

//This sets the home display into the alarm triggered state and displays a flashing indicator
void LCDFlashAlarm(uint8_t FlashSeconds);

//This Reads and displays the time, date and temperature in a scrolling motion
void LCDScrollDisplay(void);


#endif /* LCD_H_ */
