/* Arduino Sd2Card Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino Sd2Card Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino Sd2Card Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#define __NOP __nop

#include "driverlib.h"

#include "Sd2Card.h"

extern "C"
{
	#include "setclockspeeds.h"
}


#define SD_TRACE(N, M)

// SPI functions
//==============================================================================
// functions for hardware SPI
//------------------------------------------------------------------------------
/**
 * initialize SPI pins
 */
static void spiBegin() {

	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN5 | GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);
}
//------------------------------------------------------------------------------
/**
 * Initialize hardware SPI
 * Set SCK rate to F_CPU/pow(2, 1 + spiRate) for spiRate [0,6]
 */
static void spiInit(eUSCI_SPI_MasterConfig *config) {

	MAP_SPI_initMaster(EUSCI_B0_BASE, config);
	MAP_SPI_enableModule(EUSCI_B0_BASE);
}
//------------------------------------------------------------------------------
/** SPI receive a byte */
static  uint8_t spiRec() {
	MAP_SPI_transmitData(EUSCI_B0_BASE, 0xFF);
	return MAP_SPI_receiveData(EUSCI_B0_BASE);
}
//------------------------------------------------------------------------------
/** SPI receive multiple bytes */
static uint8_t spiRec(uint8_t* buf, size_t n) {
  for(size_t i = 0; i < n; i++)
  {
	  buf[i] = spiRec();
  }
  return 0;
}
//------------------------------------------------------------------------------
/** SPI send a byte */
static void spiSend(uint8_t b) {
	MAP_SPI_transmitData(EUSCI_B0_BASE, b);
	MAP_SPI_receiveData(EUSCI_B0_BASE);
}
//------------------------------------------------------------------------------
static void spiSend(const uint8_t* buf , size_t n) {
  for(size_t i = 0; i < n; i++)
  {
	  spiSend(buf[i]);
  }
}
//==============================================================================
// Sd2Card member functions
//------------------------------------------------------------------------------
// send command and return error code.  Return zero for OK
uint8_t Sd2Card::cardCommand(uint8_t cmd, uint32_t arg) {
  // select card
  chipSelectLow();

  // wait up to 300 ms if busy
  waitNotBusy(300);

  uint8_t *pa = reinterpret_cast<uint8_t *>(&arg);

  // send command
  spiSend(cmd | 0x40);

  // send argument
  for (int8_t i = 3; i >= 0; i--) spiSend(pa[i]);

  // send CRC - correct for CMD0 with arg zero or CMD8 with arg 0X1AA
  spiSend(cmd == CMD0 ? 0X95 : 0X87);

  // skip stuff byte for stop read
  if (cmd == CMD12) spiRec();

  // wait for response
  for (uint8_t i = 0; ((status_ = spiRec()) & 0X80) && i != 0XFF; i++);
  return status_;
}
//------------------------------------------------------------------------------
/**
 * Determine the size of an SD flash memory card.
 *
 * \return The number of 512 byte data blocks in the card
 *         or zero if an error occurs.
 */
