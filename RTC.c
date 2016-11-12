/*
 * RTC.c
 * Written By: Joshua Aitken
 * Date Created: 10/30/2016
 * Date Last Modified: 10/30/2016
 *
 * Requirements: -Pins 6.5, 6.4 are used for I2C
 * 				 -DCO clock is maintained at 24 MHz
 * 				 -Output prints to regular terminal
 */

#include "driverlib.h"
#include <stdio.h>

void I2CInit(void); //Sets up I2C and assigns RTC slave address
//Writes time input into RTC registers
void RTCWrite(uint8_t Min, uint8_t Hour, uint8_t Date, uint8_t Month);
void RTCReadTime(void); //Reads time from RTC registers into RTC_registers
void RTCGet_Time(char Time[]); //Converts current time in RTC_registers from BCD to decimal & displays
void RTCGet_Date(char Date[]); //Converts date in RTC_registers to weekday and numerical display
float RTCGet_Temp(void); //Reads and displays temp in Celsius

char Pin[10]; //Array of pincode chars for PinCode entry function
uint8_t RTC_registers[19]; //Global array to store date/time bytes


float RTCGet_Temp(void){
    // Set Master in transmit mode
    	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
	// Wait for bus release
    	while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
    	//Set pointer to temp registers
    	I2C_masterSendSingleByte(EUSCI_B1_BASE,0x11);
    	// Wait for bus release
    	while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
    	// Set Master in receive mode
    	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
    	// Wait for bus release, ready to receive
    	while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
    	// read from RTC registers (pointer auto increments after each read)
    	RTC_registers[17]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE); //integer
    	RTC_registers[18]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE); //fraction
    	while( I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);

	float TempC = (float)(0x7F & RTC_registers[17]) + (float)(RTC_registers[18]>>6)*0.25;
	if( RTC_registers[17] & 0x80 ){
			TempC *= -1;
	}
	return TempC;
}


void RTCGet_Date(char Date[]){
	uint8_t NumDay = RTC_registers[3]; //convert BCD to Dec
	uint8_t NumDate = ((RTC_registers[4] & 0x30)>>4)*10 + (RTC_registers[4] & 0x0F); //convert BCD to Dec
	uint8_t NumMonth = ((RTC_registers[5] & 0x10)>>4)*10 + (RTC_registers[5] & 0x0F); //convert BCD to Dec
	uint8_t NumYear = ((RTC_registers[6] & 0xF0)>>4)*10 + (RTC_registers[6] & 0x0F);  //convert BCD to Dec

	int i = 0;
	switch(NumDay){ //Weekdays 1-7
		case 1:
			Date[0] = 'M';
			Date[1] = 'o';
			Date[2] = 'n';
			Date[3] = 'd';
			Date[4] = 'a';
			Date[5] = 'y';
			i = 6;
			break;
		case 2:
			Date[0] = 'T';
			Date[1] = 'u';
			Date[2] = 'e';
			Date[3] = 's';
			Date[4] = 'd';
			Date[5] = 'a';
			Date[6] = 'y';
			i = 7;
			break;
		case 3:
			Date[0] = 'W';
			Date[1] = 'e';
			Date[2] = 'd';
			Date[3] = 'n';
			Date[4] = 'e';
			Date[5] = 's';
			Date[6] = 'd';
			Date[7] = 'a';
			Date[8] = 'y';
			i = 9;
			break;
		case 4:
			Date[0] = 'T';
			Date[1] = 'h';
			Date[2] = 'u';
			Date[3] = 'r';
			Date[4] = 's';
			Date[5] = 'd';
			Date[6] = 'a';
			Date[7] = 'y';
			i = 8;
			break;
		case 5:
			Date[0] = 'F';
			Date[1] = 'r';
			Date[2] = 'i';
			Date[3] = 'd';
			Date[4] = 'a';
			Date[5] = 'y';
			i = 6;
			break;
		case 6:
			Date[0] = 'S';
			Date[1] = 'a';
			Date[2] = 't';
			Date[3] = 'u';
			Date[4] = 'r';
			Date[5] = 'd';
			Date[6] = 'a';
			Date[7] = 'y';
			i = 8;
			break;
		case 7:
			Date[0] = 'S';
			Date[1] = 'u';
			Date[2] = 'n';
			Date[3] = 'd';
			Date[4] = 'a';
			Date[5] = 'y';
			i = 6;
			break;
	}

	Date[(i++)] = ' ';
	Date[(i++)] = NumMonth/10 + 48;
	Date[(i++)] = NumMonth%10 + 48;
	Date[(i++)] = '/';
	Date[(i++)] = NumDate/10 + 48;
	Date[(i++)] = NumDate%10 + 48;
	Date[(i++)] = '/';
	Date[(i++)] = NumYear/10 + 48;
	Date[(i++)] = NumYear%10 + 48;
}

