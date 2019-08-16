/**
	2019-8-16
	

*/
#include "uhead.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_flash.h"
#include "24l01.h"

u8 rxDate[12] = {0};

//时钟配置
void RCC_Config()
{
   //时钟初始化 16mhz                                          
    CLK->CKDIVR  &= 0XE7;   
    CLK->CKDIVR &= 0xF8;    
    CLK->ICKCR |= CLK_ICKCR_HSION;     
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_I2C1,DISABLE);    
    CLK_PeripheralClockConfig(CLK_Peripheral_SPI1,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_BEEP,DISABLE);    
    CLK_PeripheralClockConfig(CLK_Peripheral_DAC,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM1,DISABLE);    
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_LCD,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_DMA1,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_COMP,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_BOOTROM,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_AES,DISABLE);    
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM5,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_SPI2,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART2,DISABLE);    
    CLK_PeripheralClockConfig(CLK_Peripheral_USART3,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_BOOTROM,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_CSSLSE,DISABLE); 
}

//没有用到的引脚设置低功耗
void FreeGPIO_Config()
{
  GPIO_Init(GPIOA,GPIO_Pin_1,GPIO_Mode_Out_PP_High_Slow);
  GPIO_Init(GPIOA,GPIO_Pin_2,GPIO_Mode_Out_PP_High_Slow);
  GPIO_Init(GPIOA,GPIO_Pin_3,GPIO_Mode_Out_PP_High_Slow);
  GPIO_Init(GPIOB,GPIO_Pin_7,GPIO_Mode_Out_PP_High_Slow);
  GPIO_Init(GPIOC,GPIO_Pin_2,GPIO_Mode_Out_PP_High_Slow);
  GPIO_Init(GPIOA,GPIO_Pin_0,GPIO_Mode_Out_PP_High_Slow);
  
}
void main()
{    
   
    RCC_Config();
    FreeGPIO_Config();
	UART_INIT(115200);
	Init_NRF24L01();
	
    NRF24L01_RX_Mode();                         //配置接收模式 
	
    while(1)
    {           
        //NRF24L01程序
	  if(READ_IRQ_IN == 0 )
	  {
		//NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,0X40);
		//debug("get data \r\n");
		if(NRF24L01_RxPacket(rxDate)==0)       //接收到信息
		{
			debug(" rxDate[0] : %c  ",rxDate[0]);
			debug(" rxDate[1] : %c\r\n",rxDate[1]);
			rxDate[0] = 0;
			rxDate[1] = 0;

		}	  
	  }else
		debug("\n****************************************\n");

    }   
    
}

//TIM2更新中断,500ms
INTERRUPT_HANDLER(TIM2_UPD_OVF_TRG_BRK_USART2_TX_IRQHandler,19)
{

    TIM2->SR1 = (uint8_t)(~(uint8_t)TIM2_IT_Update);//TIM2_ClearITPendingBit(TIM2_IT_Update);     
}


//自动唤醒
INTERRUPT_HANDLER(RTC_CSSLSE_IRQHandler,4)
{       
   RTC_ClearITPendingBit(RTC_IT_WUT);  
}


#ifdef USE_FULL_ASSERT
void assert_failed(u8* file,u32 line)
{
  while(1);
}
#endif
              
             