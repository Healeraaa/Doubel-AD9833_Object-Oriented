#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
#include <stdint.h>
#include "usart.h"
#include "AD9833.h"

extern uint8_t Serial_RxPacket[];

uint8_t Serial_GetRxFlag(void);
void USART1_IRQ_Task(void);
void USART_ComandTask(uint8_t comand);

#endif
