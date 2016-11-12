/*
 * Keypad.c
 * Written By: Joshua Aitken
 * Date Created: 10/30/2016
 * Date Last Modified: 10/30/2016
 *
 * Requirements: -Uses Pins 5.0-5.3,2.5-2.7
 * 				 -Input Pins (5.0-5.3) connect to 10k external pull-up Rs
 *
 * 				 Row 0: Pin 5.0
 * 				 Row 1: Pin 5.1
 * 				 Row 2: Pin 5.2
 * 				 Row 3: Pin 5.3
 * 				 Col 0: Pin 2.5
 * 				 Col 1: Pin 2.6
 * 				 Col 2: Pin 2.7
 */

#include "driverlib.h"
#include <stdio.h>

//Sets up GPIO pins for keypad
void Init_Keypad(void);
//Reads input port pins into array and validates ones-touch
uint8_t Debounce_keys(void);
//Reads No_entires keypad inputs into Pin array
void Key_Entry(uint8_t No_Entries, uint8_t AcceptNonDigits);

static uint8_t State[8]; //array used for Debounce_keys function
char Pin[10]; //Array of pincode chars for PinCode entry function


//Initializes the pins for the keypad I/O
void Init_Keypad(void){
    P5SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3); //select P5 bits to GPIO
    P5SEL1 &= ~(BIT0 | BIT1 |  BIT2 | BIT3); //select P5 bits to GPIO
    P2SEL0 &= ~(BIT7 | BIT6 | BIT5); //Set P2 bits to GPIO
    P2SEL1 &= ~(BIT7 | BIT6 | BIT5); //Set P2 bits to GPIO

    P5DIR &= ~(BIT0 |  BIT1 | BIT2 | BIT3); //Set P5 bits to input
    P2DIR &= ~(BIT7 | BIT6 | BIT5); //Set P2 bits to input

    P5REN |= (BIT0 |  BIT1 | BIT2 | BIT3); //Enable pull-up Rs inputs
    P5OUT |= (BIT0 |  BIT1 | BIT2 | BIT3); //Set to pull-up for inputs
}

/*Accepts up to 10 key presses to update Pin array
*If No_Entries > 10, first No_Entries - 10 inputs will left-shift out*/
void Key_Entry(uint8_t No_Entries, uint8_t AcceptNonDigits){
	int i = 0; //Pin array index

    uint8_t ColSelect = 0x00; //Hex value of column pin
    uint8_t RowSelect = 0x00; //Hex Value of Row pin

    while( i < No_Entries ){
    		//Determine which column is at low level (on)
    		ColSelect = (BIT7 | BIT6 | BIT5) & P2DIR;

    		RowSelect = Debounce_keys(); //Check for button press
    		if( RowSelect ){ //if P5 pin high (button depressed)
    			switch(ColSelect){
					case 32: //Col0
						switch( RowSelect ){
							case 1: //Row 0
								Pin[i] = '1';
							break;
							case 2: //Row 1
								Pin[i] = '4';
							break;
							case 4: //Row 2
								if( AcceptNonDigits ){
									Pin[i] = '7';
								}
								else{
									i--;
									printf("Entry must be a valid digit.\n");
								}
							break;
							case 8: //Row 3
								Pin[i] = '*';
							break;
						}
					break;
					case 64: //Col1
						switch( RowSelect ){
							case 1: //Row 0
								Pin[i] = '2';
							break;
							case 2: //Row 1
								Pin[i] = '5';
							break;
							case 4: //Row 2
								Pin[i] = '8';
							break;
							case 8: //Row 3
								Pin[i] = '0';
							break;
						}
					break;
					case 128: //Col2
						switch( RowSelect ){
							case 1: //Row 0
								Pin[i] = '3';
							break;
							case 2: //Row 1
								Pin[i] = '6';
							break;
							case 4: //Row 2
								Pin[i] = '9';
							break;
							case 8: //Row 3
								if( AcceptNonDigits ){
									Pin[i] = '#';
								}
								else {
									i--;
									printf("Entry must be a valid digit.\n");
								}
							break;
						}
					break;
    				}
    			if( i < 10){
    				i++;
    			}
    			else{
    				if( No_Entries > 10 ){
        				int n;
        				for(n = 0; n > 10; n++){
        					Pin[n] = Pin[n+1];
        				}
    				}
    			}
    		}
    		switch( ColSelect){
			case 32: //Column 0
				P2DIR &= ~BIT5;
				P2DIR |= BIT6;
				P2OUT &= ~BIT6;
			break;
			case 64: //Column 1
				P2DIR &= ~BIT6;
				P2DIR |= BIT7;
				P2OUT &= ~BIT7;
			break;
			case 128: //Column 2
				P2DIR &= ~BIT7;
				P2DIR |= BIT5;
				P2OUT &= ~BIT5;
			break;
			default: //Set Col0 to check
				P2DIR |= BIT5;
				P2OUT &= ~BIT5;
    		}
    		__delay_cycles(10);
    }
}

uint8_t Debounce_keys(void){
	uint8_t idx = 0;
	while( idx < 8){
		State[idx] = (BIT0 | BIT1 | BIT2 | BIT3) & ~P5IN; //0x0F & ~P2IN
		idx++;
	}
	int RowVal = State[0];
	idx = 1;
	while( idx < 8){
		RowVal &= State[idx];
		idx++;
	}
	if( RowVal == 1 || RowVal == 2 || RowVal == 4 | RowVal == 8){
		while( RowVal == (0x0F & ~P5IN));
		return RowVal;
	}
	else{
		return 0;
	}
}
