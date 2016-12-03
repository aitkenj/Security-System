 /*
 * i2c_bus.c
 * Written By: Chris Vanzomern
 * Date Created: 11/20/2016
 * Date Last Modified: 11/20/2016
 *
 * Requirements: -Must include i2c_bus.h, driverlib.h, setclockspeeds.h
 * 				 -Clk signal @ 3MHz, SMCLK, 100KBPS, module B
 */

#include "i2c_bus.h"

#include "driverlib.h"

#include "setclockspeeds.h"


// timeout before giving up on an i2c bus operation
#define I2C_BUS_EVENT_TIMEOUT 10000
#define I2C_BUS_CLEAR_TIMEOUT 100000


// counts all bus errors
int i2c_bus_error_counter = 0;

// counts each time the bus stays busy too long
int i2c_bus_busy_counter = 0;


const eUSCI_I2C_MasterConfig i2c_config =
{
		.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK,
		.i2cClk = 3000000,
		.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS,
		.byteCounterThreshold = 0,
		.autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP
};



// if the bus is busy for too long
int i2c_wait_for_bus_clear(int timeout)
{
	while(timeout--)
	{
		if(!MAP_I2C_isBusBusy(EUSCI_B1_BASE))
		{
			return 1;
		}
	}
	return 0;
}



void i2c_init()
{
	set_main_clocks();

	MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P6, GPIO_PIN4 | GPIO_PIN5);

	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
			GPIO_PORT_P6,
			GPIO_PIN4 | GPIO_PIN5,
			GPIO_PRIMARY_MODULE_FUNCTION);

	MAP_I2C_initMaster(EUSCI_B1_BASE, &i2c_config);

	MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, 0x68);

	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	MAP_I2C_enableModule(EUSCI_B1_BASE);
}


// Sends data to a device over I2C.
int i2c_send(uint8_t device_address, uint8_t* data, size_t length)
{
	int bus_errors_this_transmission = 0;

	if(length < 1)
	{
		return 0; // why are you trying to send nothing, anyway?
	}

	if(length == 1)
	{
		return i2c_send_single(device_address, data[0]);
	}

	if(!i2c_wait_for_bus_clear(I2C_BUS_CLEAR_TIMEOUT))
	{
		i2c_bus_busy_counter++;
		return 0; // bus is busy, so we fail
	}

	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);


	if(!i2c_wait_for_bus_clear(I2C_BUS_CLEAR_TIMEOUT))
	{
		i2c_bus_busy_counter++;
		return 0; // bus is busy, so we fail
	}


	if(!MAP_I2C_masterSendMultiByteStartWithTimeout(EUSCI_B1_BASE, data[0], I2C_BUS_EVENT_TIMEOUT))
	{
		bus_errors_this_transmission++;

		goto stop_transmission;
	}

	//Small code modification until c99 code switch is made
	int i;
	for(i = 1; i < length - 1; i++)
	{
		if(!MAP_I2C_masterSendMultiByteNextWithTimeout(EUSCI_B1_BASE, data[i], I2C_BUS_EVENT_TIMEOUT))
		{
			bus_errors_this_transmission++;
			goto stop_transmission;
		}
	}

	if(!MAP_I2C_masterSendMultiByteFinishWithTimeout(EUSCI_B1_BASE, data[length - 1], I2C_BUS_EVENT_TIMEOUT))
	{
		bus_errors_this_transmission++;
		goto stop_transmission;
	}

stop_transmission:

	if(!MAP_I2C_masterSendMultiByteStopWithTimeout(EUSCI_B1_BASE, I2C_BUS_EVENT_TIMEOUT))
	{
		bus_errors_this_transmission++;
	}

	i2c_bus_error_counter += bus_errors_this_transmission;

	return !bus_errors_this_transmission;
}


// Sends a single byte of data over I2C.
int i2c_send_single(uint8_t device_address, uint8_t data)
{
	if(!i2c_wait_for_bus_clear(I2C_BUS_CLEAR_TIMEOUT))
	{
		i2c_bus_busy_counter++;
		return 0; // bus is busy, so we fail
	}

	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);


	if(!i2c_wait_for_bus_clear(I2C_BUS_CLEAR_TIMEOUT))
	{
		i2c_bus_busy_counter++;
		return 0; // bus is busy, so we fail
	}

	if(!MAP_I2C_masterSendSingleByteWithTimeout(EUSCI_B1_BASE, data, I2C_BUS_EVENT_TIMEOUT))
	{
		i2c_bus_error_counter++;
		return 0;
	}

	return 1;
}


// Receives data over I2C.
int i2c_receive(uint8_t device_address, uint8_t* data, size_t length)
{
	if(length < 1)
	{
		return 0; // why would you do this?
	}


	if(!i2c_wait_for_bus_clear(I2C_BUS_CLEAR_TIMEOUT))
	{
		i2c_bus_busy_counter++;
		return 0; // bus is busy, so we fail
	}

	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);

	if(!i2c_wait_for_bus_clear(I2C_BUS_CLEAR_TIMEOUT))
	{
		i2c_bus_busy_counter++;
		return 0; // bus is busy, so we fail
	}

	// This code works, but only receives one byte at a time.
	// Attempting to receive multiple bytes at a time causes
	// the bus to never be released, and I have no idea why.

	//TI compiler is restricted to c89 mode, revert to c99 code
	//once compiler change is made
	int i;
	for(i = 0; i < length; i++)
	{
		// There is no need for a timeout here.
		// Receiving will never fail, just give invalid data.
		data[i] = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
	}

	return 1;
}
