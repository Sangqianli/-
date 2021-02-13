#include "pid.h"
int pid_calculate(PID_TypeDef* pid)
{
    pid->last_err  = pid->err;

    pid->last_output = pid->output;

    pid->err = pid->target - pid->measure;


    pid->pout = pid->kp * pid->err;
    pid->iout += (pid->ki * pid->err);
    pid->dout =  pid->kd * (pid->err - pid->last_err);

    //�����Ƿ񳬳�����
    if(pid->iout > pid->IntegralLimit)
        pid->iout = pid->IntegralLimit;
    if(pid->iout < - pid->IntegralLimit)
        pid->iout = - pid->IntegralLimit;//λ��ʽPID;

    //pid�����
    pid->output = pid->pout + pid->iout + pid->dout;
//		pid->output = pid->output*(1-smooth) + pid->last_output*smooth;  //�˲�

    pid->last_output=pid->output;

    if(pid->output>pid->MaxOutput)
    {
        pid->output = pid->MaxOutput;
    }
    if(pid->output < -(pid->MaxOutput))
    {
        pid->output = -(pid->MaxOutput);
    }
    return pid->output;
}

void 	pid_clear(PID_TypeDef* pid)
{
    pid->iout=0;
    pid->err=0;
    pid->pout=0;
    pid->output=0;

}
