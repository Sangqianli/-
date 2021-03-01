/**
 * @file        monitor_task.c
 * @author      RobotPilots@2020
 * @Version     V1.0
 * @date        9-November-2020
 * @brief       Monitor&Test Center
 */

/* Includes ------------------------------------------------------------------*/
#include "chassis_task.h"

#include "device.h"
#include "rp_math.h"
#include "cmsis_os.h"

/* Private macro -------------------------------------------------------------*/
#define Normal_Speed -2000
#define First_Speed  -1000
#define Swerve_Ratio 0.1
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
Chassis_t Chassis_process;
/* Private functions ---------------------------------------------------------*/
static void Cruise_First()
{
    static uint8_t step=0;	/*��ʼ������*/
    if(Chassis_process.init_flag == false)		/*δ�������ɳ�ʼ��*/
    {
        switch(step)
        {
        case 0:			/*��ʼ����������߿�*/
        {
            if(path_sensor.info->left_touch)		/*��ߵĵ㴥����*/
            {
                Chassis_process.Speed_taget=0;
                step=1;
            }
            else	/*δʶ�� -- �����˶�*/
            {
                Chassis_process.Speed_taget = -1000;	/*�����˶�*/
            }
            break;
        }
        case 1:
        {
            if(path_sensor.info->left_touch)
            {
                Chassis_process.Speed_taget=100;	/*����΢�������ڵ��պò������㴥���ص�λ��*/
            }
            else			/*΢����ɺ���ͣס��������һ�����ڽ׶�*/
            {
                Chassis_process.Speed_taget=0;
                path_sensor.info->mileage_total=0;	/*�������� -- ��ʼ��¼*/
                step=2;
            }
            break;
        }
        case 2:
        {
            if(path_sensor.info->right_touch)	/*�Ѿ�ʶ���ҹ��*/
            {
                Chassis_process.Speed_taget=0;
                step=3;
            }
            else 		/*δʶ��*/
            {
                Chassis_process.Speed_taget=1000;			/*�����˶�*/
            }
            break;
        }
        case 3:
        {
            if(path_sensor.info->right_touch)
            {
                Chassis_process.Speed_taget=-100;		/*�Ѿ�ʶ���ҵ㴥���أ�����΢�����պ�δʶ�𵽵�״̬*/
            }
            else			/*΢�����*/
            {
                Chassis_process.Speed_taget=0;															/*ֹͣ�˶�*/
                Chassis_process.Mileage_atrip=path_sensor.info->mileage_total;	/*��¼�������*/
                step=0;																							/*��λstep*/
                Chassis_process.init_flag=true;											/*���±�־λ*/
                Chassis_process.Derection_flag=1;//�����˶���־λ
            }
            break;
        }
        }
    }
}
static void Cruise_Normal()
{
    if(Chassis_process.Derection_flag == 1)
    {
        if(path_sensor.info->left_touch)//�����˶�ʱ�����㴥����
        {
            Chassis_process.swerve_judge = true;//��ʼ�����ж�
        }

        if(Chassis_process.swerve_judge)
        {
            if(path_sensor.info->left_touch == false)//�������㴥�����ͷ�
            {
                Chassis_process.Derection_flag = -1;//�����ܹ�
                path_sensor.info->mileage_total = 0;//��������
                Chassis_process.swerve_judge = false;
                Chassis_process.swerve_flag = false;//�������
            }
        }
    }//��

    if(Chassis_process.Derection_flag == -1)
    {
        if(path_sensor.info->right_touch)//�����˶�ʱ�����㴥����
        {
            Chassis_process.swerve_judge = true;//��ʼ�����ж�
        }
        if(Chassis_process.swerve_judge)
        {
            if(path_sensor.info->right_touch == false)//�������㴥�����ͷ�
            {
                Chassis_process.Derection_flag = 1;//�����ܹ�
                Chassis_process.Mileage_atrip = path_sensor.info->mileage_total;//��¼��������
                Chassis_process.swerve_judge = false;
                Chassis_process.swerve_flag = false;//�������
            }
        }
    }//��
}

/*�����������շ�Χ�Ĵ���*/
static void Chassis_Rebound()
{
    if( ( (Chassis_process.Mileage_atrip-path_sensor.info->mileage_now)<=(Chassis_process.Mileage_atrip*Swerve_Ratio) )  ||
            ( (path_sensor.info->mileage_now<(Chassis_process.Mileage_atrip*Swerve_Ratio)) ) )
    {
        Chassis_process.swerve_flag = true;//�������̿�ʼ
    }
    if(Chassis_process.swerve_flag)
        Chassis_process.PVM.out = 0;
    else//û�з���������Ѳ��
    {
        Chassis_process.Speed_taget=Chassis_process.Derection_flag*Normal_Speed;
    }
}

static void Chassis_RCcontrol()
{
    Chassis_process.Speed_taget = rc_sensor.info->ch2*Chassis_process.rotate_ratio;
    Chassis_process.PVM.target = Chassis_process.Speed_taget;
    Chassis_process.PVM.measure = motor[CHASSIS].info->speed;
    pid_calculate(&Chassis_process.PVM);
    NormalData_0x200[0] = Chassis_process.PVM.out;
}

static void Chassis_AUTOcontrol()
{
    Chassis_process.PVM.target = Chassis_process.Speed_taget;
    Chassis_process.PVM.measure = motor[CHASSIS].info->speed;
    pid_calculate(&Chassis_process.PVM);
    if(Chassis_process.swerve_flag)
        Chassis_process.PVM.out = 0;
    NormalData_0x200[0] = (int16_t)Chassis_process.PVM.out;
}
/* Exported functions --------------------------------------------------------*/
void Chassis_Init()
{
    Chassis_process.PVM.kp = 18;
    Chassis_process.PVM.ki = 0.11;
    Chassis_process.PVM.kd = 0;
    Chassis_process.PVM.integral_max = 8000;
    Chassis_process.PVM.out_max = 12000;
    Chassis_process.init_flag = false;
    Chassis_process.Derection_flag = 1;//��ʼ����
    Chassis_process.rotate_ratio = 8;
}
void StartChassisTask(void const * argument)
{
    for(;;)
    {
        if(sys.state == SYS_STATE_NORMAL)
        {
            if(sys.remote_mode == RC)
            {
                Chassis_RCcontrol();
            }
            else if(sys.remote_mode == AUTO)
            {
                Cruise_First();
                if(Chassis_process.init_flag)
                {
                    Cruise_Normal();
                    Chassis_Rebound();
                }
                Chassis_AUTOcontrol();
            }
        }
        osDelay(2);
    }
}


