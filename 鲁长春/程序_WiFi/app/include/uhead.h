/**
 * 公共头文件
 */

#ifndef		__UHEAD_H
#define		__UHEAD_H

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "c_types.h"
#include "os_type.h"
#include "mem.h"

#define		SWITCH				5		// gpio5为继电器控制引脚
#define		LED_STATE_ADDR		100		// 保存LED灯状态的地址

void Switch_State(u8 state);	// 开关状态设置

#define	DEBUG_LEVEL 1

#if  DEBUG_LEVEL > 0
	#include "uart.h"
	#define		UART_INIT()			uart_init(115200,115200)
	#define  	debug(...) 			os_printf(__VA_ARGS__)
#else
	#define  	debug(...)
	#define		UART_INIT()
#endif


#define		malloc			os_zalloc
#define		free			os_free
#define		strlen(a)		os_strlen((const char*)a)		// 获取字符串长度
#define	 	memcpy			os_memcpy		//内存拷贝
#define		mem_set			os_memset

#define		GetU16LittleEnding(pdata)	(*((pdata)+1)|((u16)(*(pdata))<<8))	//获得16位的小端数值

#endif
