#ifndef __CHASSIS_TASK_H
#define __CHASSIS_TASK_H

/* Includes ------------------------------------------------------------------*/
#include "rp_config.h"
#include "system_task.h"
/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct Chassis {
	pid_ctrl_t	 PVM;
	float        Speed_taget;
	int32_t		 Mileage_atrip;//�������
	uint8_t		 init_flag;
	int8_t       Derection_flag;//1Ϊ��-1Ϊ��
	uint8_t      rotate_ratio;//ң��������
	bool         swerve_judge;//�����Ƿ���ɵ��ж�
    bool         swerve_flag;//�������̱�־λ
} Chassis_t;
/* Exported functions --------------------------------------------------------*/
extern Chassis_t Chassis_process;
void   Chassis_Init(void);
void   StartChassisTask(void const * argument);
#endif
