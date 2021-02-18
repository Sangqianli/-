#include "visual_process.h"
/*Ŀ���ٶȵĶ�������*/
QueueObj target_speed=
{
    .nowLength =0,
    .queueLength = 100,
    .queue = {0},
    .queueTotal=0,
    .full_flag=0
};
/*Ŀ����ٶȵĶ�������*/
QueueObj target_accel=
{
    .nowLength =0,
    .queueLength = 100,
    .queue = {0},
    .queueTotal = 0,
    .full_flag=0
};
/*Ŀ��������ƵĶ�������*/
QueueObj target_distance_tendency=
{
    .nowLength =0,
    .queueLength = 100,
    .queue = {0},
    .queueTotal = 0,
    .full_flag=0
};


extern bool vision_update_flag;
extern extVisionRecvData_t Vision_receive;//�Ӿ��������Ľṹ��
extern u8 Vision_SentData[60];//���͸��Ӿ�������
extern CRUISE_Mode Cruise_mode;
const  uint16_t ACTIVE_MAX_CNT = 2,LOST_MAX_CNT = 10; 	/*����ʶ��Ͷ�ʧ�ж�����ֵ*/
Visual_TypeDef VisualProcess;//���������ݽ���ṹ��
float vision_yaw_raw=0,vision_pitch_raw=0,vision_dis_meter;//�Ӿ����ݴ���������
float target_yaw_raw=0,target_yaw_kf=0;	/*Ŀ���yaw��ԭʼ����,�������˲�����*/	float target_speed_raw=0,target_speed_kf=0;	/*Ŀ���yaw�ٶ�ԭʼ����,�������˲�����*/
float target_accel_raw=0,target_accel_kf=0;	 /*��Ŀ����ٶȵ�Ԥ�����*/
float target_dis_tend;//Ŀ�����任����
float predict_angle=0,k_pre=0.10f,predict_angle_raw=0,use_predic=0.9f;		/*yawԤ�ⳬǰ�Ƕ�,Ԥ�����0.1*/	float feedforward_angle=0,k_ff=1.f,use_ff=1; 	/*ǰ���� -- ���ٶ�*/

uint16_t vision_update_fps = 0; /*�Ӿ����ݸ��µ�֡��*/
extKalman_t kalman_visionYaw,kalman_targetYaw,kalman_visionPitch,kalman_targetPitch,kalman_visionDistance,kalman_targetDistance;
extKalman_t kalman_accel,kalman_speedYaw;
float visionYaw_R=1,targetYaw_R=2000,visionPitch_R=0.5,targetPitch_R=3000,visionDis_R=1,targetDis_R=1000;//0.2,2000,0.5,3000
float predictAccel_R=100,speedYaw_R=400;//400,1200
void Vision_Kalman_Init()
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


void Sent_to_Vision()
{
    static u8 Sent_cnt=0;//���ͼ��
    u8 *yw,*pw,*ys,*ps;
    static float Yaw_raw,Pitch_raw;
    static  float Yaw_speed,Pitch_speed;

    Yaw_raw=((Gimbal_yaw_PPM.PID_PPM.measure)/8191)*360;
    Pitch_raw=((Gimbal_pitch_PPM.PID_PPM.measure)/8191)*360;//����Ƕ�ת��
    Yaw_speed=(Gimbal_yaw_PVM.speed_get*2*3.1415/60);
    Pitch_speed=(Gimbal_pitch_PVM.speed_get*2*3.1415/60);


    yw=(u8*)&Yaw_raw;
    pw=(u8*)&Pitch_raw;
    ys=(u8*)&Yaw_speed;
    ps=(u8*)&Pitch_speed;

    Append_CRC8_Check_Sum(Vision_SentData, 3);
    Append_CRC16_Check_Sum(Vision_SentData,22);

    Vision_SentData[0]=0xA5;
    Vision_SentData[1]=1;
    /*С�˷��ͣ����ֽ��Ǹ�λ*/
    Vision_SentData[3]= *yw;
    Vision_SentData[4]=*(yw+1);
    Vision_SentData[5]=*(yw+2);
    Vision_SentData[6]=*(yw+3);//Yaw��Ƕ�����

    Vision_SentData[7]= *pw;
    Vision_SentData[8]=*(pw+1);
    Vision_SentData[9]=*(pw+2);
    Vision_SentData[10]=*(pw+3);//Pitch��Ƕ�����

    Vision_SentData[11]= *ys;
    Vision_SentData[12]=*(ys+1);
    Vision_SentData[13]=*(ys+2);
    Vision_SentData[14]=*(ys+3);//Yaw����ٶ�����

    Vision_SentData[15]= *ps;
    Vision_SentData[16]=*(ps+1);
    Vision_SentData[17]=*(ps+2);
    Vision_SentData[18]=*(ps+3);//Yaw����ٶ�����

    Sent_cnt++;

    if(Sent_cnt>=5)
    {
        Vision_Sent(Vision_SentData);
        Sent_cnt=0;
    }


}

