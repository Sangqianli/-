#include "pwm.h"

int PWM3_Target=1000;//1130��ʼ�����1600


void Friction_PWM(int16_t pwm1,int16_t pwm2)
{
	PWM1 = pwm1+1000;	
	PWM2 = pwm2+1000;
}


void PWM3_Init(void)	//TIM3  Ħ����
{
	GPIO_InitTypeDef          gpio;
	TIM_TimeBaseInitTypeDef   tim;
	TIM_OCInitTypeDef         oc;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);		//TIM1--TIM8ʹ���ڲ�ʱ��ʱ,��APB2�ṩ

	gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&gpio);

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7, GPIO_AF_TIM3);      
	
	tim.TIM_Prescaler = 84-1;//ԭ1680-1
	tim.TIM_CounterMode = TIM_CounterMode_Up;		//���ϼ���
	tim.TIM_Period = 2500-1;	// 2.5ms(ÿ����) +1����+1us
	tim.TIM_ClockDivision = TIM_CKD_DIV1;		//����ʱ�ӷָ��Ϊ1�Ļ����2
	TIM_TimeBaseInit(TIM3,&tim);
	
	oc.TIM_OCMode = TIM_OCMode_PWM2;		//ѡ��ʱ��ģʽ
	oc.TIM_OutputState = TIM_OutputState_Enable;		//ѡ������Ƚ�״̬
	oc.TIM_OutputNState = TIM_OutputState_Disable;	//ѡ�񻥲�����Ƚ�״̬
	oc.TIM_Pulse = 0;		//���ô�װ�벶��Ƚ���������ֵ
	oc.TIM_OCPolarity = TIM_OCPolarity_Low;		//�����������
	oc.TIM_OCNPolarity = TIM_OCPolarity_High;		//���û����������
	oc.TIM_OCIdleState = TIM_OCIdleState_Reset;		//ѡ�����״̬�µķǹ���״̬
	oc.TIM_OCNIdleState = TIM_OCIdleState_Set;		//ѡ�񻥲�����״̬�µķǹ���״̬
	
	TIM_OC1Init(TIM3,&oc);		//ͨ��3
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);
	
	TIM_OC2Init(TIM3,&oc);		//ͨ��3
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);
				 
	TIM_ARRPreloadConfig(TIM3,ENABLE);
	
	TIM_CtrlPWMOutputs(TIM3,ENABLE);
	
	TIM_Cmd(TIM3,ENABLE);
	


		
		PWM1 = 1000;
		PWM2 = 1000;

}

void Feeding_Bullet_PWM(int16_t pwm1)
{
  if(pwm1<0)
		pwm1 = abs(pwm1);

	TIM3->CCR2=pwm1;
	TIM3->CCR1=pwm1;
}

void Magazine_Output_PWM(int16_t pwm)
{
	TIM1->CCR2=pwm;
}