uint32_t Sd2Card::cardSize() {
  csd_t csd;
  if (!readCSD(&csd)) return 0;
  if (csd.v1.csd_ver == 0) {
    uint8_t read_bl_len = csd.v1.read_bl_len;
    uint16_t c_size = (csd.v1.c_size_high << 10)
                      | (csd.v1.c_size_mid << 2) | csd.v1.c_size_low;
    uint8_t c_size_mult = (csd.v1.c_size_mult_high << 1)
                          | csd.v1.c_size_mult_low;
    return (uint32_t)(c_size + 1) << (c_size_mult + read_bl_len - 7);
  } else if (csd.v2.csd_ver == 1) {
    uint32_t c_size = 0X10000L * csd.v2.c_size_high + 0X100L
                      * (uint32_t)csd.v2.c_size_mid + csd.v2.c_size_low;
    return (c_size + 1) << 10;
  } else {
    error(SD_CARD_ERROR_BAD_CSD);
    return 0;
  }
}
//------------------------------------------------------------------------------
void Sd2Card::chipSelectHigh() {
	MAP_GPIO_setOutputHighOnPin(chipSelectPort_, chipSelectPin_);
	spiSend(0XFF);
}
//------------------------------------------------------------------------------
void Sd2Card::chipSelectLow() {
  spiInit(&spiConfig_);
  MAP_GPIO_setOutputLowOnPin(chipSelectPort_, chipSelectPin_);
}
//------------------------------------------------------------------------------
/** Erase a range of blocks.
 *
 * \param[in] firstBlock The address of the first block in the range.
 * \param[in] lastBlock The address of the last block in the range.
 *
 * \note This function requests the SD card to do a flash erase for a
 * range of blocks.  The data on the card after an erase operation is
 * either 0 or 1, depends on the card vendor.  The card must support
 * single block erase.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::erase(uint32_t firstBlock, uint32_t lastBlock) {
  csd_t csd;
  if (!readCSD(&csd)) goto fail;
  // check for single block erase
  if (!csd.v1.erase_blk_en) {
    // erase size mask
    uint8_t m = (csd.v1.sector_size_high << 1) | csd.v1.sector_size_low;
    if ((firstBlock & m) != 0 || ((lastBlock + 1) & m) != 0) {
      // error card can't erase specified area
      error(SD_CARD_ERROR_ERASE_SINGLE_BLOCK);
      goto fail;
    }
  }
  if (type_ != SD_CARD_TYPE_SDHC) {
    firstBlock <<= 9;
    lastBlock <<= 9;
  }
  if (cardCommand(CMD32, firstBlock)
    || cardCommand(CMD33, lastBlock)
    || cardCommand(CMD38, 0)) {
      error(SD_CARD_ERROR_ERASE);
      goto fail;
  }
  if (!waitNotBusy(SD_ERASE_TIMEOUT)) {
    error(SD_CARD_ERROR_ERASE_TIMEOUT);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Determine if card supports single block erase.
 *
 * \return The value one, true, is returned if single block erase is supported.
 * The value zero, false, is returned if single block erase is not supported.
 */
bool Sd2Card::eraseSingleBlockEnable() {
  csd_t csd;
  return readCSD(&csd) ? csd.v1.erase_blk_en : false;
}
//------------------------------------------------------------------------------
/**
 * Initialize an SD flash memory card.
 *
 * \param[in] sckRateID SPI clock rate selector. See setSckRate().
 * \param[in] chipSelectPin SD chip select pin number.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.  The reason for failure
 * can be determined by calling errorCode() and errorData().
 */