uint32_t vision_this_time,vision_last_time;/*�Ӿ����ܵ�ʱ��*/
float YawTarget_now,PitchTarget_now;//ʵ���Ӿ������Ƕ�
float update_cloud_yaw = 0,update_cloud_pitch=0;	/*��¼�Ӿ���������ʱ����̨���ݣ����´ν�����*/
bool predict_flag = false;	/*Ԥ�⿪���ı�־λ*/
void Vision_task()
{

//	static uint8_t last_identify_flag=0;/*��һ�εĽ��ձ�־λ*/
    static uint16_t active_cnt=0,lost_cnt=0;/*�������/��ʧ����--����ʶ���δʶ���໥�л��Ĺ���*/

//update_target_yaw=0;	/*���ݸ���ʱ��Ŀ������	*/
    if(vision_update_flag == true)	/*�Ӿ�����������*/
    {
        if(Vision_receive.identify_target == 1)		/*�����жϵ�ǰ�Ƿ�ʶ��*/
        {
            active_cnt++; 	/*��Ծ����*/

            if(active_cnt >= ACTIVE_MAX_CNT) /*�ﵽ��ֵ���϶�Ϊʶ��*/
            {
                Cruise_mode = ATTACK;

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
                Cruise_mode = SCOUT;
                active_cnt = 0;
                lost_cnt = 0;
                /*���ģʽ���л�*/
                Clear_Queue(&target_speed);
                Clear_Queue(&target_accel);
                Clear_Queue(&target_distance_tendency);
                /*���������Ϣ����ֹ��һ�������ܵ�Ӱ��*/
//				predict_delay_cnt = 0;	 	/*����Ԥ�����*/
                predict_angle_raw=0;//��0Ԥ���
                predict_flag = false;			/*�ر�Ԥ��*/
            }
        }



//		Vision_receive.yaw_angle=myDeathZoom(0,0.2,Vision_receive.yaw_angle);
//		Vision_receive.pitch_angle=myDeathZoom(0,0.1,Vision_receive.pitch_angle);
        vision_yaw_raw = (Vision_receive.yaw_angle-3.5f) * CONVER_SCALE;	/*�任Ϊ��е�Ƕȵĳ߶�,������Ӧ24.756..24.2*/
        vision_pitch_raw=(Vision_receive.pitch_angle+6.99f)* 22.9f;//�����ǿ���������ֱ�Ӽӣ�ע�ⵥλ6.69

        vision_dis_meter=Vision_receive.distance/1000.f;

        vision_yaw_raw=myDeathZoom(0,6,vision_yaw_raw);
        vision_pitch_raw=myDeathZoom(0,3,vision_pitch_raw);//��������10,5

        VisualProcess.YawGet_KF = KalmanFilter(&kalman_visionYaw,vision_yaw_raw); 	/*���Ӿ��Ƕ��������������˲�*/
        VisualProcess.PitchGet_KF = KalmanFilter(&kalman_visionPitch,vision_pitch_raw);
        if(vision_dis_meter>0.0001f)
            VisualProcess.DistanceGet_KF =KalmanFilter(&kalman_visionDistance,vision_dis_meter);
//		VisualProcess.DistanceGet_KF=vision_dis_meter;
        YawTarget_now=update_cloud_yaw+VisualProcess.YawGet_KF;
        PitchTarget_now=update_cloud_pitch+VisualProcess.PitchGet_KF;

        update_cloud_yaw=Gimbal_yaw_PPM.PID_PPM.measure;/*�Ӿ����ݸ���ʱ����̨�Ƕ�*/
        update_cloud_pitch=Gimbal_pitch_PPM.PID_PPM.measure;

        /*�Ӿ���������.......................................*/
//		target_speed_raw = Get_Target_Speed(10,YawTarget_now);

        target_speed_raw = Get_Diff(20,&target_speed,YawTarget_now);//�°�

        target_speed_raw = KalmanFilter(&kalman_speedYaw,target_speed_raw);
        target_speed_raw=myDeathZoom(0,0.3,target_speed_raw);
        /*�����Ӿ����µ��������������ٶ�-���ڸ������ڲ��ȶ����ʲ�����ɢ��ʽ����*/
//		target_accel_raw = Get_Target_Accel(5,target_speed_raw);	 /*��ȡ���ٶ�*/

        target_accel_raw = Get_Diff(10,&target_accel,target_speed_raw);	 /*�°��ȡ���ٶ�*/
        target_accel_raw = myDeathZoom(0,1,target_accel_raw);		/*�������� - �˳�0�㸽��������*/
        target_accel_raw = KalmanFilter(&kalman_accel,target_accel_raw);

        target_dis_tend =  Get_Diff(10,&target_distance_tendency,	VisualProcess.DistanceGet_KF);
        target_dis_tend =  myDeathZoom(0,0.1,target_dis_tend);

        /*......................................................*/
//		last_identify_flag = Vision_receive.identify_target;//��¼ʶ���־λ
        vision_update_flag = false;		/*�����־λ*/
    }

    /*ֱ�Ӹ������.................................*/
	VisualProcess.YawTarget_KF=KalmanFilter(&kalman_targetYaw,YawTarget_now);
	VisualProcess.PitchTarget_KF=KalmanFilter(&kalman_targetPitch,PitchTarget_now);
    /*..................................................*/

    /*������Ԥ�ⷽ��Ĵ���...................................................................................*/
//    if(predict_flag==true)
//    {
//        float dir_factor;
//        if((target_speed_raw*target_accel_raw)>=0)
//        {
//            dir_factor= 1.f;//4

//        }
//        else
//        {
//            dir_factor= 1.8f;//4.5
//        }

//        feedforwaurd_angle = k_ff * target_accel_raw; 	/*����ǰ����*/

//        //	feedforward_angle = constrain(feedforward_angle,-28.f,28.f);	 /*ǰ����16*/

//        predict_angle_raw = (0.8f*target_speed_raw*VisualProcess.DistanceGet_KF+2.2f*dir_factor*feedforward_angle) ;	/*����Ԥ��Ƕ�*/

//        //	predict_angle_raw = constrain(predict_angle_raw,-60,60);	/*�޷�28*/
//        //	predict_angle = RampFloat((abs(predict.0_angle_raw - predict_angle)/200),predict_angle_raw,predict_angle); 	/*б�´���*/
//    }
//    VisualProcess.YawTarget_KF=YawTarget_now+predict_angle_raw*use_predic;
//    VisualProcess.YawTarget_KF=KalmanFilter(&kalman_targetYaw,VisualProcess.YawTarget_KF);
//    VisualProcess.PitchTarget_KF=PitchTarget_now+600*target_dis_tend;
//    VisualProcess.PitchTarget_KF=KalmanFilter(&kalman_targetPitch,VisualProcess.PitchTarget_KF);

    /*...................................................................................................*/

//	Sent_to_Vision();//���͸��Ӿ�

}

