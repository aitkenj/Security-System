 /*
 * setclockspeeds.c
 * Written By: Chris Vanzomern
 * Date Created: 11/20/2016
 * Date Last Modified: 11/27/2016
 *
 * Requirements: -External clock sources, HFXT and LFXT are available
 * 				 -CPU can run at 48 MHz with set_main_clocks()
 * 				 -delay_ms() and delay_us() assume 48 MHz master clk
 */

#include "setclockspeeds.h"

#include "driverlib.h"

int mclk_value = 0;
int smclk_value = 0;

void set_main_clocks()
{
	static int has_run = 0;

	if(has_run) { return; }

	has_run = 1;

	/* Configuring pins for peripheral/crystal usage */
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
	GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
	CS_setExternalClockSourceFrequency(32000,48000000); // enables getMCLK, getSMCLK to know externally set frequencies
	/* Starting HFXT in non-bypass mode without a timeout. Before we start
	* we have to change VCORE to 1 to support the 48MHz frequency */
	MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
	MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
	MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
	CS_startHFXT(false); // false means that there are no timeouts set, will return when stable
	/* Initializing MCLK to HFXT (effectively 48MHz) */
	MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
	MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);


	while((mclk_value != 48000000) || (smclk_value != 3000000)) // trap on wrong mclk or smclk
	{
		mclk_value = MAP_CS_getMCLK();
		smclk_value = MAP_CS_getSMCLK();
	}

	SysTick->CTRL = 0;
	SysTick->LOAD = 0x00FFFFFF;
	SysTick->VAL = 0;
	SysTick->CTRL = 0x5;
}

//Method stub for setting ACLK
void set_wdt_clocks(void){
	//TODO: Complete set-up of ACLK
}

//Method stub for clock used for i2s
void set_i2s_audio_clock(void){
	//TODO
}

//Initializes Systick timer: used by delay_us & delay_ms
void SysTick_init(void){
	SysTick->CTRL &= ~0x0001; //disable timer
	SysTick->CTRL |= 0x0004;  //Set clock source
	SysTick->CTRL &= ~0x0002; //Disable interrupt
}

void delay_ms(int ms)
{
	delay_us(ms * 1000);
}

void delay_us(int us)
{
	SysTick->CTRL |= 0x0001; //enable Systick
	while(us > 349525) // longer delays would overflow counter
	{
		SysTick->LOAD = ((349525*48) - 1);
		SysTick->VAL = 0;

		while(!(SysTick->CTRL & 0x00010000));

		us -= 349525;
	}

	SysTick->LOAD = ((us * 48) - 1);
	SysTick->VAL = 0;
	while(!(SysTick->CTRL & 0x00010000));
	SysTick->CTRL &= ~0x0001; //disable timer
}

