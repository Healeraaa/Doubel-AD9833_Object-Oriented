#include "SPIHard.h"

void SPI1_Init(void)
{
  // 定义并初始化SPI和GPIO的配置结构体
  LL_SPI_InitTypeDef SPI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  // 使能SPI1和GPIOA的时钟
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);  // 使能SPI1时钟
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA); // 使能GPIOA时钟

  /*SPI1 GPIO Configuration
    PA5   ------> SPI1_SCK
    PA7   ------> SPI1_MOSI
  */
  // 配置GPIO引脚为SPI功能
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5 | LL_GPIO_PIN_7;  // 选择PA5和PA7引脚
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;        // 设置为复用模式
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH; // 设置GPIO速度为非常高
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL; // 设置输出类型为推挽输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;               // 不使用上下拉电阻
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;             // 选择复用功能5（SPI1）
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);                // 初始化GPIOA

  // 配置SPI1的参数
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;         // 全双工模式
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;                      // 主模式
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;              // 数据宽度为8位
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;           // 时钟极性为高电平
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;                // 时钟相位为第二个边沿采样
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;                          // 软件控制NSS信号
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;      // 波特率预分频为2
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;                    // 高位先发送
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE; // 禁用CRC计算
  SPI_InitStruct.CRCPoly = 10;                                   // CRC多项式（未使用）
  LL_SPI_Init(SPI1, &SPI_InitStruct);                            // 初始化SPI1

  // 设置SPI协议为Motorola模式
  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);

  // 使能 SPI1
  LL_SPI_Enable(SPI1);
}


void SPI1_Transmit8(uint8_t data)
{
  // 等待发送缓冲区为空（可以写入新数据）
  while (!LL_SPI_IsActiveFlag_TXE(SPI1)){}
  // 发送数据
  LL_SPI_TransmitData8(SPI1, data);
  // 等待数据从发送缓冲区转移到移位寄存器
  while (!LL_SPI_IsActiveFlag_TXE(SPI1)){}
  // 等待 SPI 总线空闲（数据从移位寄存器发送完成）
  while (LL_SPI_IsActiveFlag_BSY(SPI1)){}
}

void SPI1_Transmit16(uint16_t data)
{
  // 等待发送缓冲区为空（可以写入新数据）
  while (!LL_SPI_IsActiveFlag_TXE(SPI1)){}
  // 发送数据
  LL_SPI_TransmitData16(SPI1, data);
  // 等待发送完成（确保数据从移位寄存器中发送出去）
  while (!LL_SPI_IsActiveFlag_TXE(SPI1)){}
  while (LL_SPI_IsActiveFlag_BSY(SPI1)){}
}

/**
 * @brief  SPI1 数据位宽转换
 * @param  dataWidth This parameter can be one of the following values:
          @arg @ref LL_SPI_DATAWIDTH_8BIT
          @arg @ref LL_SPI_DATAWIDTH_16BIT
 * @retval None
 */
void SPI1_SetDataWidth(uint32_t dataWidth)
{
  // 禁用 SPI，准备更改配置
  LL_SPI_Disable(SPI1);
  // 设置新的数据位宽（8位或16位）
  LL_SPI_SetDataWidth(SPI1, dataWidth);
  // 启用 SPI
  LL_SPI_Enable(SPI1);
}
