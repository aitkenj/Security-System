#include "driverlib.h"


void keypad_irq_handler(); // defined in keypad.c


void PORT2_IRQHandler()
{
	keypad_irq_handler();

	MAP_GPIO_clearInterruptFlag(GPIO_PORT_P2, 0xFF);
}
