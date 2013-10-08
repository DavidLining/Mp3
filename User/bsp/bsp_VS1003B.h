#ifndef __BSP_VS1003B_H
#define __BSP_VS1003B_H

#include "stm32f10x.h"

#define VS_WRITE_COMMAND 	0x02
#define VS_READ_COMMAND 	0x03

//VS1003ºƒ¥Ê∆˜∂®“Â
#define SCI_MODE        	0x00   
#define SCI_STATUS      	0x01   
#define SCI_BASS        	0x02   
#define SCI_CLOCKF      	0x03   
#define SCI_DECODE_TIME 	0x04   
#define SCI_AUDATA      	0x05   
#define SCI_WRAM        	0x06   
#define SCI_WRAMADDR    	0x07   
#define SCI_HDAT0       	0x08   
#define SCI_HDAT1       	0x09 
  
#define SCI_AIADDR      	0x0a   
#define SCI_VOL         	0x0b   
#define SCI_AICTRL0     	0x0c   
#define SCI_AICTRL1     	0x0d   
#define SCI_AICTRL2     	0x0e   
#define SCI_AICTRL3     	0x0f   
#define SM_DIFF         	0x01   
#define SM_JUMP         	0x02   
#define SM_RESET        	0x04   
#define SM_OUTOFWAV     	0x08   
#define SM_PDOWN        	0x10   
#define SM_TESTS        	0x20   
#define SM_STREAM       	0x40   
#define SM_PLUSV        	0x80   
#define SM_DACT         	0x100   
#define SM_SDIORD       	0x200   
#define SM_SDISHARE     	0x400   
#define SM_SDINEW       	0x800   
#define SM_ADPCM        	0x1000   
#define SM_ADPCM_HP     	0x2000 		 

#define VS_VOL_MUTE		0xFE  /* æ≤“Ù,”√”⁄…Ë÷√SCI_VOLºƒ¥Ê∆˜ */


void vs1003_Init(void);
uint8_t vs1003_TestRam(void);
void vs1003_TestSine(void);
void vs1003_SoftReset(void);
void ResetDecodeTime(void);

uint8_t vs1003_ReqNewData(void);
void vs1003_PreWriteData(void);
void vs1003_WriteData(uint8_t _ucData);

void vs1003_SetVolume(uint8_t _ucVol);

#endif

















