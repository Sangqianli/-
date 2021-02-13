#ifndef _PID_H
#define _PID_H



#include "stdint.h"
typedef struct _PID_TypeDef
{


    float target;							//Ŀ��ֵ

    float kp;
    float ki;
    float kd;

    float   measure;					//����ֵ
    float   err;							//���
    float   last_err;      		//�ϴ����
    float   previous_err;  //���ϴ����

    float pout;
    float iout;
    float dout;

    float output;						//�������
    float last_output;			//�ϴ����

    float MaxOutput;				//����޷�
    float IntegralLimit;		//�����޷�
} PID_TypeDef;

typedef struct
{
    PID_TypeDef PID_PVM;
    int16_t speed_get;
    int16_t speed_set;

} PVM_TypeDef;

typedef struct
{
    PID_TypeDef PID_PPM;
    int16_t position_round;
    uint16_t position_get_now;
    uint16_t position_get_last;
    int16_t position_set;
    int16_t position;

    float yaw_get;
    float yaw_base;
    float yaw_target;
} PPM_TypeDef;





int pid_calculate(PID_TypeDef* pid);

void 	pid_clear(PID_TypeDef* pid);



















#endif
