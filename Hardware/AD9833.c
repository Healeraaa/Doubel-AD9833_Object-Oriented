#include "AD9833.h"

spi_bus_t AD9833_dev1 =
    {
        .SPI_SDA_PORT = GPIOB,
        .SPI_SCL_PORT = GPIOB,
        .SPI_CS1_PORT = GPIOB,
        .SPI_CS2_PORT = GPIOB,
        .SPI_SDA_PIN = LL_GPIO_PIN_12,
        .SPI_SCL_PIN = LL_GPIO_PIN_13,
        .SPI_CS1_PIN = LL_GPIO_PIN_14,
        .SPI_CS2_PIN = LL_GPIO_PIN_15,
};

void AD9833_GPIO_init(spi_bus_t *dev)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 根据相应引脚开启时钟
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

    // 配置引脚为输出模式
    GPIO_InitStruct.Pin = dev->SPI_SDA_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(dev->SPI_SDA_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = dev->SPI_SCL_PIN;
    LL_GPIO_Init(dev->SPI_SCL_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = dev->SPI_CS1_PIN;
    LL_GPIO_Init(dev->SPI_CS1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = dev->SPI_CS2_PIN;
    LL_GPIO_Init(dev->SPI_CS2_PORT, &GPIO_InitStruct);
}

// 引脚操作函数
void SDATA(spi_bus_t *dev, int state)
{
    if (state)
    {
        LL_GPIO_SetOutputPin(dev->SPI_SDA_PORT, dev->SPI_SDA_PIN);
    }
    else
    {
        LL_GPIO_ResetOutputPin(dev->SPI_SDA_PORT, dev->SPI_SDA_PIN);
    }
}

void SCLK(spi_bus_t *dev, int state)
{
    if (state)
    {
        LL_GPIO_SetOutputPin(dev->SPI_SCL_PORT, dev->SPI_SCL_PIN);
    }
    else
    {
        LL_GPIO_ResetOutputPin(dev->SPI_SCL_PORT, dev->SPI_SCL_PIN);
    }
}

void FSYNC_High(spi_bus_t *dev, FSYNC_State_t state)
{
    switch (state)
    {
    case FSYNC_BOTH:
        LL_GPIO_SetOutputPin(dev->SPI_CS1_PORT, dev->SPI_CS1_PIN);
        LL_GPIO_SetOutputPin(dev->SPI_CS2_PORT, dev->SPI_CS2_PIN);
        break;
    case FSYNC_1:
        LL_GPIO_SetOutputPin(dev->SPI_CS1_PORT, dev->SPI_CS1_PIN);
        break;
    case FSYNC_2:
        LL_GPIO_SetOutputPin(dev->SPI_CS2_PORT, dev->SPI_CS2_PIN);
        break;
    default:
        break;
    }
}

void FSYNC_Low(spi_bus_t *dev, FSYNC_State_t state)
{
    switch (state)
    {
    case FSYNC_BOTH:
        LL_GPIO_ResetOutputPin(dev->SPI_CS1_PORT, dev->SPI_CS1_PIN);
        LL_GPIO_ResetOutputPin(dev->SPI_CS2_PORT, dev->SPI_CS2_PIN);
        break;
    case FSYNC_1:
        LL_GPIO_ResetOutputPin(dev->SPI_CS1_PORT, dev->SPI_CS1_PIN);
        break;
    case FSYNC_2:
        LL_GPIO_ResetOutputPin(dev->SPI_CS2_PORT, dev->SPI_CS2_PIN);
        break;
    default:
        break;
    }
}

// 这部分函数可以视为私有函数，只能在该文件内部使用
static void AD9833_SPI_Send16Bit(AD9833 *self, uint16_t Byte)
{
    uint8_t i;
    for (i = 0; i < 16; i++)
    {
        SDATA(self->pins, Byte & (0x8000 >> i)); // 调用 SDATA 引脚控制函数
        Delay_us(1);
        SCLK(self->pins, 0); // 调用 SCLK 引脚控制函数
        Delay_us(1);
        SCLK(self->pins, 1);
    }
}

static void AD9833_WriteData(AD9833 *self, FSYNC_State_t n, uint16_t Data)
{
    FSYNC_Low(self->pins, n); // 调用 FSYNC 引脚控制函数
    Delay_us(1);
    AD9833_SPI_Send16Bit(self, Data);
    FSYNC_High(self->pins, n); // 复位 FSYNC 引脚
    Delay_us(1);
}

void AD9833_WaveMode(AD9833 *self, FSYNC_State_t n, uint16_t wave)
{
    switch (n)
    {
    case FSYNC_BOTH:
        self->wave1 = wave;
        self->wave2 = wave;
        self->WriteData(self, n, self->wave1);
        break;
    case FSYNC_1:
        self->wave1 = wave;
        self->WriteData(self, n, self->wave1);
        break;
    case FSYNC_2:
        self->wave2 = wave;
        self->WriteData(self, n, self->wave2);
        break;
    default:
        break;
    }
}

void AD9833_SetFrequency(AD9833 *self, FSYNC_State_t n, uint32_t freq)
{
    uint32_t freq_reg_value = (uint32_t)((uint64_t)(freq) * (1 << 28) / (self->mclk));
    uint16_t freq_lsb = freq_reg_value & 0x3FFF;
    uint16_t freq_msb = (freq_reg_value >> 14) & 0x3FFF;
    switch (n)
    {
    case FSYNC_BOTH:
        self->freq1 = freq;
        self->freq2 = freq;
        break;
    case FSYNC_1:
        self->freq1 = freq;
        break;
    case FSYNC_2:
        self->freq2 = freq;
        break;
    default:
        break;
    }
    self->WriteData(self, n, FREQ_REGISTER_0 | freq_lsb);
    self->WriteData(self, n, FREQ_REGISTER_0 | freq_msb);
}

void AD9833_SetPhase(AD9833 *self, FSYNC_State_t n, uint16_t phase)
{
    uint16_t data, phase_temp;
    switch (n)
    {
    case FSYNC_BOTH:
        self->phase1 = phase & 0x0FFF;
        self->phase2 = phase & 0x0FFF;
        break;
    case FSYNC_1:
        self->phase1 = phase & 0x0FFF;
        break;
    case FSYNC_2:
        self->phase2 = phase & 0x0FFF;
        break;
    default:
        break;
    }
    phase_temp = phase & 0x0FFF; // 确保相位值在 0-4095 范围内
    data = PHASE_REGISTER_0 | phase_temp;
    self->WriteData(self, n, data);
}

void Ad9833_SetWave(AD9833 *self, uint16_t Wave1, uint16_t Wave2, uint32_t freq1, uint32_t freq2, uint16_t phase1, uint16_t phase2)
{
    self->wave1 = Wave1;            // 设置波形模式
    self->wave2 = Wave2;            // 设置波形模式
    self->freq1 = freq1;            // 设置信号频率
    self->freq2 = freq2;            // 设置信号频率
    self->phase1 = phase1 & 0x0FFF; // 确保相位值在 0-4095 范围内
    self->phase2 = phase2 & 0x0FFF; // 确保相位值在 0-4095 范围内

    self->WriteData(self, FSYNC_BOTH, 0x0100); // 复位
    self->WriteData(self, FSYNC_BOTH, 0x2100); // 选择写入
    Delay_ms(1);
    self->SetFrequency(self, FSYNC_1, self->freq1); // 设置频率
    Delay_ms(1);
    self->SetFrequency(self, FSYNC_2, self->freq2); // 设置频率
    Delay_ms(1);
    self->SetPhase(self, FSYNC_1, self->phase1); // 设置Phase1
    Delay_ms(1);
    self->SetPhase(self, FSYNC_2, self->phase2); // 设置Phase2
    Delay_ms(1);
    if (self->wave1 == self->wave2)
    {
        self->WaveMode(self, FSYNC_BOTH, self->wave1); // 设置波形模式
        Delay_ms(1);
    }
    else
    {
        self->WaveMode(self, FSYNC_1, self->wave1); // 设置波形模式
        Delay_ms(1);
        self->WaveMode(self, FSYNC_2, self->wave2); // 设置波形模式
        Delay_ms(1);
    }

    // 6. 【关键】清除复位位，启动输出
    // self->WriteData(self, FSYNC_BOTH, 0x2000); // 清除RESET位，启动输出
    // Delay_ms(1);
}

// 初始化AD9833
void AD9833_Init(AD9833 *self)
{
    AD9833_GPIO_init(self->pins);       // 初始化GPIO引脚
    FSYNC_High(self->pins, FSYNC_BOTH); // 复位 FSYNC 引脚
    SCLK(self->pins, 0);
    SDATA(self->pins, 1);
    self->WriteData(self, 0, 0x0100); // 复位
    self->WriteData(self, 0, 0x2100); // 选择写入
}

// 创建 AD9833 对象
AD9833 *AD9833_Create(uint32_t mclk, spi_bus_t *pins)
{
    AD9833 *obj = (AD9833 *)malloc(sizeof(AD9833));
    if (obj == NULL)
    {
        return NULL; // 内存分配失败
    }

    // obj->SPI_Send16Bit =  AD9833_SPI_Send16Bit;
    obj->WriteData = AD9833_WriteData;
    obj->WaveMode = AD9833_WaveMode;
    obj->SetFrequency = AD9833_SetFrequency;
    obj->SetPhase = AD9833_SetPhase;
    obj->SetWave = Ad9833_SetWave;

    // 设置时钟频率
    obj->mclk = mclk;
    // obj->wave1 = Wave[mode]; // 设置波形模式
    // obj->wave2 = Wave[mode]; // 设置波形模式
    // obj->freq1 = freq;       // 设置信号频率
    // obj->freq2 = freq;       // 设置信号频率
    // obj->phase1 = phase & 0x0FFF; // 确保相位值在 0-4095 范围内
    // obj->phase2 = phase & 0x0FFF; // 确保相位值在 0-4095 范围内
    obj->pins = pins; // 设置引脚配置

    // 默认的波形选择
    // obj->Wave[WAVE_NONE] = 0x00C0;
    // obj->Wave[WAVE_SINE] = 0x2000;
    // obj->Wave[WAVE_TRIANGLE] = 0x2002;
    // obj->Wave[WAVE_SQUARE] = 0x2028;

    return obj;
}

// 销毁 AD9833 对象
void AD9833_Destroy(AD9833 *obj)
{
    if (obj != NULL)
    {
        free(obj);
    }
}
