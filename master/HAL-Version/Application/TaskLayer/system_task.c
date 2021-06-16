/**
 * @file        system_task.c
 * @author      RobotPilots@2020
 * @Version     V1.0
 * @date        27-October-2020
 * @brief       Decision Center.
 */

/* Includes ------------------------------------------------------------------*/
#include "system_task.h"

#include "cmsis_os.h"
#include "device.h"

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

system_t sys = {
    .remote_mode = RC,
    .state = SYS_STATE_RCLOST,
    .gimbal_now = MASTER, //Ĭ��ң��ģʽ�¿��Ƶ�������̨
    .auto_mode = AUTO_MODE_SCOUT,
    .switch_state.SYS_RESET = false,
    .switch_state.REMOTE_SWITCH = false,
    .switch_state.AUTO_MODE_SWITCH = false,
    .switch_state.ALL_READY	= false,
    .fire_state.FRICTION_OPEN = false,
    .fire_state.FIRE_OPEN = false,
    .predict_state.PREDICT_OPEN = false,
    .predict_state.PREDICT_ACTION = false,
};

/* Private functions ---------------------------------------------------------*/
static void Data_clear()
{
    sys.fire_state.FRICTION_OPEN = false;
    sys.fire_state.FIRE_OPEN = false;
    sys.predict_state.PREDICT_OPEN = false;
    sys.auto_mode = AUTO_MODE_SCOUT;
	sys.gimbal_now = MASTER;
	
	Mode_Data = 0;

    Chassis_process.init_flag = false;
    Chassis_process.Mode = CHASSIS_NORMAL;
    Chassis_process.Safe = CHASSIS_SAFE;
	Chassis_process.Fire = FIRE_ALL;
	Chassis_process.Spot_taget = 0;
	
	Gimbal_process.Scout_direction = 1;

    pid_clear(&Gimbal_process.YAW_PPM);
    pid_clear(&Gimbal_process.YAW_PVM);
    pid_clear(&Gimbal_process.PITCH_PPM);
    pid_clear(&Gimbal_process.PITCH_PVM);
    pid2_clear(&Gimbal_process.PITCH2_PVM);
	
	pid_clear(&Chassis_process.PPM);
	pid_clear(&Chassis_process.PVM);	
	
    motor[GIMBAL_YAW].info->angle_sum = 0;
    motor[GIMBAL_PITCH].info->angle_sum = 0;

    Gimbal_process.Yaw_taget = 0;
    Gimbal_process.Pitch_taget = 0;

    Chassis_process.Trip_times = 0;
}
/**
 *	@brief	ͨ��ң��������ϵͳ��Ϣ(������״̬������ң����Ϣ)
 */
static void rc_update_info(void)
{
    if(sys.state != SYS_STATE_NORMAL) {

    }
    else {
        if( rc_sensor.info->s2 == RC_SW_MID )
        {
            sys.remote_mode = RC;
        }
        else if(rc_sensor.info->s2 == RC_SW_UP)
        {
            sys.remote_mode = AUTO;
        } else if(rc_sensor.info->s2 == RC_SW_DOWN)
        {
            sys.remote_mode = INSPECTION;
        }
    }
}

/**
 *	@brief	����ң�����л����Ʒ�ʽ
 */
static void system_ctrl_mode_switch(void)
{
    static uint16_t tum_cnt = 0;
    if( (rc_sensor.info->s2_switch_uptomid)||(rc_sensor.info->s2_siwtch_up)||(rc_sensor.info->s2_switch_downtomid)||(rc_sensor.info->s2_siwtch_down) )
    {
        sys.switch_state.REMOTE_SWITCH = true;
        sys.switch_state.RESET_CAL = true;
        sys.switch_state.ALL_READY = false;
        rc_sensor.info->s2_switch_uptomid = false;
        rc_sensor.info->s2_siwtch_up = false;
        rc_sensor.info->s2_switch_downtomid = false;
        rc_sensor.info->s2_siwtch_down = false;

        Data_clear();
    }
    if( abs(rc_sensor.info->thumbwheel) >=630 )
    {
        tum_cnt++;
        if(tum_cnt > 100)
        {
            if(rc_sensor.info->thumbwheel >= 630)
            {
                sys.gimbal_now = MASTER;
            } else if(rc_sensor.info->thumbwheel <= -630)
            {
                sys.gimbal_now = LEADER;
            }
            tum_cnt = 0;
        }
    }
}


static void system_state_machine(void)
{
    if( (sys.switch_state.REMOTE_SWITCH == false)&&(sys.switch_state.SYS_RESET == false) )
        sys.switch_state.ALL_READY = true;
    if(sys.switch_state.ALL_READY)//ϵͳ�����Ҹ�λ��ɺ������л�
    {
        system_ctrl_mode_switch();
    }
}

/* Exported functions --------------------------------------------------------*/
void Application_Init()  //������ʼ��
{
    Chassis_Init();
    Gimbal_Init();
    Fire_Init();
    Vision_Init();
}
/**
 *	@brief	ϵͳ��������
 */
void StartSystemTask(void const * argument)
{
    for(;;)
    {
        portENTER_CRITICAL();

        // ����ң����Ϣ
        rc_update_info();

        /* ң������ */
        if(rc_sensor.work_state == DEV_OFFLINE)
        {
            sys.state = SYS_STATE_RCLOST;
            RC_ResetData(&rc_sensor);
            Data_clear();//���������Ϣ
        }
        /* ң������ */
        else if(rc_sensor.work_state == DEV_ONLINE)
        {
            /* ң������ */
            if(rc_sensor.errno == NONE_ERR)
            {
                /* ʧ���ָ� */
                if(sys.state == SYS_STATE_RCLOST)
                {
                    // ���ڴ˴�ͬ����̨��λ��־λ
                    // ϵͳ������λ
                    sys.switch_state.SYS_RESET = true;//ʧ����λ��־λ
                    sys.switch_state.RESET_CAL = true;
                    sys.switch_state.ALL_READY = false;//δ��λ��
                    sys.remote_mode = RC;
//					sys.state = SYS_STATE_NORMAL;
                }
                sys.state = SYS_STATE_NORMAL;
                // ���ڴ˴��ȴ���̨��λ��������л�״̬
                system_state_machine();
            }
            /* ң�ش��� */
            else if(rc_sensor.errno == DEV_DATA_ERR) {
                sys.state = SYS_STATE_RCERR;
                //reset CPU
                __set_FAULTMASK(1);
                NVIC_SystemReset();
            } else {
                sys.state = SYS_STATE_WRONG;
                //reset CPU
                __set_FAULTMASK(1);
                NVIC_SystemReset();
            }
        }

        portEXIT_CRITICAL();

        osDelay(2);
    }
}
