#include "driverlib.h"

#include "setclockspeeds.h"


// rough timing diagram for LRCLK, DCLK, and DATA in I2S
//
//       _______:                ...    :________________...____:                ...
// LRCLK        \________________...____/                ...    \________________...
//              :                       :                       :
//         _   _:  _   _   _   _ ...   _:  _   _   _   _ ...   _:  _   _   _   _ ...
// DCLK  _/ \_/ \_/ \_/ \_/ \_/ \...\_/ \_/ \_/ \_/ \_/ \...\_/ \_/ \_/ \_/ \_/ \...
//              :                       :                       :
//       ___ ___:___ ___ ___ ___ ... ___:___ ___ ___ ___ ... ___:___ ___ ___ ___ ...
// DATA  _*_X_*_X_*_X_0_X_1_X_2_X...X_E_X_F_X_0_X_1_X_2_X...X_E_X_F_X_0_X_1_X_2_X...
//              :                       :                       :
//              :                       :                       :
//         startup  | --- left  channel --- | --- right channel --- | --- left ch...
//
// note that changes in DATA and LRCLK occur on falling edge of DCLK,
// and that LRCLK toggles 1 DCLK-cycle before the switch between left and right channels





// Half the samples are for left channel, half for right...
// the DAC requires both channels be sent, and using DMA
// means we have to interleave both channels in memory,
// even though one is not used.
// Oh well.
#define SOUND_SAMPLE_N_BYTES_PER_CHANNEL 2 // 16bps = 2Bps
#define SOUND_N_CHANNELS 2 // left and right channels
#define SOUND_SAMPLES_PER_SECOND 8000 // 8kHz
#define SOUND_BUFFER_N_SAMPLES 80000 // number of samples, enough for 10ms at 8kHz


#pragma DATA_ALIGN(dma_control_table, 1024)
uint8_t dma_control_table[1024]; // empty memory for DMA module to use for control table

// Buffers for sound
#define SOUND_BUFFER_N_BYTES (SOUND_SAMPLE_N_BYTES_PER_CHANNEL * SOUND_N_CHANNELS * SOUND_BUFFER_N_SAMPLES)
static uint8_t sound_buffer_A[SOUND_BUFFER_N_BYTES] = {0x7F};
static uint8_t sound_buffer_B[SOUND_BUFFER_N_BYTES] = {0x7F};




// SPI interface is EUSCI_B2 on pins 3.5-3.7
#define SOUND_SPI_PORT GPIO_PORT_P3
#define SOUND_DCLK_PIN GPIO_PIN5
#define SOUND_DATA_PIN GPIO_PIN6

#define SOUND_LRCLK_RESET_PORT GPIO_PORT_P4
#define SOUND_LRCLK_RESET_PIN GPIO_PIN2

#define SOUND_SPI_MODULE EUSCI_B2_BASE

#define SOUND_DMA_CHANNEL DMA_CH4_EUSCIB2TX0

#define SOUND_DMA_CHANNEL_ENABLE DMA_CHANNEL_4


static const eUSCI_SPI_MasterConfig sound_spi_config =
{
	.selectClockSource = EUSCI_SPI_CLOCKSOURCE_SMCLK,
	.clockSourceFrequency = CLOCKSPEED_SMCLK_FREQ,
	.desiredSpiClock = 256000,

	.msbFirst = EUSCI_SPI_MSB_FIRST,

	// clock phase and polarity are set so data is changed on falling edge
	.clockPhase = EUSCI_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
	.clockPolarity = EUSCI_SPI_CLOCKPOLARITY_INACTIVITY_LOW,

	.spiMode = EUSCI_SPI_3PIN
};


void DMA_INT0_IRQHandler()
{
	uint32_t ui32Mode;

	// Check if the ping-pong "A" transfer is complete.
	ui32Mode = MAP_DMA_getChannelMode( SOUND_DMA_CHANNEL | UDMA_PRI_SELECT );
	if(ui32Mode == UDMA_MODE_STOP)
	{
		MAP_DMA_setChannelTransfer(SOUND_DMA_CHANNEL | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG, sound_buffer_A,
					(void*) MAP_UART_getTransmitBufferAddressForDMA(SOUND_SPI_MODULE), SOUND_BUFFER_N_BYTES);

		for(int i = 0; i < )
	}

	// Check if the ping-pong "B" transfer is complete.
	ui32Mode = MAP_DMA_getChannelMode( SOUND_DMA_CHANNEL | UDMA_ALT_SELECT );
	if(ui32Mode == UDMA_MODE_STOP)
	{
		MAP_DMA_setChannelTransfer(SOUND_DMA_CHANNEL | UDMA_ALT_SELECT, UDMA_MODE_PINGPONG, sound_buffer_B,
				(void*) MAP_UART_getTransmitBufferAddressForDMA(SOUND_SPI_MODULE), SOUND_BUFFER_N_BYTES);
	}

}


