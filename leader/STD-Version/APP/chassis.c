#include <chassis.h>

float chassis_position;

PVM_TypeDef Chassis;

int32_t mileage;//���
int32_t mileage_max;//�����



bool init_flag=false;		/*�����������ʼ��*/
bool left_PES=false;		/*��ߵĹ�翪��״̬*/
bool right_PES=false;		/*�ұߵĹ�翪��״̬*/

float rotate_ratio=8;

int8_t Derection_Flag;//1Ϊ��-1Ϊ��

void Chassis_Init()
{
    Chassis.PID_PVM.kp=8;
    Chassis.PID_PVM.ki=0.15;
    Chassis.PID_PVM.kd=0;
    Chassis.PID_PVM.IntegralLimit=8000;
    Chassis.PID_PVM.MaxOutput=12000;

    Derection_Flag=1;

}

int32_t Get_Position(void)
{
    int32_t tim_cnt=0;	/*��ȡ��ʱ���������ı���*/

    tim_cnt = (int32_t)TIM1->CNT;			/*��ȡ��ʱ��1�ļ���ֵ*/

    if(tim_cnt>1200)
        tim_cnt -= 2400;		/*��ʱ���ļ���ֵΪ2400����Ϊʹ���˶�ʱ���ı������ӿڣ������ǻ����AB����λ��ϵ�����мӼ������Ե���ֵ����1200ʱ����Ϊ��Ϊ����Ӽ���0�����*/

    TIM1->CNT = 0;		/*����ֵ��Ҫ��0*/

    return tim_cnt;	/*���ر��β������ݣ�������*/


}

/**
* @brief ��ȡ��翪�ص�״̬
* @param void
* @return void
*	��ȡ״̬
*/
void Get_Switch_Status()
{
    if(Left_PES == 1)
        left_PES=false;
    else
        left_PES=true;//��翪����������true

    if(Right_PES == 1)
        right_PES=false;
    else
        right_PES=true;
}


void Chassis_Data()
{

    mileage += Get_Position();

    Get_Switch_Status();

}

void Cruise_Normal()
{
    static uint8_t delay_dir=0;
    if(Derection_Flag==1)
    {
        if(left_PES==true)//�����˶�ʱ������翪��
        {
            delay_dir++;
            if(delay_dir>50)
            {
                delay_dir=0;
                mileage=0;//��������
            }

        }

        if(mileage<100)
        {
            Derection_Flag=-1;
        }
        else
        {
            Chassis.PID_PVM.target=Derection_Flag*1000;
        }
    }//��

    if(Derection_Flag==-1)
    {
        if(right_PES==true)//�����˶�ʱ������翪��
        {
            delay_dir++;
            if(delay_dir>50)
            {
                delay_dir=0;
                mileage_max=mileage;//��¼��������
            }

        }

        if(mileage>(mileage_max-100))
        {
            Derection_Flag=1;
        }
        else
        {
            Chassis.PID_PVM.target=Derection_Flag*1000;
        }
    }//��

}

float  Chassis_Q=2,Chassis_R=4;
int see;

void Chassis_task()
{
    static extKalman_t Chassis_p;
    static int8_t K_create=0;
    if(K_create==0)
    {
        KalmanCreate(&Chassis_p,Chassis_Q,Chassis_R);
        K_create=1;
    }

    Chassis_Data();


    if(Remote_Mode)
    {

        Chassis.PID_PVM.target=-(RC_Ctl.rc.ch2-1024)*rotate_ratio;
        Chassis.PID_PVM.target=KalmanFilter(&Chassis_p,Chassis.PID_PVM.target);
    }//ң�ؿ���

    if(Cruise_Mode)//Ѳ��״̬
    {
        static uint8_t step=0;	/*��ʼ������*/
        if(init_flag == false)		/*δ�������ɳ�ʼ��*/
        {
            switch(step)
            {
            case 0:			/*��ʼ����������߿�*/
            {
                static uint8_t delay0=0;
                if(left_PES==true)		/*��ߵĹ�翪��*/
                {
                    Chassis.PID_PVM.target=0;
                    delay0++;
                    if(delay0>=50)		/*��ֹ�� -- ��ʱȷ��*/
                    {
                        delay0=0;
                        step=1;
                    }
                }
                else	/*δʶ�� -- �����˶�*/
                {
                    Chassis.PID_PVM.target = 800;	/*�����˶�*/
                    delay0=0;
                }
                break;
            }
            case 1:
            {
                if(left_PES==true)
                {
                    Chassis.PID_PVM.target=-100;	/*����΢�������ڵ��պò�������翪�ص�λ��*/
                }
                else			/*΢����ɺ���ͣס��������һ�����ڽ׶�*/
                {
                    Chassis.PID_PVM.target=0;
                    mileage=0;	/*�������� -- ��ʼ��¼*/
                    step=2;
                }
                break;
            }
            case 2:
            {
                static uint8_t delay2=0;
                if(right_PES==true)	/*�Ѿ�ʶ���ҹ��*/
                {
                    Chassis.PID_PVM.target=0;					/*��ֹ�� -- ��ʱȷ��*/
                    delay2++;
                    if(delay2>=50)
                        step=3;
                }
                else if(right_PES==false)		/*δʶ��*/
                {
                    Chassis.PID_PVM.target=-800;			/*�����˶�*/
                    delay2=0;												/*��ֹ�� -- ��ʱȷ��*/
                }
                break;
            }
            case 3:
            {
                if(right_PES==true)
                {
                    Chassis.PID_PVM.target=100;		/*�Ѿ�ʶ���ҹ�翪�أ�����΢�����պ�δʶ�𵽵�״̬*/
                }
                else			/*΢�����*/
                {
                    Chassis.PID_PVM.target=0;															/*ֹͣ�˶�*/

                    mileage_max=mileage;	/*��¼�������*/

                    step=0;																							/*��λstep*/
                    init_flag=true;											/*���±�־λ*/
                }
                break;
            }
            }
        }
        if(init_flag == true)		/*�������ɳ�ʼ��*/
        {
            Cruise_Normal();

        }
    }
    pid_calculate(&Chassis.PID_PVM);


}



