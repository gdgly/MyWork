#include "NRF_App.h"
#include "NRF24L01_AUTO_ACK.H"
#include "24l01.h"
#include "CUI_RTOS.H"
#include "LED_SHOW.H"
#include "stmflash.h"
Nrf24l01_PRXStr 	RXprx 		= {0};				// NRF接收结构体
u8 			RXtxbuf[7] 	= {0,0,0,0,0,'O','K'};		// nrf发送缓存
u8 			RXrxbuf[7] 	= {0};				        // nrf接收缓存			        
u8          DM_num = 0;

extern  u8 		            flag_duima  		;	//对码状态
extern  u8 		            flag_duima_clear  	;	//清除对码
extern  TaskStr* 	        taskNRF	            ;
extern  TaskLinkStr* 		tasklink            ;
extern  JugeCStr 		    NRFpowon 	        ;
extern  JugeCStr 		    NRFpowoff 	        ;
extern  JugeCStr 			NRFsleep;
extern 	u16					nrf_sleeptime 	;
extern 	u16					nrf_worktime	;
void NRF_SendCMD(Nrf24l01_PTXStr* ptx,u8* addr,u8 cmd , u8 mes);
void NRFReceived();
bool JugeRX();
void NRF_DM();
bool JugeDM();
void RXD_CallBack(Nrf24l01_PRXStr* prx) ;   // ��ͨģʽ������ɻص�����
void DMRXD_CallBack(Nrf24l01_PRXStr* prx);	// DMģʽ������ɻص�����

/*nrf��������*/
void NRF_Function()
{   
        if(taskNRF->state == Stop)
        {
          if(flag_duima == 0)
          {
            address = ADDRESS2;
            InitNRF_AutoAck_PRX(&RXprx,RXrxbuf,RXtxbuf,sizeof(RXtxbuf),BIT_PIP0,RF_CH_HZ);	//���ý���ģʽ
			debug("��ǰ��ͨѶ��ַ��");
			u8 i = 0;
			for(i =0;i<5;i++)
			{
				debug("%X	",address[i]);
			}
			debug("\r\n");
            RXprx.rxbuf = RXrxbuf;
            NRFpowon.start = 1;
            NRF24L01_PWR(0);
            RXprx.RXDCallBack = RXD_CallBack;
            OS_AddJudegeFunction(taskNRF,NRFReceived,100,JugeRX);
          }
          else
          {		
              NRFpowon.start = 0;
              NRFpowoff.start = 0;
              address = ADDRESS1;
              InitNRF_AutoAck_PRX(&RXprx,RXrxbuf,RXtxbuf,sizeof(RXtxbuf),BIT_PIP0,RF_CH_HZ);	//���ý���ģʽ
			  RXprx.txbuf[0] = ADDRESS3[0];
			  RXprx.txbuf[1] = ADDRESS3[1];
			  RXprx.txbuf[2] = ADDRESS3[2];
			  RXprx.txbuf[3] = ADDRESS3[3];
			  RXprx.txbuf[4] = ADDRESS3[4];
			  RXprx.txbuf[5] = 'D';
			  RXprx.txbuf[6] = 'M';
      		  NRF24L01_RX_AtuoACKPip(RXprx.txbuf,7,RXprx.pip);	//���Ӧ���ź�
              RXprx.RXDCallBack =  DMRXD_CallBack;
			  RXprx.rxbuf = RXrxbuf;
			  
              DM_num = DM_NUM;
			  NRF24L01_PWR(1);
			  NRFpowon.start = 0;
			  NRFpowon.counter = 0;
			  NRFpowoff.start = 0;
			  NRFpowoff.counter = 0;
              OS_AddJudegeFunction(taskNRF,NRF_DM,1000,JugeDM);
          }
           OS_AddTask(tasklink,taskNRF);	
        }
}


void ChangeNRFCmd(u8* buf);
// ��ѭ�������еģ���ʾ���յ�������
void NRFReceived()
{
	if(RXprx.hasrxlen)
	{
//		debug("\r\n");		
//		for(u8 i=0;i<RXprx.hasrxlen;i++)
//		  {
//			debug("[%d]=%d	",i,RXprx.rxbuf[i]);
//		  }		
//		debug("\r\n------\r\n\r\n");
		//�����յ����ֽڣ�ӳ�����Ӧ�Ŀ�������
	   debug("WAKE_UP\r\n");
	   NRF24L01_ClearFIFO();
		nrf_sleeptime = GZ_SLEEP_TIME;
		nrf_worktime = GZ_WORK_TIME;
		NRFsleep.start = 1;
		NRFsleep.counter = 0;
		ChangeNRFCmd(RXprx.rxbuf);
		RXprx.hasrxlen = 0;
	}
}

