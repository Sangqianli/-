#ifndef RM__VISION__H
#define RM__VISION__H
#include "system.h"


//��ʼ�ֽڣ�Э��̶�β0xA5
#define 	VISION_SOF					(0xA5)

//���ȸ���Э�鶨��
#define		VISION_LEN_HEADER		3			//֡ͷ��
#define 	VISION_LEN_DATA 		18    //���ݶγ��ȣ����Զ���
#define     VISION_LEN_TAIL			2			//ƵβCRC16
#define		VISION_LEN_PACKED		23		//���ݰ����ȣ����Զ���

#define    VISION_OFF           (0x00)
#define    VISION_RED_ATTACK    (0x01)
#define    VISION_BLUE_ATTACK   (0x02)
#define    VISION_RED_BASE      (0x03)
#define    VISION_BLUE_BASE     (0x04)
#define    VISION_RED_GETBOMB   (0x05)
#define    VISION_BLUE_GETBOMB  (0x06)


//�������뷢��ָ������бȽϣ����շ�һ��ʱ���ݿ���

//֡ͷ��CRC8У��,��֤���͵�ָ������ȷ��

//PC�շ���STM32�շ��ɾ����ϵ,���½ṹ��������STM32,PC�������޸�


typedef __packed struct
{
	//ͷ
	uint8_t		SOF;			//֡ͷ��ʼλ,�ݶ�0xA5
	uint8_t 	CmdID;		//ָ��
	uint8_t 	CRC8;			//֡ͷCRCУ��,��֤���͵�ָ������ȷ��
}extVisionSendHeader_t;


/*****************�Ӿ����հ���ʽ********************/
typedef __packed struct
{
	/*ͷ*/
	uint8_t 	SOF;			//֡ͷ��ʼλ,0xA5
	uint8_t 	CmdID;		//ָ��
	uint8_t 	CRC8;			//֡ͷCRCУ��,��֤���͵�ָ������ȷ��
	/*����*/
	float pitch_angle;
	float yaw_angle;
	float distance;
	uint8_t identify_target;// �Ƿ�ʶ��Ŀ��	��λ��0/1
	uint8_t anti_gyro;	// �Ƿ�ʶ��С����	��λ��0/1	
	uint8_t buff_change_armor_four;	// �Ƿ��л������Ŀ�װ�װ�		��λ��0/1
	uint8_t identify_buff;	// �Ƿ�ʶ��Buff	��λ��0/1
	uint8_t identify_too_close;	// Ŀ��������	��λ��0/1
	uint8_t	anti_gyro_change_armor;	// �Ƿ��ڷ�����״̬���л�װ�װ�	��λ��0/1
	/*β*/
	uint16_t CRC16;
	u8 flag;
}extVisionRecvData_t;
/*****************�Ӿ����Ͱ���ʽ********************/
typedef __packed struct
{
	
	/* ���� */  
	uint8_t    sentry_mode;   		//�����ڱ�ģʽ
	uint8_t		 base_far_mode;     //Զ�̵������ģʽ
	uint8_t    base_near_mode;    //���̵������ģʽ(Ҳ������ǰ��վ)
	uint8_t    fric_speed; 				//�ӵ�����(ֱ�ӷּ���)
	/* β */
	uint16_t  CRC16;
}extVisionSendData_t;


//�������д��CRCУ��ֵ
//���ǿ���ֱ�����ùٷ�����CRC����

//ע��,CRC8��CRC16��ռ�ֽڲ�һ��,8Ϊһ���ֽ�,16Ϊ2���ֽ�

//д��    CRC8 ����    Append_CRC8_Check_Sum( param1, param2)
//���� param1����д����֡ͷ���ݵ�����(֡ͷ������ݻ�ûдû�й�ϵ),
//     param2����CRC8д������ݳ���,���Ƕ������ͷ�����һλ,Ҳ����3

//д��    CRC16 ����   Append_CRC16_Check_Sum( param3, param4)
//���� param3����д����   ֡ͷ + ����  ������(��������ͬһ������)
//     param4����CRC16д������ݳ���,���Ƕ�����������ݳ�����22,������22

/*----------------------------------------------------------*/



extern extVisionRecvData_t Vision_receive;

void Vision_Read_Data(uint8_t *ReadFormUsart);
void Vision_Send_Data(uint8_t CmdID);

void Usart1_Sent_Byte(u8 ch);
void Vision_Sent(u8 *buff);
void Vision_Init(void);

/******************���ָ���С����*******************/
void Vision_Ctrl(void);
void Vision_Auto_Attack_Off(void);
void Vision_Auto_Attack_Ctrl(void);

void Vision_Error_Yaw(float *error);
void Vision_Error_Pitch(float *error);
void Vision_Error_Angle_Yaw(float *error);
void Vision_Error_Angle_Pitch(float *error);
void Vision_Get_Distance(float *distance);

bool Vision_IF_Updata(void);
void Vision_Clean_Updata_Flag(void);
int16_t Get_Attack_attention_Mode(void);



#endif


