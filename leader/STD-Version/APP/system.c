#include "system.h"

extern int SystemMode;
static volatile uint32_t usTicks = 0;

uint32_t currentTime = 0;
uint32_t loopTime_1ms=0;
uint32_t previousTime = 0;
uint16_t cycleTime = 0;

short accx,accy,accz;
short gyrox,gyroy,gyroz;	//������ԭʼ����
float pitch,roll,yaw,yaw_10;		//ŷ����

float RampFloat(float step,float target,float current)
{
    float tmp;
    if(abs(current - target)>abs(step))
    {
        if(current < target)
            tmp = current + step;
        else
            tmp = current - step;
    }
    else
    {
        tmp = target;
    }
    return tmp;
}
/**
* @brief �������͵��������ƺ���
* @param
- float center:��������ֵ
- float width:�������
- float input:����ֵ
* @return int16_t
* ���뵽������ķ���ֵ.
*/
float myDeathZoom(float center,float width,float input)
{
    if(abs(input-center)>width)
    {
        //���������ĵľ�����ڿ��,������������
        return input;
        //�򷵻�����ֵ,��ֵ����
    }
    else
    {
        //���������ĵľ���С�ڿ��,����������
        return center;
        //�򷵻�����ֵ.
    }

}
//�޷�
float constrain(float amt, float low, float high)
{
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}
int32_t constrain_int32(int32_t amt, int32_t low, int32_t high)
{
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}

int16_t constrain_int16(int16_t amt, int16_t low, int16_t high)
{
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}

int constrain_int(int amt,int low,int high)
{
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}

//��������ʼ��
static void cycleCounterInit(void)
{
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    usTicks = clocks.SYSCLK_Frequency / 1000000;
}

//��΢��Ϊ��λ����ϵͳʱ��
uint32_t micros(void)
{
    register uint32_t ms, cycle_cnt;
    do {
        ms = sysTickUptime;
        cycle_cnt = SysTick->VAL;
    } while (ms != sysTickUptime);
    return (ms * 1000) + (usTicks * 1000 - cycle_cnt) / usTicks;
}

//΢�뼶��ʱ
void delay_us(uint32_t us)
{
    uint32_t now = micros();
    while (micros() - now < us);
}

//���뼶��ʱ
void delay_ms(uint32_t ms)
{
    while (ms--)
        delay_us(1000);
}

//�Ժ���Ϊ��λ����ϵͳʱ��
uint32_t millis(void)
{
    return sysTickUptime;
}

//ϵͳ��ʼ��
void systemInit(void)
{
    cycleCounterInit();
    SysTick_Config(SystemCoreClock / 1000);	//�δ�ʱ�����ã�1ms
}
int SystemMonitor=Normal_Mode;
void Stop()
{
//   CAN2_Send(0x200,0x0000,0x0000,0x0000,0x0000);
    CAN2_Send(0x1FF,0x0000,0x0000,0x0000,0x0000);

    Feeding_Bullet_PWM(1000);

    Red_On;
//	Blue_Off;
    Green_Off;
//	Orange_On;
    Laser_Off;
}

void LED_Flicker(void)
{   static int cnt=0;
    static int colour=1;
    if(colour==1)
    {
        Orange_On;
        Blue_Off;
    }
    if(colour==(-1))
    {
        Orange_Off;
        Blue_On;
    }
    cnt++;
    if(cnt>500)
    {
        cnt=0;
        colour=(-colour);
    }
}

u32 gyrpupdate_fps;//�������ٶȸ���ʱ��
void gyro_fps(short *g)
{
    static short  last_update,now_update;
    static u32 up_time;
    last_update = now_update ;
    now_update =*g;
    if(last_update != now_update )
    {
        gyrpupdate_fps=millis()-up_time;
        up_time=millis();
    }
}
int pass_num;
bool pass_flag=1;
void System_Init(void)
{

    cycleCounterInit();
    SysTick_Config(SystemCoreClock / 1000);//�δ�ʱ�����ã�1ms

    CAN1_Init(); 
    CAN2_Init();
    usart2_Init();
    USART5_Init();
    Led_Init();
    IO_Init();
    Vision_Init();
    Laser_Init();
    TIM1_Init();
    PWM3_Init();
    BMI_Init();

    Chassis_Init();
    Gimbal_Init();
    Launcher_Init();
    Vision_Kalman_Init();

}

//��ѭ��

void Loop(void)
{

    if(TimeFlag.ms_1>=1)
    {
        BMI_GET_DATA(&gyrox,&gyroy,&gyroz,&accx,&accy,&accz);
        BMI_Get_data(&pitch,&roll,&yaw,&gyrox,&gyroy,&gyroz,&accx,&accy,&accz);//��ȡŷ����
//		RP_SendToPc(yaw,pitch,roll,0,0,0);
        LED_Flicker();
        Vision_task();
//		gyro_fps(&gyroz);
        TimeFlag.ms_1=0;

    }

    if(TimeFlag.ms_2>=2)
    {
        if(SystemMonitor == Normal_Mode)
        {
            Launcher_task();
            if(Remote_Mode)
            {
                Chassis_task();
                Gimbal_task();
            }
            if(Cruise_Mode)
            {
                Chassis_task();
                Gimbal_task();
            }
            if(Check_Mode)
            {
//				Chassis_Data();
            }
//	    CAN1_Send(0x1FF,(int16_t)(Gimbal_yaw_PVM.PID_PVM.output),(int16_t)(Gimbal_pitch_PVM.PID_PVM.output),0x0000,0x0000);
//			CAN1_Send(0x200,(int16_t)(Chassis.PID_PVM.output),0x0000,0x0000,(int16_t)(Launcher_Dial.PID_PVM.output));
//           CAN2_Send(0x1FF,(int16_t)(-Gimbal_pitch_PVM.PID_PVM.output),0x0000,0x0000,0x0000);
//			CAN2_Send(0x1FF,(int16_t)(-Gimbal_pitch_PVM.PID_PVM.output),(int16_t)(Gimbal_yaw_PVM.PID_PVM.output),(int16_t)(Launcher_Dial.PID_PVM.output),0x0000);
 //       CAN2_Send(0x1FF,0x0000,(int16_t)(Gimbal_yaw_PVM.PID_PVM.output),0x0000,0x0000);
        }
        TimeFlag.ms_2=0;
    }
}








