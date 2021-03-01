/**
 * @file        can_potocol.c
 * @author      RobotPilots@2020
 * @Version     V1.0
 * @date        9-September-2020
 * @brief       CAN Potocol.
 */
 
/* Includes ------------------------------------------------------------------*/
#include "can_potocol.h"

#include "drv_can.h"
#include "chassis_motor.h"

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
 *	@brief	��CAN�����ж�ȡ�����λ�÷���
 */
static uint16_t CAN_GetMotorAngle(uint8_t *rxData)
{
	uint16_t angle;
	angle = ((uint16_t)rxData[0] << 8| rxData[1]);
	return angle;
}

/**
 *	@brief	��CAN�����ж�ȡ�����ת��ת�ٷ���
 */
static int16_t CAN_GetMotorSpeed(uint8_t *rxData)
{
	int16_t speed;
	speed = ((uint16_t)rxData[2] << 8| rxData[3]);
	return speed;
}

/**
 *	@brief	��CAN�����ж�ȡ�����ʵ��ת�ص�������
 */
static int16_t CAN_GetMotorCurrent(uint8_t *rxData)
{
	int16_t current;
	current = ((int16_t)rxData[4] << 8 | rxData[5]);
	return current;
}

///**
// *	@brief	��CAN�����ж�ȡ�����ʵ�����ת��
// */
//static int16_t CAN_GetMotorTorque(uint8_t *rxData)
//{
//	int16_t torque;
//	torque = ((int16_t)rxData[4] << 8 | rxData[5]);
//	return torque;
//}

/**
 *	@brief	RM3508 CAN��ʶ��
 */
static uint32_t RM3508_GetStdId(drv_can_t *drv)
{
	if((drv->can_id - 0x201U) < 4)
		return 0x200;
	else
		return 0x1FF;
}

/**
 *	@brief	RM3508 CAN�����±�
 */
static uint8_t RM3508_GetDrvId(drv_can_t *drv)
{
	return (drv->can_id - 0x201U)%4;
}

///**
// *	@brief	GM6020 CAN��ʶ��
// */
//static uint32_t GM6020_GetStdId(drv_can_t *drv)
//{
//	if((drv->can_id - 0x205U) < 4)
//		return 0x1FF;
//	else
//		return 0x2FF;
//}

///**
// *	@brief	GM6020 CAN�����±�
// */
//static uint8_t GM6020_GetDrvId(drv_can_t *drv)
//{
//	return (drv->can_id - 0x205U)%4;
//}

///**
// *	@brief	RM2006 CAN��ʶ��
// */
//static uint32_t RM2006_GetStdId(drv_can_t *drv)
//{
//	if((drv->can_id - 0x201U) < 4)
//		return 0x200;
//	else
//		return 0x1FF;
//}

///**
// *	@brief	RM2006 CAN�����±�
// */
//static uint8_t RM2006_GetDrvId(drv_can_t *drv)
//{
//	return (drv->can_id - 0x201U)%4;
//}

/* Exported functions --------------------------------------------------------*/
void chassis_motor_update(chassis_motor_t *chas_motor, uint8_t *rxBuf)
{
	chassis_motor_info_t *motor_info = chas_motor->info;
	
	motor_info->angle = CAN_GetMotorAngle(rxBuf);
	motor_info->speed = CAN_GetMotorSpeed(rxBuf);
	motor_info->current = CAN_GetMotorCurrent(rxBuf);
	
	motor_info->offline_cnt = 0;//
}

void chassis_motor_init(chassis_motor_t *motor)
{
	drv_can_t *drv_can = motor->driver;
	
	motor->info->offline_cnt = motor->info->offline_max_cnt+1;
	motor->work_state = DEV_OFFLINE;
	
	motor->errno = NONE_ERR;
	if(motor->id == DEV_ID_CHASSIS_LF) {
		drv_can->drv_id = RM3508_GetDrvId(drv_can);
		drv_can->std_id = RM3508_GetStdId(drv_can);
	}
	else if(motor->id == DEV_ID_CHASSIS_RF) {
		drv_can->drv_id = RM3508_GetDrvId(drv_can);
		drv_can->std_id = RM3508_GetStdId(drv_can);
	}
	else if(motor->id == DEV_ID_CHASSIS_LB) {
		drv_can->drv_id = RM3508_GetDrvId(drv_can);
		drv_can->std_id = RM3508_GetStdId(drv_can);	
	}
	else if(motor->id == DEV_ID_CHASSIS_RB) {
		drv_can->drv_id = RM3508_GetDrvId(drv_can);
		drv_can->std_id = RM3508_GetStdId(drv_can);		
	}
	else {
		motor->errno = DEV_ID_ERR;
	}
}

/**
 *	@brief	CAN ���͵�������
 */
void CAN_SendSingleData(drv_can_t *drv, int16_t txData)
{
	int16_t txArr[4] = {0, 0, 0, 0};
	
	txArr[drv->drv_id] = txData;
	if(drv->type == DRV_CAN1)
		CAN1_SendData(drv->std_id, txArr);
	else if(drv->type == DRV_CAN2)
		CAN2_SendData(drv->std_id, txArr);
}

/**
 *	@brief	CAN �������ݻ���
 */
void CAN_SendDataBuff(drv_type_t drv_type, uint32_t std_id, int16_t *txBuff)
{
	if(drv_type == DRV_CAN1)
		CAN1_SendData(std_id, txBuff);
	else if(drv_type == DRV_CAN2)
		CAN2_SendData(std_id, txBuff);
}

/**
 *	@brief	CAN1 ��������
 */
void CAN1_rxDataHandler(uint32_t canId, uint8_t *rxBuf)
{
	/* ��ǰ�� */
	if(canId == CHASSIS_CAN_ID_LF)
	{
		// ���µ��̵������
		chassis_motor[CHAS_LF].update(&chassis_motor[CHAS_LF], rxBuf);
		chassis_motor[CHAS_LF].check(&chassis_motor[CHAS_LF]);
	}
	/* ��ǰ�� */
	else if(canId == CHASSIS_CAN_ID_RF)
	{
		// ���µ��̵������
		chassis_motor[CHAS_RF].update(&chassis_motor[CHAS_RF], rxBuf);
		chassis_motor[CHAS_RF].check(&chassis_motor[CHAS_RF]);
	}
	/* ����� */
	else if(canId == CHASSIS_CAN_ID_LB)
	{
		// ���µ��̵������
		chassis_motor[CHAS_LB].update(&chassis_motor[CHAS_LB], rxBuf);
		chassis_motor[CHAS_LB].check(&chassis_motor[CHAS_LB]);
	}
	/* �Һ��� */
	else if(canId == CHASSIS_CAN_ID_RB)
	{
		// ���µ��̵������
		chassis_motor[CHAS_RB].update(&chassis_motor[CHAS_RB], rxBuf);
		chassis_motor[CHAS_RB].check(&chassis_motor[CHAS_RB]);
	}
}

/**
 *	@brief	CAN2 ��������
 */
void CAN2_rxDataHandler(uint32_t canId, uint8_t *rxBuf)
{
}

