
#include "24l01.h"
#define	__IO	volatile
u8 RF_CH_HZ =111;                                  //频率0~125
u8  ADDRESS1[TX_ADR_WIDTH]={1,1,1,1,1};			// DM地址
u8  ADDRESS2[RX_ADR_WIDTH]={1,1,1,1,1};  		// DM成功后通讯地址 
u8  ADDRESS3[5]={0};                            //保存本地地址
u8  ADDRESS4[RX_ADR_WIDTH]={2,2,2,2,2};  		//与传感器通讯地址 
u8* address = ADDRESS1;

void NRF24L01_ResetAddr(u8* add)
{
	CE_OUT_0; 
	address = add;
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,address,TX_ADR_WIDTH);    //写本地地址	
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,address,RX_ADR_WIDTH); //写接收端地址	
	CE_OUT_1; 

}

//用ID号生产新的收发地址
void CreatNewAddr(u8* ChipID,u8* newAddr)
{
      u8 dat[5] = {0};
      dat[0] = ChipID[0]^ChipID[5];
      dat[1] = ChipID[1]^ChipID[11];
      dat[2] = ChipID[2]^ChipID[10];
      dat[3] = ChipID[3]^ChipID[9]^ChipID[6];
      dat[4] = ChipID[4]^ChipID[8]^ChipID[7];
      newAddr[0] ^= dat[0];
      newAddr[1] ^= dat[1];
      newAddr[2] ^= dat[2];
      newAddr[3] ^= dat[3];
      newAddr[4] ^= dat[4];
}

/*******************************************************************************
****函数名称:
****函数功能:获取芯片ID函数
****版本:V1.0
****日期:14-2-2014
****入口参数:无
****出口参数:无
****说明:96位唯一ID
********************************************************************************/
void Get_ChipID(u8 *ChipID)
{
	ChipID[0] = *(__IO u8 *)(0X4926); 
	ChipID[1] = *(__IO u8 *)(0X4927); 
	ChipID[2] = *(__IO u8 *)(0X4928);
    ChipID[3] = *(__IO u8 *)(0X4929);
	ChipID[4] = *(__IO u8 *)(0X492A); 
	ChipID[5] = *(__IO u8 *)(0X492B); 
	ChipID[6] = *(__IO u8 *)(0X492C);
	ChipID[7] = *(__IO u8 *)(0X492D); 
	ChipID[8] = *(__IO u8 *)(0X492E); 
	ChipID[9] = *(__IO u8 *)(0X492F);
	ChipID[10] = *(__IO u8 *)(0X4930); 
	ChipID[11] = *(__IO u8 *)(0X4931); 
}
//依据唯一ID，产生一个新地址
void NRF_CreatNewAddr(u8* addr)
{
    u8 chipID[12]= {0};
    Get_ChipID(chipID);
    CreatNewAddr(chipID,addr);
}
/*****************SPI时序函数******************************************/
u8 SPI2_ReadWriteByte(unsigned char date)
{
#ifdef DMA_SPI 
     while(SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE) == RESET);
        SPI_SendData(SPI1,date);
     while(SPI_GetFlagStatus(SPI1,SPI_FLAG_RXNE) == RESET);
        return SPI_ReceiveData(SPI1);   
#else
    unsigned char i;
   	for(i=0;i<8;i++)                // 循环8次
   	{
	  if(date&0x80)
	    MOSI_OUT_1;
	  else
	    MOSI_OUT_0;                 // byte最高位输出到MOSI
   	  date<<=1;                     // 低一位移位到最高位 
   	  SCLK_OUT_1;       
	  if(READ_MISO_IN)              // 拉高SCK，nRF24L01从MISO读入1位数据，同时从MOSI输出1位数据
   	   date|=0x01;       	        // 读MISO到byte最低位
   	  SCLK_OUT_0;            	        // SCK置低
 
   	}
	MOSI_OUT_0;
   return(date);           	// 返回读出的一字节    
#endif

}


//进入低功耗时释放GPIO
void NRF24L01_GPIO_Lowpower(void)
{
   // GPIO_Init(NRF24L01_IRQ_PIN,GPIO_Mode_In_PU_No_IT);      		//,当IRQ为低电平时为中断触发

    CSN_OUT_1;
    CE_OUT_0;
    MOSI_OUT_0;
    SCLK_OUT_0;
}


	  
//初始化24L01的IO口
void NRF24L01_GPIO_Init(void)
{ 	
  //引脚初始化
	MYGPIO_SETMODE_OUTPUT(NRF24L01_CE_PIN);
	MYGPIO_SETMODE_INPUT(NRF24L01_IRQ_PIN);
	MYGPIO_SETMODE_OUTPUT(NRF24L01_CSN_PIN);
	MYGPIO_SETMODE_OUTPUT(MOSI_PIN);
	MYGPIO_SETMODE_INPUT(MISO_PIN);
	SCLK_MODE_INIT();
}