void setup_sound_dma()
{
	MAP_DMA_enableModule();
	MAP_DMA_setControlBase(dma_control_table);

	MAP_DMA_assignChannel(SOUND_DMA_CHANNEL);

	/* Disabling channel attributes of alternate DMA channel which will not be used in the basic DMA mode */
	MAP_DMA_disableChannelAttribute(SOUND_DMA_CHANNEL,  UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

	/* Setting control parameters: 'UDMA_SIZE_8' and 'UDMA_SRC_INC_8' are selected because TX/RX data buffer size limited to one character */
	// primary channel
	MAP_DMA_setChannelControl(SOUND_DMA_CHANNEL | UDMA_PRI_SELECT,  UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_1);
	// secondary channel
	MAP_DMA_setChannelControl(SOUND_DMA_CHANNEL | UDMA_ALT_SELECT,  UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_1);


	/* Setting channel transfers */
	// primary channel
	MAP_DMA_setChannelTransfer(SOUND_DMA_CHANNEL | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG, sound_buffer_A,
			(void*) MAP_SPI_getTransmitBufferAddressForDMA(EUSCI_B2_BASE), SOUND_BUFFER_N_BYTES);
	// secondary channel
	MAP_DMA_setChannelTransfer(SOUND_DMA_CHANNEL | UDMA_ALT_SELECT, UDMA_MODE_PINGPONG, sound_buffer_B,
			(void*) MAP_SPI_getTransmitBufferAddressForDMA(EUSCI_B2_BASE), SOUND_BUFFER_N_BYTES);


	/* Assigning interrupts */
	MAP_DMA_assignInterrupt(DMA_INT0, SOUND_DMA_CHANNEL_ENABLE);
	MAP_DMA_clearInterruptFlag(DMA_CH4_EUSCIB2TX0 & 0x0F);

	/* Enable interrupts for both DMA channels */
	MAP_Interrupt_enableInterrupt(DMA_INT0);

	/* Enable DMA channel */
	MAP_DMA_enableChannel(SOUND_DMA_CHANNEL_ENABLE);
}

//
// In addition to DCLK, which is a regular SPI clock,
// there is a separate LRCLK to indicate the left and right
// channel of the audio data.
// This is useless to us, since we only use one channel, but
// is still required by the audio IC. We've implemented it
// in hardware so we can use the hardware SPI implementation
// and DMA. This setup routine resets LRCLK and sends an extra
// SPI 'tick' so LRCLK will have the correct timing.
//
void setup_sound_lrclk_timing()
{
	// prepare pins for initializing LRCLK
	MAP_GPIO_setAsOutputPin(SOUND_SPI_PORT, SOUND_DCLK_PIN);
	MAP_GPIO_setAsOutputPin(SOUND_SPI_PORT, SOUND_DATA_PIN);
	MAP_GPIO_setAsOutputPin(SOUND_LRCLK_RESET_PORT, SOUND_LRCLK_RESET_PIN);

	// prepare DCLK for falling edge
	MAP_GPIO_setOutputHighOnPin(SOUND_SPI_PORT, SOUND_DCLK_PIN);

	// send LRCLK reset pulse
	MAP_GPIO_setOutputHighOnPin(SOUND_LRCLK_RESET_PORT, SOUND_LRCLK_RESET_PIN);
	delay_us(1); // a few clock cycles for the LRCLK reset pulse (really only need a few hundred ns)
	MAP_GPIO_setOutputLowOnPin(SOUND_LRCLK_RESET_PORT, SOUND_LRCLK_RESET_PIN);

	delay_us(1); // a few cycles recovery time for LRCLK reset (really only need a few tens of ns)

	// trigger DCLK once to get correct timing later
	MAP_GPIO_setOutputLowOnPin(SOUND_SPI_PORT, SOUND_DCLK_PIN);
}


void setup_sound_spi()
{
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(
			SOUND_SPI_PORT,
			SOUND_DCLK_PIN | SOUND_DATA_PIN,
			GPIO_PRIMARY_MODULE_FUNCTION);
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(SOUND_SPI_PORT, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

	MAP_SPI_initMaster(SOUND_SPI_MODULE, &sound_spi_config);

    MAP_SPI_enableModule(SOUND_SPI_MODULE);
}


void sound_init()
{
	setup_sound_lrclk_timing();
	setup_sound_spi();
	setup_sound_dma();
}
