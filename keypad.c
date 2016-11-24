#include "driverlib.h"

#include "input.h"


#define KEYPAD_DRIVE_PORT GPIO_PORT_P5
#define KEYPAD_SENSE_PORT GPIO_PORT_P2

#define ALL_KEYPAD_DRIVE_PINS (GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3)
#define ALL_KEYPAD_SENSE_PINS (GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7)

const int keypad_drive_pins[4] = {GPIO_PIN0, GPIO_PIN1, GPIO_PIN2, GPIO_PIN3};
const int keypad_sense_pins[3] = {GPIO_PIN5, GPIO_PIN6, GPIO_PIN7};






// Using this function to drive the keypad inputs ensures only one is active
// at any time.
void drive_keypad_drive_pin(int new_pin)
{
	static volatile int old_pin;

	MAP_GPIO_setAsInputPinWithPullDownResistor(KEYPAD_DRIVE_PORT, old_pin);

	old_pin = new_pin;

	MAP_GPIO_setAsOutputPin(KEYPAD_DRIVE_PORT, new_pin);
	MAP_GPIO_setOutputHighOnPin(KEYPAD_DRIVE_PORT, new_pin);
}


inline int read_keypad_sense_pin(int pin)
{
	return MAP_GPIO_getInputPinValue(KEYPAD_SENSE_PORT, pin) == GPIO_INPUT_PIN_HIGH;
}



void keypad_init()
{
	MAP_GPIO_setAsInputPinWithPullDownResistor(KEYPAD_DRIVE_PORT, ALL_KEYPAD_DRIVE_PINS);
	MAP_GPIO_setAsInputPinWithPullDownResistor(KEYPAD_SENSE_PORT, ALL_KEYPAD_SENSE_PINS);

	MAP_GPIO_interruptEdgeSelect(KEYPAD_SENSE_PORT, ALL_KEYPAD_SENSE_PINS, GPIO_LOW_TO_HIGH_TRANSITION);
	MAP_GPIO_enableInterrupt(KEYPAD_SENSE_PORT, ALL_KEYPAD_SENSE_PINS);

	drive_keypad_drive_pin(ALL_KEYPAD_DRIVE_PINS);
}


// Drives each input high, one at a time, and reads the outputs.
// Does not try to process the results, but returns a bitfield giving
// what the output of each combination was.
int keypad_scan_once()
{
	int result = 0;

	for(int row = 0; row < 4; row++)
	{
		drive_keypad_drive_pin(keypad_drive_pins[row]);

		// No delay is necessary here.
		// each keypad_in line is driven low before being switched from output to input,
		// so there is no need to wait for the pull-down resistor to drain it.

		for(int col = 0; col < 3; col++)
		{
			result |= read_keypad_sense_pin(keypad_sense_pins[col]) << (row + col*4);
		}
	}
	drive_keypad_drive_pin(0);
	return result;
}


// converts a keypress (or combination of keypresses) into a character
char keypad_convert_to_char(int raw_press)
{
	switch(raw_press)
	{
	case 0x0000:
		return 0;
	case 0x0001:
		return '#';
	case 0x0002:
		return '9';
	case 0x0004:
		return '6';
	case 0x0008:
		return '3';
	case 0x0010:
		return '0';
	case 0x0020:
		return '8';
	case 0x0040:
		return '5';
	case 0x0080:
		return '2';
	case 0x0100:
		return '*';
	case 0x0200:
		return '7';
	case 0x0400:
		return '4';
	case 0x0800:
		return '1';
	default:
		return 'E';
	}
}



void keypad_irq_handler()
{
	input_append(keypad_convert_to_char(keypad_scan_once()));

	drive_keypad_drive_pin(ALL_KEYPAD_DRIVE_PINS);
}