bool JugeRX()
{
  return (bool)flag_duima;
}

//�����ַ��flash
void SaveFlashAddr(u8* buf)
{
  ADDRESS2[0] = buf[0];
  ADDRESS2[1] = buf[1];
  ADDRESS2[2] = buf[2];
  ADDRESS2[3] = buf[3];
  ADDRESS2[4] = buf[4];
   		u8 i = 0;
	for(i =0;i<7;i++)
	{
		debug("buf[%d] = 0x%x\r\n",i,buf[i]);
	}
  FLASH_ProgramByte(EEPROM_ADDRESS0,ADDRESS2[0]);
  FLASH_ProgramByte(EEPROM_ADDRESS1,ADDRESS2[1]);
  FLASH_ProgramByte(EEPROM_ADDRESS2,ADDRESS2[2]);
  FLASH_ProgramByte(EEPROM_ADDRESS3,ADDRESS2[3]);
  FLASH_ProgramByte(EEPROM_ADDRESS4,ADDRESS2[4]);

}

// ���DM
void ClearDM()
{
     // debug("clear DM \r\n");
    FLASH_ProgramByte(EEPROM_ADDRESS0,ADDRESS1[0]);
    FLASH_ProgramByte(EEPROM_ADDRESS1,ADDRESS1[1]);
    FLASH_ProgramByte(EEPROM_ADDRESS2,ADDRESS1[2]);
    FLASH_ProgramByte(EEPROM_ADDRESS3,ADDRESS1[3]);
    FLASH_ProgramByte(EEPROM_ADDRESS4,ADDRESS1[4]);
    ADDRESS2[0] = ADDRESS1[0];
    ADDRESS2[1] = ADDRESS1[1];
    ADDRESS2[2] = ADDRESS1[2];
    ADDRESS2[3] = ADDRESS1[3];
    ADDRESS2[4] = ADDRESS1[4];
    address = ADDRESS2;
    InitNRF_AutoAck_PRX(&RXprx,RXrxbuf,RXtxbuf,sizeof(RXtxbuf),BIT_PIP0,RF_CH_HZ);	//���ý���ģʽ

}
void NRF_DM()
{
  
  DM_num--;
  if(DM_num&0x01)
    LEN_GREEN_Open();
  else LEN_GREEN_Close();
  if(DM_num == 0)
    flag_duima = 0;
}
bool JugeDM()
{
  return (bool)(flag_duima == 0);
}
//����ģʽ�Զ�������ɻص�����
void RXD_CallBack(Nrf24l01_PRXStr* prx) 
{
        prx->txbuf[0] = prx->rxbuf[0];
        prx->txbuf[1] = prx->rxbuf[1];
        prx->txbuf[2] = prx->rxbuf[2];
        prx->txbuf[3] = prx->rxbuf[3];
        prx->txbuf[4] = prx->rxbuf[4];
        prx->txbuf[5] = 'O';
        prx->txbuf[6] = 'K';
        NRF24L01_RX_AtuoACKPip(prx->txbuf,prx->txlen,prx->pip);//���Ӧ���ź�	
        prx->rxlen = NRF24L01_GetRXLen();
        NRF24L01_Read_Buf(RD_RX_PLOAD,prx->rxbuf + prx->hasrxlen,prx->rxlen);	//��ȡ����
        prx->hasrxlen += prx->rxlen;		
        NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,(1 << STATUS_BIT_IRT_RXD)); 	// ���RX_DS�жϱ�־
        debug("RX_OK ");
}


//�ɹ�״̬��LED��˸6�Σ�������ͬ��
void StateSuccess();
//DMģʽ������ɻص�����
void DMRXD_CallBack(Nrf24l01_PRXStr* prx)
{
	prx->rxlen = NRF24L01_GetRXLen();	
	NRF24L01_Read_Buf(RD_RX_PLOAD,prx->rxbuf,prx->rxlen);	//��ȡ����   	
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,(1 << STATUS_BIT_IRT_RXD)); 	// ���RX_DS�жϱ�־

	if(prx->rxbuf[5] == CMD_DM && prx->rxbuf[6] == MES_DM) 
	{ 
		SaveFlashAddr(prx->rxbuf);
		flag_duima = 0;
		StateSuccess();
	    debug("\r\n---DM���---\r\n");
	}
	prx->rxlen = 0;
}


//IRQ �ж�
INTERRUPT_HANDLER(EXTI2_IRQHandler,10)
{
		
	if(GPIO_READ(NRF24L01_IRQ_PIN) == 0) 
	{	
     // if(flag_duima == 0)	
		RXprx.IRQCallBack(&RXprx);
     // else
     //   TXptx.IRQCallBack(&TXptx);
	}
   	EXTI_ClearITPendingBit (EXTI_IT_Pin2);
}
