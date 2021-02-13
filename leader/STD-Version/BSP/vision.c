#include "vision.h"

u8 Vision_Sum[60] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,};
u8 Vision_2[60] = {0};
bool vision_update_flag=false;
void Vision_Init()
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    DMA_Cmd(DMA2_Stream5, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ��USART1ʱ��

    //����1��Ӧ���Ÿ���ӳ��
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9����ΪUSART1
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10����ΪUSART1

    //USART1�˿�����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9��GPIOA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
    GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

    //USART1 ��ʼ������
    USART_InitStructure.USART_BaudRate = 115200;//����������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
    USART_Init(USART1, &USART_InitStructure); //��ʼ������1

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//Usart1 �ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;		//�����ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);	//ʹ�ܴ���1�����ж�
    //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	//ʹ�ܴ���1�����ж�

    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE); //ʹ�ܴ��ڵ�DMAģʽ


    DMA_InitStructure.DMA_Channel = DMA_Channel_4;													//ѡ��ͨ��4
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART1->DR);				//DMA�����ַ
    DMA_InitStructure.DMA_Memory0BaseAddr = (u32)(&Vision_Sum[0]);			//�洢����ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;									//���赽�洢��ģʽ
    DMA_InitStructure.DMA_BufferSize = 50;																	//���ν��յ���֡����Ŀ
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				//���������ģʽ
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;									//�洢������ģʽ
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//���跢�͵������ݳ���
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					//�洢�������������ݳ���
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;													//�洢��ѭ������ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;									//���ȼ�  ��
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;									//FIFOģʽ��ֹ
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;						//FIFOģʽ��ֵ
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;							//�洢��ͻ����������
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;			//����ͻ����������

    DMA_Init(DMA2_Stream5, &DMA_InitStructure);															//DMA1��ʼ��

    DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_FEIF5);			//�������IT��־λ
    DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_DMEIF5);
    DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TEIF5);
    DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_HTIF5);
    DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);

    DMA_Cmd(DMA2_Stream5, ENABLE);

    USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���5

}

u32 rec = 0;
u32 rec2 = 0;
u32 rec3 = 0;
u32 rec4 = 0;

u8 length_total = 18;

extVisionRecvData_t Vision_receive;

u8 vision_flag = 0;
u8 Vision_SentData[50] = {0xa5, 1, };
u8 Head_length = 3;
u32 length;

u32 Flag=0;

void USART1_IRQHandler(void)                	//����1�жϷ������
{
    u8 receive_lenth;
//	u8 i;
    u32 pot = 0;


    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //����
    {
        rec++;
        DMA_Cmd(DMA2_Stream5, DISABLE);

        receive_lenth = USART1->SR;//�ȶ�SR���ٶ�DR
        receive_lenth = USART1->DR;
        receive_lenth=receive_lenth;

        length = 50 - DMA_GetCurrDataCounter(DMA2_Stream5);
        memcpy((void*)(&Vision_2), (const void*)(Vision_Sum), length);
        Flag++;
        pot = 0;
        while(pot<length)
        {
            if(Vision_2[pot] == 0xa5)
            {
                rec2++;
                if(Verify_CRC8_Check_Sum(&Vision_2[pot], Head_length))
                {
                    rec3++;
                    if(Verify_CRC16_Check_Sum(&Vision_2[pot], length_total))
                    {


                        memcpy((void*)(&Vision_receive), (const void*)(&Vision_2[pot]), length_total);
                        Vision_receive.flag = Vision_receive.identify_target;
                        vision_update_flag = true;

                        pot+=18;
                    }
                    else
                    {
                        pot++;
                    }
                }
                else
                {
                    pot++;
                }
            }
            else
            {
                pot++;
            }

        }
        memset(Vision_Sum, 0, 50);
//		for(i = 0; i<50; i++)
//		{
//			Vision_Sum[i]=0;
//		}
        USART_ClearFlag(USART1, USART_FLAG_IDLE);

        DMA2_Stream5->NDTR = 50;
        DMA2_Stream5->M0AR = (u32)(&Vision_Sum[0]);



        DMA_ClearFlag(DMA2_Stream5, DMA_IT_TCIF5);

        DMA_Cmd(DMA2_Stream5, ENABLE);
    }
    else if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
    {
        USART_ClearFlag(USART1, USART_FLAG_TC);

    }
    else if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        USART_ClearFlag(USART1, USART_FLAG_RXNE);
        rec4++;
    }

}

void Usart1_Sent_Byte(u8 ch)
{
    USART1->DR = ch;
    while((USART1->SR&0x40)==0);
}

void Vision_Sent(u8 *buff)
{
    u8 i = 1;
    for(i = 0; i <22; i++)
    {
        Usart1_Sent_Byte(buff[i]);
    }
}