void Clear_Queue(QueueObj* queue)
{
    for(uint16_t i=0; i<queue->queueLength; i++)
    {
        queue->queue[i]=0;
    }
    queue->nowLength = 0;
    queue->queueTotal = 0;
    queue->aver_num=0;
    queue->Diff=0;
    queue->full_flag=0;
}
/**
* @brief ��ȡĿ����ٶ�
* @param void
* @return void
*	�Զ��е��߼�
*/
float Get_Target_Speed(uint8_t queue_len,float angle)
{
    float sum=0;
    float tmp=0;
	
    if(queue_len>target_speed.queueLength)
        queue_len=target_speed.queueLength;
    //��ֹ���
    if(target_speed.nowLength<queue_len)
    {
        //����δ����ֻ������
        target_speed.queue[target_speed.nowLength] = angle;
        target_speed.nowLength++;
    }
    else
    {
//		target_speed.nowLength=0;
        //����������FIFO��
        for(uint16_t i=0; i<queue_len-1; i++)
        {
            target_speed.queue[i] = target_speed.queue[i+1];
            //���¶���
        }
        target_speed.queue[queue_len-1] = angle;
    }

    //���������
    for(uint16_t j=0; j<queue_len; j++)
    {
        sum+=target_speed.queue[j];
    }
    tmp = sum/(queue_len/1.f);
    tmp = (angle - tmp);
    return tmp;
}



