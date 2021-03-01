/**
 * @file        monitor_task.c
 * @author      RobotPilots@2020
 * @Version     V1.0
 * @date        9-November-2020
 * @brief       Monitor&Test Center
 */

/* Includes ------------------------------------------------------------------*/
#include "vision_task.h"
#include "device.h"
#include "cmsis_os.h"

/* Private macro -------------------------------------------------------------*/
#define ACTIVE_MAX_CNT  2
#define LOST_MAX_CNT    10	/*����ʶ��Ͷ�ʧ�ж�����ֵ*/
#define CONVER_SCALE_YAW    21.1f//20.86
#define CONVER_SCALE_PITCH  22.9f
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
float vision_mach_yaw,vision_mach_pitch,vision_dis_meter;//�Ӿ�����ת��

extKalman_t kalman_visionYaw,kalman_targetYaw,kalman_visionPitch,kalman_targetPitch,kalman_visionDistance,kalman_targetDistance;
extKalman_t kalman_accel,kalman_speedYaw;
float visionYaw_R=1,targetYaw_R=2000,visionPitch_R=0.5,targetPitch_R=3000,visionDis_R=1,targetDis_R=1000;//0.2,2000,0.5,3000
float predictAccel_R=100,speedYaw_R=400;//400,1200

uint8_t Vision_SentData[60];//���͸��Ӿ�������

float YawTarget_now,PitchTarget_now;//ʵ���Ӿ������Ƕ�
float update_cloud_yaw = 0,update_cloud_pitch=0;	/*��¼�Ӿ���������ʱ����̨���ݣ����´ν�����*/
/* Exported variables --------------------------------------------------------*/
Vision_process_t Vision_process;
/* Private functions ---------------------------------------------------------*/
static void Sent_to_Vision_Version2_1()
{
    static uint8_t Sent_cnt=0;//���ͼ��
    static uint32_t now_time;
    uint8_t *time;
    now_time=xTaskGetTickCount();
    time=(uint8_t*)&now_time;

    Append_CRC8_Check_Sum(Vision_SentData, 3);
    Append_CRC16_Check_Sum(Vision_SentData,22);

    Vision_SentData[0]=0xA5;
    Vision_SentData[1]=1;
    /*С�˷��ͣ����ֽ��Ǹ�λ*/
    Vision_SentData[3]= *time;
    Vision_SentData[4]=*(time+1);
    Vision_SentData[5]=*(time+2);
    Vision_SentData[6]=*(time+3);//ʱ������

    Sent_cnt++;
    if(Sent_cnt>=100)
    {
        UART1_SendData(Vision_SentData,23);
        Sent_cnt=0;
    }
}

