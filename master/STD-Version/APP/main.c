#include "system.h"

uint32_t Remote_time = 0;

int main(void)
{
    System_Init();
    Remote_time = sysTickUptime;
    while(1)
    {
        if((sysTickUptime >= Remote_time))//���ң����һ��ʱ����û���յ���Ϣ��ǿ�н�ң������λΪû��ң�ص�״̬
        {
            RC_Ctl.rc.ch0 = 1024;
            RC_Ctl.rc.ch1 = 1024;
            RC_Ctl.rc.ch2 = 1024;
            RC_Ctl.rc.ch3 = 1024;
            RC_Ctl.rc.s1 = 3;
            RC_Ctl.rc.s2 = 3;
            Stop();
            SystemMonitor = Error_Mode;
            RobotMonitor  =prepare_Mode;

            Friction_Open=0;
			Dail_Open=0;
            delay_ms(2);
        }
        else
        {
            SystemMonitor = Normal_Mode;
            Green_On;
            Red_Off;
            Laser_On;
        }
//		Robot_Monitor();
        Loop();
    }
}