/**
* @brief ��ȡĿ��ļ��ٶ�
* @param void
* @return void
*	�Զ��е��߼�
*/
float Get_Target_Accel(uint8_t queue_len,float speed)
{
    float sum=0;
    float tmp=0;
	
    if(queue_len>target_accel.queueLength)
        queue_len=target_accel.queueLength;
    //��ֹ���

    if(target_accel.nowLength<queue_len)
    {
        //����δ����ֻ������
        target_accel.queue[target_accel.nowLength] = speed;
        target_accel.nowLength++;
    }
    else
    {
        //����������FIFO��
        for(uint16_t i=0; i<queue_len-1; i++)
        {
            target_accel.queue[i] = target_accel.queue[i+1];
            //���¶���

        }
        target_accel.queue[queue_len-1] = speed;
    }

    //���������


    for(uint16_t j=0; j<queue_len; j++)
    {
        sum+=target_accel.queue[j];
    }
    tmp = sum/(queue_len/1.f);

    tmp = (speed - tmp);

    return tmp;
}


/**
* @brief ��ȡĿ��ľ���仯����
* @param void
* @return void
*	�Զ��е��߼�
*/
float Get_Distance_Tendency(uint8_t queue_len,float dis)
{
    float sum=0;
    float tmp=0;

    if(queue_len>target_distance_tendency.queueLength)
        queue_len=target_distance_tendency.queueLength;
    //��ֹ���


    if(target_distance_tendency.nowLength<queue_len)
    {
        //����δ����ֻ������
        target_distance_tendency.queue[target_accel.nowLength] = dis;
        target_distance_tendency.nowLength++;
    }
    else
    {
        //����������FIFO��
        for(uint16_t i=0; i<queue_len-1; i++)
        {
            target_distance_tendency.queue[i] = target_distance_tendency.queue[i+1];
            //���¶���
        }
        target_distance_tendency.queue[queue_len-1] = dis;
    }

    //���������
    for(uint16_t j=0; j<queue_len; j++)
    {
        sum+=target_distance_tendency.queue[j];
    }
    tmp = sum/(queue_len/1.f);

    tmp = (dis - tmp);

    return tmp;
}


/**
* @brief ��ȡĿ��Ĳ��
* @param void
* @return void
*	�Զ��е��߼�
*/
float Get_Diff(uint8_t queue_len, QueueObj *Data,float add_data)
{
    if(queue_len>=Data->queueLength)
        queue_len=Data->queueLength;
    //��ֹ���
    Data->queueTotal-=Data->queue[Data->nowLength];
    Data->queueTotal+=add_data;

    Data->queue[Data->nowLength]=add_data;

    Data->aver_num=Data->queueTotal/queue_len;
    Data->nowLength++;

    if(Data->full_flag==0)//��ʼ����δ��
    {
        Data->aver_num=Data->queueTotal/Data->nowLength;
    }
    if(Data->nowLength>=queue_len)
    {
        Data->nowLength=0;
        Data->full_flag=1;
    }

    Data->Diff=add_data-Data->aver_num;
    return Data->Diff;
}
