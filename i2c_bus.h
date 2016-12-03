 /*
 * i2c_bus.c
 * Written By: Chris Vanzomern
 * Date Created: 11/20/2016
 * Date Last Modified: 11/20/2016
 *
 * Requirements: -Must include i2c_bus.h, driverlib.h, setclockspeeds.h
 * 				 -Clk signal @ 3MHz, SMCLK, 100KBPS, module B
 */


#pragma once

#include <stddef.h>
#include <stdint.h>

void i2c_init();

int i2c_send(uint8_t device_address, uint8_t *data, size_t length);

int i2c_send_single(uint8_t device_address, uint8_t data);

int i2c_receive(uint8_t device_address, uint8_t *data, size_t length);
