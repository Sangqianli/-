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
#include "rc_sensor.h"

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
//flag_t flag = {
//	.gimbal = {
//		.reset_start = false,
//		.reset_ok = false,
//	},
//};

system_t sys = {
	.remote_mode = RC,
	.state = SYS_STATE_RCLOST,
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
/**
 *	@brief	ͨ��ң��������ϵͳ��Ϣ(������״̬������ң����Ϣ)
 */
static void rc_update_info(void)
{
	if(sys.state != SYS_STATE_NORMAL) {
			
	}
	else {
		if( (rc_sensor.info->s2 == RC_SW_MID)||(rc_sensor.info->s2 == RC_SW_DOWN) )
		{
			sys.remote_mode = RC;
		}
		else if(rc_sensor.info->s2 == RC_SW_UP)
		{
			sys.remote_mode = AUTO;
		}
	}
}

/**
 *	@brief	����ң�����л����Ʒ�ʽ
 */
static void system_ctrl_mode_switch(void)
{
	if( (rc_sensor.info->s2_switch_uptomid)||(rc_sensor.info->s2_siwtch_up) )
	{
		sys.switch_state.REMOTE_SWITCH = true;
	}
		
	if(rc_sensor.info->s1_siwtch_up)
	{
		if(sys.fire_state.FRICTION_OPEN)
			sys.fire_state.FRICTION_OPEN = false;
		else 
			sys.fire_state.FRICTION_OPEN = true;	
		rc_sensor.info->s1_siwtch_up = false;
	}
	if(rc_sensor.info->s2_siwtch_down)
	{
		if(sys.fire_state.FIRE_OPEN)
			sys.fire_state.FIRE_OPEN = false;
		else
			sys.fire_state.FIRE_OPEN = true;
		rc_sensor.info->s2_siwtch_down = false;
	}	
}


static void system_state_machine(void)
{
	if(sys.switch_state.ALL_READY)//ϵͳ�����Ҹ�λ��ɺ������л�
	{
	    system_ctrl_mode_switch();
	}
}

/* Exported functions --------------------------------------------------------*/
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
