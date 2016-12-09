// work-around for driverlib bug in C++
#define __NOP __nop

#include "Sd2Card.h"
#include "SdFatConfig.h"

namespace
{
	static Sd2Card card;
}

extern "C"
{
	#include "sd_wrapper.h"
	#include "ST7735.h"

	int sd_init()
	{
		bool status = card.init();
		return status;
	}

	int sd_read_block(uint32_t block, uint8_t* result)
	{
		bool status = card.readBlock(block, result);
		return status;
	}

	int sd_write_block(uint32_t block, uint8_t* result)
	{
		bool status = card.writeBlock(block, result);
		return status;
	}

	int sd_error_code()
	{
		return card.errorCode();
	}
}
