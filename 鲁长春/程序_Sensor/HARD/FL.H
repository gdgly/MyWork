/********************************************************************************
*Author :       
*Time   :       
*Explain:       
*
********************************************************************************/



#ifndef __FL_H
#define __FL_H

#include "UHEAD.H"

#define		GPIO_FLU		GPIOB,GPIO_Pin_4		// FL-U风力检测,下降沿中断


void FL_GPIO_Init();
void FL_CheckStart();
void FL_CheckStop();
void FL_Control();


#endif