static void Vision_Normal()
{
    static uint16_t active_cnt=0,lost_cnt=0;/*�������/��ʧ����--����ʶ���δʶ���໥�л��Ĺ���*/
    if(vision_sensor.info->State.rx_data_update)//���ݸ���
    {
        if(vision_sensor.info->RxPacket.RxData.identify_target == 1)//ʶ��Ŀ��
        {
            active_cnt++; 	/*��Ծ����*/
            if(active_cnt >= ACTIVE_MAX_CNT) /*�ﵽ��ֵ���϶�Ϊʶ��*/
            {
                sys.auto_mode = AUTO_MODE_ATTACK;
                sys.switch_state.AUTO_MODE_SWITCH = true;
                active_cnt = 0;
                lost_cnt = 0;
                /*����ȷ�Ͻ������ж�*/
            }
        }
        else
        {
            lost_cnt++;
            if(lost_cnt >= LOST_MAX_CNT) /*�ﵽ��ֵ���϶�Ϊ��ʧ*/
            {
                sys.auto_mode = AUTO_MODE_SCOUT;
                sys.switch_state.AUTO_MODE_SWITCH = true;
                active_cnt = 0;
                lost_cnt = 0;
                /*���ģʽ���л�*/
                Clear_Queue(&Vision_process.speed_queue);
                Clear_Queue(&Vision_process.accel_queue);
                Clear_Queue(&Vision_process.dis_queue);
                /*���������Ϣ����ֹ��һ�������ܵ�Ӱ��*/
                Vision_process.predict_angle = 0;//��0Ԥ���
                sys.predict_state.PREDICT_OPEN = false;			/*�ر�Ԥ��*/
            }
        }

        vision_sensor.info->RxPacket.RxData.yaw_angle = DeathZoom(vision_sensor.info->RxPacket.RxData.yaw_angle,0,0.2);
        vision_sensor.info->RxPacket.RxData.pitch_angle = DeathZoom(vision_sensor.info->RxPacket.RxData.pitch_angle,0,0.1);

        vision_mach_yaw = vision_sensor.info->RxPacket.RxData.yaw_angle*CONVER_SCALE_YAW;
        vision_mach_pitch = vision_sensor.info->RxPacket.RxData.pitch_angle*CONVER_SCALE_PITCH;		//ת���ɻ�е�Ƕ�
        vision_dis_meter =  vision_sensor.info->RxPacket.RxData.distance/1000.f;

        vision_mach_yaw  = 	DeathZoom(vision_mach_yaw,0,6);
        vision_mach_pitch=  DeathZoom(vision_mach_pitch,0,3);

        Vision_process.data_kal.YawGet_KF = KalmanFilter(&kalman_visionYaw,vision_mach_yaw); 	/*���Ӿ��Ƕ��������������˲�*/
        Vision_process.data_kal.PitchGet_KF = KalmanFilter(&kalman_visionPitch,vision_mach_pitch);
        if(vision_dis_meter>0.0001f)
            Vision_process.data_kal.DistanceGet_KF =KalmanFilter(&kalman_visionDistance,vision_dis_meter);

        YawTarget_now=update_cloud_yaw+Vision_process.data_kal.YawGet_KF;
        PitchTarget_now=update_cloud_pitch+Vision_process.data_kal.PitchGet_KF;

        update_cloud_yaw=motor[GIMBAL_YAW].info->angle_sum;/*�Ӿ����ݸ���ʱ����̨�Ƕ�*/
        update_cloud_pitch=motor[GIMBAL_PITCH].info->angle_sum;

        /*�Ӿ���������.......................................*/
        Vision_process.speed_get = Get_Diff(20,&Vision_process.speed_queue,YawTarget_now);
        Vision_process.speed_get = KalmanFilter(&kalman_speedYaw,Vision_process.speed_get);
        Vision_process.speed_get = DeathZoom(Vision_process.speed_get,0,0.3);

        Vision_process.accel_get = Get_Diff(10,&Vision_process.accel_queue,Vision_process.speed_get);	 /*�°��ȡ���ٶ�*/
        Vision_process.accel_get = DeathZoom(Vision_process.accel_get,0,0.1);		/*�������� - �˳�0�㸽��������*/
        Vision_process.accel_get = KalmanFilter(&kalman_accel,Vision_process.accel_get);

        Vision_process.distend_get =  Get_Diff(10,&Vision_process.dis_queue,Vision_process.data_kal.DistanceGet_KF);
        Vision_process.distend_get =  DeathZoom(Vision_process.distend_get,0,0.1);
        /*......................................................*/
        vision_sensor.info->State.rx_data_update = false;

    }

    /*ֱ�Ӹ������.................................*/
//	    Vision_process.data_kal.YawTarget_KF=KalmanFilter(&kalman_targetYaw,YawTarget_now);
//	    Vision_process.data_kal.PitchTarget_KF=KalmanFilter(&kalman_targetPitch,PitchTarget_now);
    /*..................................................*/
}

static void Vision_Pridict()
{
    static float acc_use = 1.f;
    static float predic_use = 1.f;
    float dir_factor;
    if( (Vision_process.speed_get * Vision_process.accel_get)>=0 )
    {
        dir_factor= 1.f;
    }
    else
    {
        dir_factor= 1.8f;
    }

    Vision_process.feedforwaurd_angle = acc_use * Vision_process.accel_get; 	/*����ǰ����*/

    Vision_process.predict_angle = (0.8f*Vision_process.speed_get*Vision_process.data_kal.DistanceGet_KF+2.2f*dir_factor*Vision_process.feedforwaurd_angle) ;	/*����Ԥ��Ƕ�*/

    Vision_process.data_kal.YawTarget_KF=YawTarget_now+Vision_process.predict_angle*predic_use;
    Vision_process.data_kal.YawTarget_KF=KalmanFilter(&kalman_targetYaw,Vision_process.data_kal.YawTarget_KF);
    Vision_process.data_kal.PitchTarget_KF=PitchTarget_now+600*Vision_process.distend_get;
    Vision_process.data_kal.PitchTarget_KF=KalmanFilter(&kalman_targetPitch,Vision_process.data_kal.PitchTarget_KF);
}
/* Exported functions --------------------------------------------------------*/
void Vision_Init()
{
    KalmanCreate(&kalman_visionYaw,1,visionYaw_R);
    KalmanCreate(&kalman_targetYaw,1,targetYaw_R);
    KalmanCreate(&kalman_visionPitch,1,visionPitch_R);
    KalmanCreate(&kalman_targetPitch,1,targetPitch_R);
    KalmanCreate(&kalman_visionDistance,1,visionDis_R);
    KalmanCreate(&kalman_targetDistance,1,targetDis_R);
    KalmanCreate(&kalman_accel,1,predictAccel_R);
    KalmanCreate(&kalman_speedYaw,1,speedYaw_R);
}

void StartVisionTask(void const * argument)
{
    for(;;)
    {
        if( (sys.state == SYS_STATE_NORMAL) && (sys.switch_state.ALL_READY) )
        {
            Vision_Normal();
            if(sys.predict_state.PREDICT_OPEN)
            {
                Vision_Pridict();
            }
            Sent_to_Vision_Version2_1();
        }
        osDelay(2);
    }
}
