 /*
 * Flash.c
 * Written By: Joshua Aitken
 * Date Created: 11/20/2016
 * Date Last Modified: 11/20/2016
 *
 * Requirements:
 */

#include "Flash.h"
#include "msp.h"
#include "driverlib.h"
//#include "ST7735.h"

void FlashWriteData(void* Fl_Location,char FlashData[200]);
void FlashSetup(void); //Sets Internal voltage level

//void main(){
//	FlashSetup();
//    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
//	Output_Init();
//	char SavedData[200];
//	SavedData[100] = '1';
//	SavedData[101] = '1';
//	SavedData[102] = ':';
//	SavedData[103] = '1';
//	SavedData[104] = '1';
//	SavedData[105] = ':';
//	SavedData[106] = '1';
//	SavedData[107] = '1';
//	SavedData[108] = ' ';
//	SavedData[109] = '1';
//	SavedData[110] = '1';
//	SavedData[111] = '/';
//	SavedData[112] = '1';
//	SavedData[113] = '1';
//	SavedData[114] = '/';
//	SavedData[115] = '1';
//	SavedData[116] = '1';
//	SavedData[117] = ' ';
//	SavedData[118] = 'D';
//	char * Addr_Pointer;
//	char Read_Back_Data[19];
//	FlashWriteData((void *)CALIBRATION_START,SavedData);
//	int i;
//	for(i = 0; i < 19; i++){
//		Addr_Pointer = ARM_EVENT1 + i;
//		Read_Back_Data[i] = *Addr_Pointer;
//	}
//	ST7735_OutString(Read_Back_Data);
//}

//Erases the entire flash sector @ Fl_Location and stores FlashData
//Input char array must have length = 200
void FlashWriteData(void* Fl_Location, char FlashData[200]){
	/* Unprotecting Info Bank 0, Sector 0  */
	MAP_FlashCtl_unprotectSector(FLASH_INFO_MEMORY_SPACE_BANK0,FLASH_SECTOR0);
	/* Erase the flash sector starting CALIBRATION_START. */
	while(!MAP_FlashCtl_eraseSector(CALIBRATION_START));
	/* Program the flash with the new data. */
	while (!MAP_FlashCtl_programMemory(FlashData, Fl_Location, 200));
	// Note: leave first byte unprogrammed
	/* Setting the sector back to protected */
	MAP_FlashCtl_protectSector(FLASH_INFO_MEMORY_SPACE_BANK0,FLASH_SECTOR0);
}

//Sets internal voltage and wait state
void FlashSetup(void){
	MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
	FlashCtl_setWaitState(FLASH_BANK0, 2);
	FlashCtl_setWaitState(FLASH_BANK1, 2);
}
