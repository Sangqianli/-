#include "can1.h"
int CAN1_ID;

void CAN1_Init()
{
	GPIO_InitTypeDef gpio;
	NVIC_InitTypeDef nvic;
	CAN_InitTypeDef can;
	CAN_FilterInitTypeDef filter;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,ENABLE);

	gpio.GPIO_Mode=GPIO_Mode_AF;
	gpio.GPIO_Pin=GPIO_Pin_1|GPIO_Pin_0;
	gpio.GPIO_OType = GPIO_OType_PP;//��©���
 	gpio.GPIO_PuPd = GPIO_PuPd_UP;//��������
 	gpio.GPIO_Speed = GPIO_Speed_100MHz;//
	GPIO_Init(GPIOD,&gpio);

	GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_CAN1);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_CAN1);

	nvic.NVIC_IRQChannel = CAN1_RX0_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
	
	can.CAN_TTCM = DISABLE;		//��ʱ�䴥��ͨ��ģʽ
	can.CAN_ABOM = DISABLE;		//����Զ����߹���
	can.CAN_AWUM = DISABLE;		//˯��ģʽͨ���������(���CAN->MCR��SLEEPλ)
	can.CAN_NART = DISABLE;		//��ֹ�����Զ����� ���߸������һ��CAN ��Ӱ�췢�� ��ʱ�ɸ�ΪENABLE
	can.CAN_RFLM = DISABLE;		//���Ĳ��������µĸ��Ǿɵ�
	can.CAN_TXFP = ENABLE;		//���ȼ��ɱ��ı�ʶ������
	can.CAN_BS1=CAN_BS1_9tq;
	can.CAN_BS2=CAN_BS2_4tq;
	can.CAN_Mode=CAN_Mode_Normal;
	can.CAN_Prescaler=3;
	can.CAN_SJW=CAN_SJW_1tq;
	CAN_Init(CAN1,&can);
	
	filter.CAN_FilterNumber=0;  							 			//������0
	filter.CAN_FilterMode=CAN_FilterMode_IdMask;   	//����ģʽ
	filter.CAN_FilterScale=CAN_FilterScale_32bit;   // 32λ��
	filter.CAN_FilterFIFOAssignment=0;              //������0������FIFO0
	filter.CAN_FilterActivation=ENABLE;   				  //���������
	filter.CAN_FilterIdHigh=0x0000;                 //32λID
	filter.CAN_FilterIdLow=0x0000;
	filter.CAN_FilterMaskIdHigh=0x0000;             //32λMask
	filter.CAN_FilterMaskIdLow=0x0000;
	CAN_FilterInit(&filter);
	
	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);    ////FIFO0��Ϣ�Һ��ж�����
}



void CAN1_Send(uint32_t Equipment_ID,int16_t Data0,int16_t Data1,int16_t Data2,int16_t Data3)		//���̵��
{		
	CanTxMsg TxMessage;
	
	TxMessage.StdId = Equipment_ID;					 //ʹ�õ���չID�����820R��ʶ��0X200
	TxMessage.IDE = CAN_ID_STD;				 //��׼ģʽ
	TxMessage.RTR = CAN_RTR_DATA;			 //����֡RTR=0��Զ��֡RTR=1
	TxMessage.DLC = 8;							 	 //���ݳ���Ϊ8�ֽ�

	TxMessage.Data[0] = Data0>>8; 
	TxMessage.Data[1] = Data0;
	TxMessage.Data[2] = Data1>>8; 
	TxMessage.Data[3] = Data1;
	TxMessage.Data[4] = Data2>>8; 
	TxMessage.Data[5] =	Data2;
	TxMessage.Data[6] = Data3>>8; 
	TxMessage.Data[7] =	Data3;

	CAN_Transmit(CAN1, &TxMessage);	//��������
}

 
 
 
 
 
 