void NRF24L01_GPIO_Work(void)
{
    NRF24L01_GPIO_Init();
  //  GPIO_Init(NRF24L01_IRQ_PIN,GPIO_Mode_In_PU_IT);      		//,当IRQ为低电平时为中断触发

}

//使能DPL动态长度
//pipNum 通道号
void NRF24L01_EnabelDPL(u8 pipNum)
{
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,NRF24L01_Read_Reg(NRF_READ_REG + EN_AA)|(1<<pipNum));        //自动应答 
	NRF24L01_Write_Reg(NRF_WRITE_REG + FEATURE, (1<<FEATURE_BIT_EN_DPL)|(1<<FEATURE_BIT_EN_ACK_PAY));	//使能动态长度
	NRF24L01_Write_Reg(NRF_WRITE_REG + DYNPD, NRF24L01_Read_Reg(NRF_READ_REG + DYNPD)|(1<<pipNum));	//使能通道动态长度 ,Requires EN_DPL and ENAA_P0
}
//初始化配置
u8 Init_NRF24L01(u8 rf_ch)
{
    u8 i = 10;
    NRF24L01_GPIO_Init();
    while(NRF24L01_Check() && i--)         //检测模块存在,如果不存在就周期1s切换继电器状态,让LED闪烁
    {            
        debug("NRF24L01_Check EEROR\r\n"); 
        delay_ms(1000);
    }
    NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,rf_ch);            //设置信道工作频率，收发必须一致
    NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,VALUE_RF_SETUP(RF_DR_250K,RF_PWR_7dBm));// NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x6f);   //SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x0f); //设置发射速率为2MHZ，发射功率为最大值0dB	
	return i;

}




//检测24L01是否存在
//返回值:0，成功;1，失败	
u8 NRF24L01_Check(void)
{
  u8 buf[2]={0XA5,0x33};   	 
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,2);        
        buf[0] = buf[1] = 0;	
	NRF24L01_Read_Buf(TX_ADDR,buf,2);                       //读出写入的地址  
	if(buf[0]!=0XA5 || buf[1]!=0X33)return 1;                                  //检测24L01错误	
        debug("buf = %X,%X\r\n",buf[0],buf[1]);
	return 0;		                                //检测到24L01
}

//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;
        SCLK_OUT_0;    
   	CSN_OUT_0;                              //使能SPI传输
  	status =SPI2_ReadWriteByte(reg);        //发送寄存器号 
  	SPI2_ReadWriteByte(value);              //写入寄存器的值
         
  	CSN_OUT_1;                              //禁止SPI传输	   
  	return(status);       			//返回状态值
}
//读取SPI寄存器值
//reg:要读的寄存器
u8 NRF24L01_Read_Reg(u8 reg)
{
	u8 reg_val;
    SCLK_OUT_0;
 	CSN_OUT_0;                              //使能SPI传输		
  	SPI2_ReadWriteByte(reg);                //发送寄存器号
  	reg_val=SPI2_ReadWriteByte(0XFF);       //读取寄存器内容
  	CSN_OUT_1;                              //禁止SPI传输		    
  	return(reg_val);                        //返回状态值
}
//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;
     SCLK_OUT_0;
  	CSN_OUT_0;                              //使能SPI传输
  	status=SPI2_ReadWriteByte(reg);         //发送寄存器值(位置),并读取状态值   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI2_ReadWriteByte(0XFF);//读出数据
  	CSN_OUT_1;                              //关闭SPI传输
  	return status;                          //返回读到的状态值
}
//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	u8 status,u8_ctr;
    	SCLK_OUT_0 ;
 	CSN_OUT_0;                              //使能SPI传输
  	status = SPI2_ReadWriteByte(reg);       //发送寄存器值(位置),并读取状态值
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI2_ReadWriteByte(*pBuf++); //写入数据
  	CSN_OUT_1;                              //关闭SPI传输
  	return status;                          //返回读到的状态值
}				   
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//size:数据的个数
//返回值:发送完成状况
u8 NRF24L01_TxPacket(u8 *txbuf,u8 size)
{  	
    SCLK_OUT_0 ;
	CE_OUT_0;                               //StandBy I模式	
  	NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,size);
 	CE_OUT_1;                               //启动发送 置高CE激发数据发送 

#ifdef DMA_SPI
	CLK_PeripheralClockConfig(CLK_Peripheral_SPI1,DISABLE);
