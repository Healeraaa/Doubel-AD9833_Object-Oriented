#ifndef __SPIHARD_H__
#define __SPIHARD_H__

#include "main.h"

void SPI1_Init(void);
void SPI1_Transmit8(uint8_t data);
void SPI1_Transmit16(uint16_t data);
void SPI1_SetDataWidth(uint32_t dataWidth);



#endif 

