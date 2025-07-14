#ifndef __AD9833_H__
#define __AD9833_H__

#include <stdint.h>
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>
#include "Delay.h"

// 波形选择
#define FREQ_REGISTER_0 0x4000
#define PHASE_REGISTER_0 0xC000

// 波形选择宏
#define WAVE_NONE       0x00C0
#define WAVE_SINE       0x2000
#define WAVE_TRIANGLE   0x2002
#define WAVE_SQUARE     0x2028

typedef struct
{
	GPIO_TypeDef * SPI_SDA_PORT;
	GPIO_TypeDef * SPI_SCL_PORT;
    GPIO_TypeDef * SPI_CS1_PORT;
    GPIO_TypeDef * SPI_CS2_PORT;
	uint32_t SPI_SDA_PIN;
	uint32_t SPI_SCL_PIN;
	uint32_t SPI_CS1_PIN;
	uint32_t SPI_CS2_PIN;
}spi_bus_t;

typedef enum {
    FSYNC_BOTH = 0,   // 两个引脚一起
    FSYNC_1    = 1,   // 第一个引脚
    FSYNC_2    = 2    // 第二个引脚
} FSYNC_State_t;



// 定义 AD9833 结构体
typedef struct AD9833 {
    // void (*SPI_Send16Bit)(struct AD9833* self, uint16_t Byte);
    void (*WriteData)(struct AD9833* self,FSYNC_State_t n, uint16_t Data);
    void (*WaveMode)(struct AD9833 *self, FSYNC_State_t n, uint16_t wave);
    void (*SetFrequency)(struct AD9833 *self, FSYNC_State_t n, uint32_t freq);
    void (*SetPhase)(struct AD9833 *self, FSYNC_State_t n, uint16_t phase);
    void (*SetWave)(struct AD9833 *self,uint16_t Wave1,uint16_t Wave2,uint32_t freq1,uint32_t freq2,uint16_t phase1,uint16_t phase2);

    uint32_t mclk;      // 时钟频率
    uint16_t wave1;       //波形模式
    uint16_t wave2;       //波形模式
    uint32_t freq1;      //信号频率
    uint32_t freq2;      //信号频率
    uint16_t phase1;  
    uint16_t phase2;      

    // AD9833_Pins pins;  // 引脚控制对象
    spi_bus_t *pins;  // 引脚控制对象    
} AD9833;

// 公有方法声明
AD9833 *AD9833_Create(uint32_t mclk, spi_bus_t *pins);
void AD9833_Destroy(AD9833* obj);
void AD9833_Init(AD9833* self);
// void AD9833_WaveMode(AD9833 *self, FSYNC_State_t n, uint16_t wave);
// void AD9833_SetFrequency(AD9833 *self, FSYNC_State_t n, uint32_t freq);
// void AD9833_SetPhase(AD9833 *self, FSYNC_State_t n,uint16_t phase);

extern spi_bus_t AD9833_dev1;

// void GPIO_SDATA(int state);
// void GPIO_SCLK(int state);
// void GPIO_FSYNC_High(int state);
// void GPIO_FSYNC_Low(int state);

#endif // __AD9833_H__
