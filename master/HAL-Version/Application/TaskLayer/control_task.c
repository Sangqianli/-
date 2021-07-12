/**
 * @file        control_task.c
 * @author      RobotPilots@2020
 * @Version     V1.0
 * @date        9-November-2020
 * @brief       Control Center
 */

/* Includes ------------------------------------------------------------------*/
#include "control_task.h"
#include "driver.h"
#include "device.h"
#include "cmsis_os.h"

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int16_t StopData_0x200[4] = {0,0,0,0};
int16_t StopData_0x2FF[4] = {0,0,0,0};
int16_t StopPwm[2] = {0,0};
/* Exported variables --------------------------------------------------------*/
int16_t NormalData_0x200[4]= {0,0,0,0};
int16_t NormalData_0x2FF[4]= {0,0,0,0};
int16_t NormalPwm[2] = {0,0};
/* Private functions ---------------------------------------------------------*/
int16_t Mode_Data;
static void Control_Leader()
{
    NormalData_0x2FF[2] = judge_sensor.info->PowerHeatData.shooter_id2_17mm_cooling_heat;//����̨����ʱĬ��2��ǹ��
    if( judge_sensor.info->ShootData.shooter_id == 2)
        NormalData_0x2FF[3] = judge_sensor.info->ShootData.bullet_speed;  //����������
//	else
//		NormalData_0x2FF[3] = 0;
    if(sys.state == SYS_STATE_NORMAL)
    {
        Mode_Data = RP_SET_BIT(Mode_Data,1); //ң������
        if(sys.remote_mode == RC)
        {
            Mode_Data = RP_SET_BIT(Mode_Data,2);
            Mode_Data =RP_SET_BIT(Mode_Data,3);  //ң��ģʽ 11  3
        } else if(sys.remote_mode == AUTO)
        {
            Mode_Data =RP_SET_BIT(Mode_Data,2);
            Mode_Data =RP_CLEAR_BIT(Mode_Data,3);//�Զ�ģʽ  01  1
        } else if(sys.remote_mode == INSPECTION)
        {
            Mode_Data =RP_CLEAR_BIT(Mode_Data,2);
            Mode_Data =RP_SET_BIT(Mode_Data,3);//�Լ�ģʽ    10  2
        }


        if(judge_sensor.info->GameRobotStatus.robot_id == 7)//��ɫid
        {
            Mode_Data =RP_SET_BIT(Mode_Data,4);
            Mode_Data =RP_CLEAR_BIT(Mode_Data,5);//ʶ����ɫ��1��01
        }
        else if(judge_sensor.info->GameRobotStatus.robot_id == 107)//��ɫid
        {
            Mode_Data =RP_CLEAR_BIT(Mode_Data,4);
            Mode_Data =RP_SET_BIT(Mode_Data,5);//ʶ���ɫ��2��10
        } else
        {
            Mode_Data =RP_CLEAR_BIT(Mode_Data,4);
            Mode_Data =RP_CLEAR_BIT(Mode_Data,5);//û���ݻ���� ��0��00
        }

        if(sys.remote_mode != RC)
        {
            if(sys.fire_state.FRICTION_OPEN)
            {
                Mode_Data =RP_SET_BIT(Mode_Data,6);//Ħ���ֿ�����־λ��һ��
            }
            else
            {
                Mode_Data =RP_CLEAR_BIT(Mode_Data,6);
            }
            Mode_Data =RP_CLEAR_BIT(Mode_Data,7);	//����λ��գ�����λֻ����ң�ؿ�������̨��

            if( Fire_Key_info() )
            {
                Mode_Data =RP_SET_BIT(Mode_Data,8);
            }
            else
            {
                Mode_Data =RP_CLEAR_BIT(Mode_Data,8);
            }
        }
    }
    else
    {
        Mode_Data = 0;//ʧ��ʱ���
    }

    NormalData_0x200[2] = Mode_Data;
}

static void Control_Stop()
{
    CAN2_SendDataBuff(0x200,StopData_0x200);
    CAN2_SendDataBuff(0x2ff,StopData_0x2FF);

    CAN1_SendDataBuff(0x200,StopData_0x200);
    CAN1_SendDataBuff(0x2ff,StopData_0x2FF);

    FRICTION_PwmOut(0, 0);
    LED_RED_ON();
    LED_GREEN_OFF();
    LASER_OFF();

    Mode_Data = 0;
}

static void Control_Normal()
{
//    static int16_t cnt = 0;
//    if(cnt > 2)
//    {
//        CAN2_SendDataBuff(0x2ff,NormalData_0x2FF);
//        CAN2_SendDataBuff(0x2ff,NormalData_0x2FF);
//        CAN2_SendDataBuff(0x200,NormalData_0x200);
//        cnt = 0;
//    } else if(cnt <= 2)
//    {
//        CAN2_SendDataBuff(0x2ff,NormalData_0x2FF);
//        CAN2_SendDataBuff(0x2ff,NormalData_0x2FF);
//        CAN2_SendDataBuff(0x2ff,NormalData_0x2FF);
//    }
//    cnt ++;
    CAN2_SendDataBuff(0x2ff,NormalData_0x2FF);
    CAN2_SendDataBuff(0x200,NormalData_0x200);

    CAN1_SendDataBuff(0x2ff,NormalData_0x2FF);
    CAN1_SendDataBuff(0x200,NormalData_0x200);
    FRICTION_PwmOut(NormalPwm[0], NormalPwm[1]);
    LED_RED_OFF();
    LED_GREEN_ON();
    LASER_ON();
}
/* Exported functions --------------------------------------------------------*/
/**
 *	@brief	��������
 */
void StartControlTask(void const * argument)
{
    for(;;)
    {
        if(sys.state == SYS_STATE_NORMAL) {
            Control_Leader();
            Control_Normal();
        } else {
            Control_Stop();
        }
        osDelay(2);
    }
}