#endif
	return 0;//其他原因发送失败
}
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:0，接收完成；其他，错误代码
u8 NRF24L01_RxPacket(u8 *rxbuf,u8* len)
{
 
	u8 sta;		    							      
	sta = NRF24L01_GetStatus();  //读取状态寄存器的值      
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&RX_OK)//接收到数据
	{
	  	*len = NRF24L01_GetRXLen();
		debug("len = %d  sta = %x\r\n",*len,sta);
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
		u8 txbuf[2] = {0x30,0x31};
		 NRF24L01_RX_AtuoACKPip(txbuf,2,BIT_PIP0);
               
		return 0; 
	}	   
	return 1;//没收到任何数据
}					    
//该函数初始化NRF24L01到RX模式
void NRF24L01_RX_Mode(u8 pip,u8* rxaddr)
{  
	debug("RX_Mode: pip %d\r\n",pip);
	CE_OUT_0; 
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0+pip,rxaddr,RX_ADR_WIDTH); 	//写本地收地址
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,NRF24L01_Read_Reg(NRF_READ_REG + EN_RXADDR)|(0x01<<pip));   //增加允许接收地址频道
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,0xff);	//清除中断标志
	NRF24L01_Write_Reg(FLUSH_RX,0x00); 			//清除RX_FIFO寄存器
	NRF24L01_Write_Reg(FLUSH_TX,0x00);	        //清除TX_FIFO寄存器 
	NRF24L01_Write_Reg(NRF_WRITE_REG + CONFIG, 0x0f);//IRQ引脚不显示中断 上电 接收模式   1~16CRC校验   
}		

void NRF24L01_ClearFIFO(void)
{	
	CE_OUT_0; 
	NRF24L01_Write_Reg(FLUSH_TX,0x00);	        //清除TX_FIFO寄存器 
	NRF24L01_Write_Reg(FLUSH_RX,0x00);	        //清除rX_FIFO寄存器 
	//CE_OUT_1; 
}
//该函数初始化NRF24L01到TX模式	   
void NRF24L01_TX_Mode(u8* addr)
{	
	debug("TX_Mode\r\n");
	CE_OUT_0; 
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,(REPEAT_DELAY<<4)|REPEAT_TIME); //设置自动重发间隔时间;最大自动重发次数
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,addr,TX_ADR_WIDTH);     //写本地地址
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,0xff); 	//清除中断标志
	NRF24L01_Write_Reg(FLUSH_RX,0x00); 			//清除RX_FIFO寄存器
	NRF24L01_Write_Reg(FLUSH_TX,0x00);	        //清除TX_FIFO寄存器 
	NRF24L01_Write_Reg(NRF_WRITE_REG + CONFIG,0x0e);    //IRQ引脚不显示TX,MAX中断,显示RX中断 上电 发射模式  1~16CRC校验
}

//1打开0关闭电源
void NRF24L01_PWR(u8 state)
{
#ifdef DMA_SPI
CLK_PeripheralClockConfig(CLK_Peripheral_SPI1,ENABLE);
#endif
	NRF24L01_GPIO_Work();
	CE_OUT_0; 
   	 u8 config = NRF24L01_Read_Reg(CONFIG);
    if(state)
	{
	    	
		NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,config|0x02);
		CE_OUT_1;

	}	
	else 
	{
		NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,config&0xFD);
		NRF24L01_GPIO_Lowpower();
	}
    
#ifdef DMA_SPI
CLK_PeripheralClockConfig(CLK_Peripheral_SPI1,DISABLE);	
#endif
}

//获取接收RX长度
u8 NRF24L01_GetRXLen(void)
{
    SCLK_OUT_0;    
   	CSN_OUT_0;                              // 使能SPI传输
  	SPI2_ReadWriteByte(R_RX_PL_WID);        // 发送寄存器号 
  	u8 dat =SPI2_ReadWriteByte(0xff);      	// 写入寄存器的值
  	CSN_OUT_1;                              // 禁止SPI传输
	
	return dat;
}

//RX ACK 自动回复，设置通道
void NRF24L01_RX_AtuoACKPip(u8 *txbuf,u8 size,u8 pip)
{
	 NRF24L01_Write_Buf(W_ACK_PAYLOAD|pip,txbuf,size);
}

// 获取状态status
u8 NRF24L01_GetStatus(void)
{
  	u8 status;
    	SCLK_OUT_0;
 	CSN_OUT_0;                              // 使能SPI传输		
  	status = SPI2_ReadWriteByte(NOP);       // 读取寄存器内容
  	CSN_OUT_1;                              // 禁止SPI传输
	return status;
}

//设置射频数据率，发射功率
void NRF24L01_SetRF_SETUP(u8 dr,u8 pwr)
{
    	CE_OUT_0; 
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,VALUE_RF_SETUP(dr,pwr));
}

void NRF24L01_RESUSE_TX()
{
  	CE_OUT_0; 
	NRF24L01_Write_Reg(REUSE_TX_PL,0x00);	      //TX重复发上次发送的消息  
	NRF24L01_Write_Reg(NRF_FIFO_STATUS,NRF24L01_Read_Reg(NRF_FIFO_STATUS)|0x40);	      //TX重复发上次发送的消息  
	CE_OUT_1;
	
}

//获取接收到数据的通道号，输入STATUS值
//返回值为0~5，如果为7则RXFIFO为空
u8 NRD24L01_GetPip(u8 status)
{
	return (status&0x0e)>>1;
}