bool Sd2Card::init(uint32_t sckRateID, uint8_t chipSelectPort, uint16_t chipSelectPin) {
  errorCode_ = type_ = 0;
  chipSelectPort_ = chipSelectPort;
  chipSelectPin_ = chipSelectPin;
  // 16-bit init start time allows over a minute
  start_timeout(SD_INIT_TIMEOUT);
  uint32_t arg;

  MAP_GPIO_setAsOutputPin(chipSelectPort_, chipSelectPin_);
  MAP_GPIO_setOutputLowOnPin(chipSelectPort_, chipSelectPin_);
  spiBegin();

  // set SCK rate for initialization commands
  spiConfig_ = (eUSCI_SPI_MasterConfig) {
			EUSCI_SPI_CLOCKSOURCE_SMCLK,
			CLOCKSPEED_SMCLK_FREQ,
			// gets us the closest rate possible with the current clock frequency
			CLOCKSPEED_SMCLK_FREQ / (CLOCKSPEED_SMCLK_FREQ / SPI_SD_INIT_RATE),
			EUSCI_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
			EUSCI_SPI_CLOCKPOLARITY_INACTIVITY_LOW,
			EUSCI_SPI_3PIN
	};
  spiInit(&spiConfig_);

  // must supply min of 74 clock cycles with CS high.
  for (uint8_t i = 0; i < 10; i++) spiSend(0XFF);

  // command to go idle in SPI mode
  while (cardCommand(CMD0, 0) != R1_IDLE_STATE) {
    if (0 && !timeout_running()) {
      error(SD_CARD_ERROR_CMD0);
      goto fail;
    }
  }
  // check SD version
  while (1) {
    if (cardCommand(CMD8, 0x1AA) == (R1_ILLEGAL_COMMAND | R1_IDLE_STATE)) {
      type(SD_CARD_TYPE_SD1);
      break;
    }
    for (uint8_t i = 0; i < 4; i++) status_ = spiRec();
    if (status_ == 0XAA) {
      type(SD_CARD_TYPE_SD2);
      break;
    }
    if (!timeout_running()) {
      error(SD_CARD_ERROR_CMD8);
      goto fail;
    }
  }
  // initialize card and send host supports SDHC if SD2
  arg = type() == SD_CARD_TYPE_SD2 ? 0X40000000 : 0;

  while (cardAcmd(ACMD41, arg) != R1_READY_STATE) {
    // check for timeout
    if (!timeout_running()) {
      error(SD_CARD_ERROR_ACMD41);
      goto fail;
    }
  }
  // if SD2 read OCR register to check for SDHC card
  if (type() == SD_CARD_TYPE_SD2) {
    if (cardCommand(CMD58, 0)) {
      error(SD_CARD_ERROR_CMD58);
      goto fail;
    }
    if ((spiRec() & 0XC0) == 0XC0) type(SD_CARD_TYPE_SDHC);
    // discard rest of ocr - contains allowed voltage range
    for (uint8_t i = 0; i < 3; i++) spiRec();
  }
  chipSelectHigh();
  error(0);
  return setSckRate(sckRateID);

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/**
 * Read a 512 byte block from an SD card.
 *
 * \param[in] blockNumber Logical block to be read.
 * \param[out] dst Pointer to the location that will receive the data.

 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readBlock(uint32_t blockNumber, uint8_t* dst) {
  SD_TRACE("RB", blockNumber);
  // use address if not SDHC card
  if (type()!= SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD17, blockNumber)) {
    error(SD_CARD_ERROR_CMD17);
    goto fail;
  }
  return readData(dst, 512);

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Read one data block in a multiple block read sequence
 *
 * \param[in] dst Pointer to the location for the data to be read.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readData(uint8_t *dst) {
  chipSelectLow();
  return readData(dst, 512);
}
//------------------------------------------------------------------------------
bool Sd2Card::readData(uint8_t* dst, size_t count) {
  uint16_t crc;
  // wait for start block token
  start_timeout(SD_READ_TIMEOUT);
  while ((status_ = spiRec()) == 0XFF) {
    if (!timeout_running()) {
      error(SD_CARD_ERROR_READ_TIMEOUT);
      goto fail;
    }
  }
  if (status_ != DATA_START_BLOCK) {
    error(SD_CARD_ERROR_READ);
    goto fail;
  }
  // transfer data
  if (status_ = spiRec(dst, count)) {
    error(SD_CARD_ERROR_SPI_DMA);
    goto fail;
  }
  // get crc
  crc = (spiRec() << 8) | spiRec();

  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** read CID or CSR register */
bool Sd2Card::readRegister(uint8_t cmd, void* buf) {
  uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
  if (cardCommand(cmd, 0)) {
    error(SD_CARD_ERROR_READ_REG);
    goto fail;
  }
  return readData(dst, 16);

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Start a read multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 *
 * \note This function is used with readData() and readStop() for optimized
 * multiple block reads.  SPI chipSelect must be low for the entire sequence.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readStart(uint32_t blockNumber) {
  SD_TRACE("RS", blockNumber);
  if (type()!= SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD18, blockNumber)) {
    error(SD_CARD_ERROR_CMD18);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** End a read multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readStop() {
  chipSelectLow();
  if (cardCommand(CMD12, 0)) {
    error(SD_CARD_ERROR_CMD12);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/**
 * Set the SPI clock rate.
 *
 * \param[in] sckRateID A value in the range [0, 14].
 *
 * The SPI clock divisor will be set to approximately
 *
 * (2 + (sckRateID & 1)) << ( sckRateID/2)
 *
 * The maximum SPI rate is F_CPU/2 for \a sckRateID = 0 and the rate is
 * F_CPU/128 for \a scsRateID = 12.
 *
 * \return The value one, true, is returned for success and the value zero,
 * false, is returned for an invalid value of \a sckRateID.
 */
bool Sd2Card::setSckRate(uint32_t sckRateID) {
  spiRate_ = sckRateID;
  spiConfig_.desiredSpiClock = CLOCKSPEED_SMCLK_FREQ / (CLOCKSPEED_SMCLK_FREQ / spiRate_);
  MAP_SPI_changeMasterClock(EUSCI_B0_BASE, spiConfig_.clockSourceFrequency, spiConfig_.desiredSpiClock);
  return true;
}
//------------------------------------------------------------------------------
// wait for card to go not busy
bool Sd2Card::waitNotBusy(uint16_t timeoutMillis) {
  start_timeout(timeoutMillis);
  while (spiRec() != 0XFF) {
    if (!timeout_running()) goto fail;
  }
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
/**
 * Writes a 512 byte block to an SD card.
 *
 * \param[in] blockNumber Logical block to be written.
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeBlock(uint32_t blockNumber, const uint8_t* src) {
  SD_TRACE("WB", blockNumber);
  // use address if not SDHC card
  if (type() != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD24, blockNumber)) {
    error(SD_CARD_ERROR_CMD24);
    goto fail;
  }
  if (!writeData(DATA_START_BLOCK, src)) goto fail;
  // wait for flash programming to complete
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) {
    error(SD_CARD_ERROR_WRITE_TIMEOUT);
    goto fail;
  }
  // response is r2 so get and check two bytes for nonzero
  if (cardCommand(CMD13, 0) || spiRec()) {
    error(SD_CARD_ERROR_WRITE_PROGRAMMING);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Write one data block in a multiple block write sequence
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeData(const uint8_t* src) {
  chipSelectLow();
  // wait for previous write to finish
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  if (!writeData(WRITE_MULTIPLE_TOKEN, src)) goto fail;
  chipSelectHigh();
  return true;

 fail:
  error(SD_CARD_ERROR_WRITE_MULTIPLE);
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
// send one block of data for write block or write multiple blocks
bool Sd2Card::writeData(uint8_t token, const uint8_t* src) {
  uint16_t crc = 0XFFFF;

  spiSend(token);
  spiSend(src, 512);
  spiSend(crc >> 8);
  spiSend(crc & 0XFF);

  status_ = spiRec();
  if ((status_ & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
    error(SD_CARD_ERROR_WRITE);
    goto fail;
  }
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Start a write multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 * \param[in] eraseCount The number of blocks to be pre-erased.
 *
 * \note This function is used with writeData() and writeStop()
 * for optimized multiple block writes.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeStart(uint32_t blockNumber, uint32_t eraseCount) {
  SD_TRACE("WS", blockNumber);
  // send pre-erase count
  if (cardAcmd(ACMD23, eraseCount)) {
    error(SD_CARD_ERROR_ACMD23);
    goto fail;
  }
  // use address if not SDHC card
  if (type() != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD25, blockNumber)) {
    error(SD_CARD_ERROR_CMD25);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** End a write multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeStop() {
  chipSelectLow();
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  spiSend(STOP_TRAN_TOKEN);
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  chipSelectHigh();
  return true;

 fail:
  error(SD_CARD_ERROR_STOP_TRAN);
  chipSelectHigh();
  return false;
}