//Displays Current Time/Date
//Time param. must have length >= 9
void RTCGet_Time(char Time[]){
	char Period; //AM or PM
	uint8_t Seconds = ((RTC_registers[0] & 0x70)>>4)*10 + (RTC_registers[0] & 0x0F); //convert BCD to Dec
	uint8_t Minutes = ((RTC_registers[1] & 0x70)>>4)*10 + (RTC_registers[1] & 0x0F); //convert BCD to Dec
	uint8_t Hours = 0; //

	if( RTC_registers[2] & BIT6 ){
			Hours = RTC_registers[2] & 0x0F; //convert BCD to Dec
			if( RTC_registers[2] & BIT5){ Period = 'P'; } //if 1, PM
			else{ Period = 'A';} //if 0 , AM
	}
	else{
		//convert BCD to Dec
		Hours = ((RTC_registers[2] & BIT5)>>5)*20 + ((RTC_registers[2] & BIT4)>>4)*10 + (RTC_registers[2] & 0x0F);
	}

	//Assign RTC read parameters into result char array
	Time[0] = Hours/10 + 48; //tens hr unit
	Time[1] = Hours%10 + 48; //1s hr unit
	Time[2] = ':';
	Time[3] = Minutes/10 + 48; //tens min unit
	Time[4] = Minutes%10 + 48; //1s min unit
	Time[5] = ':';
	Time[6] = Seconds/10 + 48; //10s secs unit
	Time[7] = Seconds%10 + 48; //1s secs unit
	Time[8] = NULL;
}

//Uses SMCLK and assigns to DCO freq. of 24 MHz, may affect other programs
void I2CInit(void){
	MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_24); //Set to 24MHz
	//Initialize clock signal
	CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
	//Set up struct param to configure I2C
	eUSCI_I2C_MasterConfig ConfigI2C = {
			EUSCI_B_I2C_CLOCKSOURCE_SMCLK,
			24000000,
			EUSCI_B_I2C_SET_DATA_RATE_100KBPS,
			1,
			EUSCI_B_I2C_NO_AUTO_STOP
	};

	//*I2C setup credit to Rob Bossemeyer
	//Select Port6 for I2C with pins 4,5 as primary module
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
	(GPIO_PIN4 | GPIO_PIN5), GPIO_PRIMARY_MODULE_FUNCTION);
	//Enable pull-up resistor for SDA
	P6REN |= BIT4;
	P6OUT |= BIT4;
	/* Initializing I2C Master */
	MAP_I2C_initMaster(EUSCI_B1_BASE, &ConfigI2C);
	   /* Specify slave address */
	MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, 0x68); //01101000
	   /* Set Master in transmit mode */
	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
	   /* Enable I2C Module to start operations */
	MAP_I2C_enableModule(EUSCI_B1_BASE);
}


//Writes 24h time/date into RTC registers
void RTCWrite(uint8_t Min, uint8_t Hour, uint8_t Date, uint8_t Month){

	Min = (Min % 10) | ((Min / 10)<<4);
	if( Hour > 23){
		printf("Hours cannot exceed 24.\n");
		return;
	}
	Hour = ((Hour / 20)<<5) | ((Hour/10) & 0x01)<<4 | (Hour % 10);
	Date = ((Date / 10)<<4) | (Date % 10);
	Month = ((Month / 10)<<4) | (Month % 10);

	//Credited to Rob Bossemeyer
	/* Set Master in transmit mode */
	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
	   // Wait for bus release, ready to write
	while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
	// set pointer to beginning of RTC registers
	MAP_I2C_masterSendMultiByteStart(EUSCI_B1_BASE,0);
	// and write to seconds register
	MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE,0);
	// write to minutes register
	MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, Min);
	// write to hours register
	MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, Hour);
	// write to day register
	MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, 1);
	// write to date register
	MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, Date);
	// write to months register
	MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, Month);
	// write to year register and send stop
	MAP_I2C_masterSendMultiByteFinish(EUSCI_B1_BASE, 0x16);
}


//Reads the current time from RTC registers into RTC_registers
void RTCReadTime( void){
	// Set Master in transmit mode
	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
	// Wait for bus release, ready to write
	while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
	// set pointer to beginning of RTC registers
	//UCB1TXBUF = 1;
	MAP_I2C_masterSendSingleByte(EUSCI_B1_BASE,0);
	// Wait for bus release
	while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
	// Set Master in receive mode
	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
	// Wait for bus release, ready to receive
	while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
	// read from RTC registers (pointer auto increments after each read)
	RTC_registers[0]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE); //seconds
	RTC_registers[1]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE); //minutes
	RTC_registers[2]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE); //hours
	RTC_registers[3]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE); //day
	RTC_registers[4]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE); //date
	RTC_registers[5]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE); //month/century
	RTC_registers[6]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE); //year
	while( I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);
}
