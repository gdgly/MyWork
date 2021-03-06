#include "ADC_CHECK.H"
#include "keyboard.h"
#include "MX830Motor.h"

JugeCStr 	jugeYS_No 	= {0};			// 无雨水，AM打开时计时开窗
JugeCStr 	jugeYS 		= {0};			// YS信号超警戒,启动计时器，超过4S，触发	
float 		YSdat 		= 0;			// ys-u
u16 		YS_CGdat 		= 0;		// YS遥控传感器的值
JugeCStr 	YS_30 		= {0};			// YS供电标志位，当按下按键30分钟不响应YS信号
u16			ys_timer30 	= 0;			// YS不响应计时
u8 flag_1	= 0;						// 进行YS检查的时候置一
extern 		TaskLinkStr* 	tasklink;	// 任务列表
extern 		WindowState	windowstate;
extern		TaskStr* 	taskYS;			// YS测量任务
extern		u8 		flag_flag_YS_SHUT;
//extern 		u8		flag_YS_isno;
extern		u8		flag_shut_time;
//根据AD值计算电池端电压
float BatteryGetAD(u16 ad)
{
	return (0.0026*ad);//(0.0024175824175*ad);
}	

//根据AD值计算YS端电压
float YSGetAD(u16 ad)
{
	return (0.0009*ad);//(0.0008058608058*ad);
}



//YS,FL,Battery的GPIO初始化
void GPIO_ADC_Init()
{
    GPIO_Init(Battery_GPIO,GPIO_Mode_In_FL_No_IT);
	GPIO_Init(BatControl_GPIO,GPIO_Mode_Out_PP_High_Slow);
    GPIO_Init(YS_GPIO,GPIO_Mode_In_FL_No_IT);  				
	GPIO_Init(CHARGE_PRO_PIN,GPIO_Mode_In_FL_No_IT); 		//充电保护，高电平需要保护
	GPIO_Init(YSD_GPIO,GPIO_Mode_Out_PP_Low_Slow);
}

//返回相应通道ADC值
uint16_t Get_ADC_Dat(hardChannel hard_channel)
{
    uint16_t dat= 0;
    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1,ENABLE);                              //打开ADC时钟                                                                            //等待时钟稳定
    ADC_Init(ADC1,ADC_ConversionMode_Single,ADC_Resolution_12Bit,ADC_Prescaler_1);      //单次转换，12位，1分频
    ADC_Cmd(ADC1,ENABLE);                                                               //使能
    ADC_ChannelCmd(ADC1,(ADC_Channel_TypeDef)hard_channel,ENABLE);                      //选择通道号 
    
    ADC_SoftwareStartConv (ADC1);                                                       //开启软件转换
    while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);                               //等待ADC转换完成         
    dat = ADC_GetConversionValue (ADC1);                                                //获取转换值
    ADC_ChannelCmd(ADC1,(ADC_Channel_TypeDef)hard_channel,DISABLE);                                        
    ADC_Cmd(ADC1,DISABLE); 
    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1,DISABLE);
    ADC_ClearFlag (ADC1,ADC_FLAG_EOC);                                                //清除相关标识
    
    return dat;
    
}

//YS检测任务
void YS_Function()
{
  
	if(motorStruct.dir == STOP || motorStruct.dir == MOTOR_NULL)
	{
	  	float ysdat = 0;
		GPIO_SET(YSD_GPIO);
		YSdat = YSGetAD(Get_ADC_Dat(YS_Channel));
		GPIO_RESET(YSD_GPIO);
		ysdat = YSGetAD(YS_CGdat);
		YSdat = (YSdat >= ysdat)?YSdat:ysdat;
		// YS_CGdat = 0;
		//debug(".\r\n");
		if(YSdat > VALVE_YS_D && YS_30.start == 0)	//超过报警阀值
		{
			if(jugeYS.switchon == 0 && windowstate != SHUTDOWN)
			{
				debug(" YSdat = %f\r\n",YSdat);
				if(ysdat > VALVE_YS_D && YSdat == ysdat)jugeYS.switchon = 1;
				else jugeYS.start = 1;	//开着窗
			}
			if(key_AM.val == on && windowstate == SHUTDOWN)						//关着窗
			{
				jugeYS_No.counter = 0;	//清空计数
				jugeYS_No.start = 0;
				jugeYS_No.switchon = 0;
			}			
		}
		else 
		{
			jugeYS.start = 0;
			jugeYS.counter = 0;
			jugeYS.switchon = 0;
			if(key_AM.val == on && windowstate == SHUTDOWN && jugeYS_No.switchon == 0 && flag_shut_time)//关着窗
				jugeYS_No.start = 1;	//开始无YS计时。超过阀值，自动开窗
		}	
	}

}

bool JugeYS()
{

    return (bool)(YS_30.start || ( key_AM.val == off && windowstate == SHUTDOWN) || ( flag_shut_time == 0 && windowstate == SHUTDOWN));
}


//YS控制
void YS_Control()
{
	static u8 flag_0 = 0;

	if(taskYS->state == Stop)
	{
		if(((windowstate == SHUTDOWN  && key_AM.val == off)|| YS_30.start \
		    ||(windowstate == SHUTDOWN  && flag_shut_time == 0)) && flag_0 == 0) 	//不检测YS
		{
			debug("\r\nremove YS check-->\r\n");
			flag_0 = 1;
			flag_1 = 0;
			jugeYS.start = 0;
			jugeYS.counter = 0;
			jugeYS.switchon = 0;
			flag_flag_YS_SHUT = 0;
			GPIO_RESET(YSD_GPIO);										//关闭YS电源				
		}else
		if(windowstate != SHUTDOWN && YS_30.start == 0 && flag_0)
		{
			debug("flag_0 = 0\r\n");
			flag_0 = 0;
		}else
		
		if(windowstate != SHUTDOWN && YS_30.start == 0 && flag_1 == 0 )	// 开着窗或者关着窗并且AM打开并且没有30分钟限制
		{

			flag_1= 1;
			debug("\r\nadd YS check ->\r\n");
			OS_AddJudegeFunction(taskYS,YS_Function,TIM_CHECKEYS,JugeYS);
			OS_AddTask(tasklink,taskYS);								// 添加检测任务
			
		}
	}

}

