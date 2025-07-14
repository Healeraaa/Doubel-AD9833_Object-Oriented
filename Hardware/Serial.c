#include "Serial.h"

uint8_t Serial_RxPacket[5]; // 定义接收数据包数组
uint8_t Serial_RxFlag;		// 定义接收数据包标志位
#define MODE 0x01
#define FREQ 0x02
#define PHASE 0x03

/**
 * 函    数：获取串口接收数据包标志位
 * 参    数：无
 * 返 回 值：串口接收数据包标志位，范围：0~1，接收到数据包后，标志位置1，读取后标志位自动清零
 */
uint8_t Serial_GetRxFlag(void)
{
	if (Serial_RxFlag == 1) // 如果标志位为1
	{
		Serial_RxFlag = 0;
		return 1; // 则返回1，并自动清零标志位
	}
	return 0; // 如果标志位为0，则返回0
}

void USART1_IRQ_Task(void)
{
	static uint8_t RxState = 0;					   // 定义表示当前状态机状态的静态变量
	static uint8_t pRxPacket = 0;				   // 定义表示当前接收数据位置的静态变量
	if (LL_USART_IsActiveFlag_RXNE(USART1) == SET) // 判断是否是USART1的接收事件触发的中断
	{
		uint8_t RxData = LL_USART_ReceiveData8(USART1); // 读取数据寄存器，存放在接收的数据变量

		/*使用状态机的思路，依次处理数据包的不同部分*/

		/*当前状态为0，接收数据包包头*/
		if (RxState == 0)
		{
			if (RxData == 0xFF) // 如果数据确实是包头
			{
				RxState = 1;   // 置下一个状态
				pRxPacket = 0; // 数据包的位置归零
			}
		}
		/*当前状态为1，接收数据包数据*/
		else if (RxState == 1)
		{
			Serial_RxPacket[pRxPacket] = RxData; // 将数据存入数据包数组的指定位置
			pRxPacket++;						 // 数据包的位置自增
			if (pRxPacket >= 5)					 // 如果收够5个数据
			{
				RxState = 2; // 置下一个状态
			}
		}
		/*当前状态为2，接收数据包包尾*/
		else if (RxState == 2)
		{
			if (RxData == 0xFE) // 如果数据确实是包尾部
			{
				Serial_RxFlag = 1; // 接收数据包标志位置1，成功接收一个数据包
			}
			RxState = 0; // 状态归0
		}
		// printf("The Data is %d\r\n",RxData);
		// printf("The State is %d, The Packet is %d\r\n",RxState,pRxPacket);
		// printf("The Number is %d %d %d",Serial_RxPacket[0],Serial_RxPacket[1],Serial_RxPacket[2]);
		LL_USART_ClearFlag_RXNE(USART1); // 清除标志位
	}
}
void USART_ComandTask(uint8_t comand)
{
	// static uint8_t mode;
	// static uint32_t fre;
	// static uint16_t phase;

	// switch (comand)
	// {
	// case MODE:
	// 	mode = Serial_RxPacket[1];
	// 	AD9833_WaveMode(0,mode);
	// 	printf("The mode is %d\r\n", Serial_RxPacket[1]);
	// 	break;
	// case FREQ:
	// 	fre = (Serial_RxPacket[1] << 24) | (Serial_RxPacket[2] << 16) | (Serial_RxPacket[3] << 8) | Serial_RxPacket[4];
	// 	Ad9833_SetFrequency(0,fre, 25000000);
	// 	printf("The fre is %u\r\n", fre);
	// 	break;
	// case PHASE:
	// 	phase = (Serial_RxPacket[3] << 8) | Serial_RxPacket[4];
	// 	Ad9833_single(mode,fre,phase);
	// 	// Ad9833_SetPhase(0,phase);
	// 	printf("The phase is %d\r\n", phase);
	// 	break;
	// default:
	// 	break;
	// }
}