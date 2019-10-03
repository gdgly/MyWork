/********************************************************************************
*Author :       explorersoftware.taobao.com 
*Time   :       2019年8月24日15:44:36
*Explain:       通用头文件
*
********************************************************************************/

#ifndef __UHEAD_H
#define __UHEAD_H

#include "stm8l15x.h"
#include "stm8l15x_gpio.h"
#include "UDATA.H"



#define DEBUG_LEVEL 1		//调试接口

#define SWAPPER_UART 0		// 定义串口发射TX脚 1 -->PA2 ；0 -->PC3

#if 	SWAPPER_UART > 0 
#define	SWAPPER_UART_PA2_PC3()		SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortA,ENABLE);	//端口重映射，去掉注释之后USART1为PA2-TX、PA3-RX；注释之后USART1为TX-PC5、RX-PC6；复位之后USART会自动恢复至PC5、PC6
#else
#define	SWAPPER_UART_PA2_PC3()
#endif


#if  DEBUG_LEVEL > 0
#include "stm8l15x_usart.h"
#include "stdio.h"
#define  debug(...) 		printf(__VA_ARGS__);
#define	UART_INIT(baud)	do{\
	RST->CR = 0xD0;\
	SWAPPER_UART_PA2_PC3();	\
	CLK_PeripheralClockConfig (CLK_Peripheral_USART1,ENABLE);\
	USART_Init(USART1,baud, USART_WordLength_8b,USART_StopBits_1,USART_Parity_No,USART_Mode_Tx);\
	debug("\r\nstart:\r\n");\
}while(0)
#else
#define	UART_INIT(baud) RST->CR = 0x00;	//恢复RST脚为复位脚
#define debug(...)   
#endif

#define BEEP_SW 0
#if BEEP_SW > 0
#define		GPIO_BEEP			GPIOA,GPIO_Pin_0		// 蜂鸣器
#else
#define		GPIO_BEEP			GPIOC,GPIO_Pin_0		// 蜂鸣器
#endif

#ifndef 	TRUE
#define		TRUE	1
#endif

#ifndef 	FALSE
#define		FALSE	0
#endif


#define delay_ms(x)     	LSI_delayms(x/8)
#define delay_us(x)     	LSI_delayus(x/8)

#define	GPIO_SET(pin)		GPIO_SetBits(pin)
#define	GPIO_RESET(pin)		GPIO_ResetBits(pin)
#define	GPIO_READ(pin)		GPIO_ReadInputDataBit(pin)



//定时判断数据类型
typedef struct{
	u8 start;			//启动定时
	u8 switchon;		//定时到，开关打开
	u32 counter;		//计数
}JugeCStr;

typedef enum {ISOK = 0, ISERROR = 1} Flag_Status;

/*公用函数*/

void LSI_delayus(unsigned int  nCount);   //16M 晶振时  延时 1个微妙
void LSI_delayms(unsigned int  nCount);
bool Juge_counter(JugeCStr* juge, u32 swdat);

//故障编码
#define		ERROR_OK		0	// 无故障
#define		ERROR_BAT		(0x01)	// BAT电压过低
#define		ERROR_MOTOR		(0x02)	// 马达持续转动超时
#define		ERROR_CHARG		(0x04)	// 充电保护
#define		ERROR_BH		(0x10)	// BH没有波形


#endif

//	debug("sys clk souce: %d\r\n frq: %lu\r\n",CLK_GetSYSCLKSource(),CLK_GetClockFreq());