void CAN1_RX0_IRQHandler()
{
	CanRxMsg RxMessage;
	 
	
	if(CAN_GetITStatus(CAN1,CAN_IT_FMP0)!=RESET)
	{
		CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);		//����жϹ���
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);		//����can����
		

		CAN1_ID=RxMessage.StdId-0x200;
		switch(CAN1_ID)
		{
			case 1:
				Chassis.speed_get=RxMessage.Data[2];
			  Chassis.speed_get=Chassis.speed_get<<8;
			  Chassis.speed_get=Chassis.speed_get|RxMessage.Data[3];
			  Chassis.PID_PVM.measure=Chassis.speed_get;//���̵��3510
			  break;


			case 4:
        Launcher_Dial.speed_get=RxMessage.Data[2];
        Launcher_Dial.speed_get=Launcher_Dial.speed_get<<8;
			  Launcher_Dial.speed_get=Launcher_Dial.speed_get|RxMessage.Data[3];
			  Launcher_Dial.PID_PVM.measure=Launcher_Dial.speed_get;
			  break;
			
			case 5:
			  	
				Gimbal_yaw_PPM.position_get_last=Gimbal_yaw_PPM.position_get_now;
			  Gimbal_yaw_PPM.position_get_now=RxMessage.Data[0];
			  Gimbal_yaw_PPM.position_get_now=Gimbal_yaw_PPM.position_get_now<<8;
			  Gimbal_yaw_PPM.position_get_now=Gimbal_yaw_PPM.position_get_now|RxMessage.Data[1];
			  if((Gimbal_yaw_PPM.position_get_last-Gimbal_yaw_PPM.position_get_now)>8100)
				{
					Gimbal_yaw_PPM.position_round++;
				}
				if((Gimbal_yaw_PPM.position_get_now-Gimbal_yaw_PPM.position_get_last)>8100)
				{
					Gimbal_yaw_PPM.position_round--;
				}
				Gimbal_yaw_PPM.PID_PPM.measure=(Gimbal_yaw_PPM.position_round*cnt_per_round+Gimbal_yaw_PPM.position_get_now-1588);
				
				
				Gimbal_yaw_PVM.speed_get=RxMessage.Data[2];
			  Gimbal_yaw_PVM.speed_get=Gimbal_yaw_PVM.speed_get<<8;
			  Gimbal_yaw_PVM.speed_get=Gimbal_yaw_PVM.speed_get|RxMessage.Data[3];				
			  break;
				
			case 6:
			  Gimbal_pitch_PPM.position_get_last=Gimbal_pitch_PPM.position_get_now;
			  Gimbal_pitch_PPM.position_get_now=RxMessage.Data[0];
			  Gimbal_pitch_PPM.position_get_now=Gimbal_pitch_PPM.position_get_now<<8;
			  Gimbal_pitch_PPM.position_get_now=Gimbal_pitch_PPM.position_get_now|RxMessage.Data[1];
			  if((Gimbal_pitch_PPM.position_get_last-Gimbal_pitch_PPM.position_get_now)>5150)
				{
					Gimbal_pitch_PPM.position_round++;
				}
				if((Gimbal_pitch_PPM.position_get_now-Gimbal_pitch_PPM.position_get_last)>5150)
				{
					Gimbal_pitch_PPM.position_round--;
				}			  
			  
			  Gimbal_pitch_PPM.PID_PPM.measure=(Gimbal_pitch_PPM.position_round*cnt_per_round+Gimbal_pitch_PPM.position_get_now-30);
			  
				Gimbal_pitch_PVM.speed_get=RxMessage.Data[2];
			  Gimbal_pitch_PVM.speed_get=Gimbal_pitch_PVM.speed_get<<8;
			  Gimbal_pitch_PVM.speed_get=Gimbal_pitch_PVM.speed_get|RxMessage.Data[3];		
				break;

			default :
			  break;
		}
				
				
			  
			
				
				
		
		
		
		
		
	}

}



