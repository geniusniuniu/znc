#include <stdio.h>
#include "PID.h"
#include "LQ_MotorServo.h"
#include "LQ_GPIO.h"
#include "MyPWM.h"
#include "My_Timer.h"
#include "LQ_MPU6050_DMP.h"
#include "MyEncoder.h"
#include <math.h>

PID_Structure PID_Struct;

float Dynamic_zero_Pitch;
float Dynamic_zero_Roll;

float Pid_Out;
float PID_Out_F;
char Flag_Status = 0;
float Target_Turn_Speed;


void PID_Init(PID_Structure *pid)
{

    //直立环
    pid->Kp_omegar = 0;
    pid->Kd_omegar = 0;
    pid->Pid_omegar_out = 0;
    pid->omegar_expect_value = 0;//期望值
    pid->omegar_now_value = 0;//当前值


    // 角度环
    pid->Kp_Angle = 0;
    pid->Kd_Angle = 0;
    pid->Ki_Angle = 0;
    pid->Pid_Angle_out = 0;
    pid->Angle_expect_value = 0;//期望值
    pid->Angle_now_value = 0;//当前值


    //速度环
    pid->Kp_Speed = 0;
    pid->Ki_Speed = 0;
    pid->Pid_Speed_out = 0;
    pid->Speed_expect_value = 0;//期望值
    pid->Speed_now_value = 0;//当前值

    pid->Kp_Balance = 0;
    pid->Kd_Balance = 0;
    pid->Ki_Balance = 0;
    pid->Pid_Balance_out = 0;
    pid->Balance_expect_value = 0;//期望值
    pid->Balance_now_value = 0;//当前值

    pid->Kp_Front_Speed = 0;
    pid->Ki_Front_Speed = 0;
    pid->Pid_Front_Speed_out = 0;
    pid->Front_expect_value = 0;//期望值
    pid->Front_now_value = 0;//当前值

    pid->Turn_Kp = 0;
    pid->Turn_Ki = 0;
    pid->Turn_Kd = 0;
    pid->Pid_Turn_out = 0;

}

/*
 *
 * 限幅函数
 * *Output  要限幅的数的地址，
 *  Max     最大值，
 *  Min     最小值
 */
void Limit_Out(float *Output,float Max,float Min)
{
    if (*Output <= Min) *Output = Min;
    else if (*Output >= Max) *Output = Max;
}


//绝对值函数
float My_Abs(float a)
{
    if(a<=0) return -a;
    return a;
}


//############################左右平衡环#######################################//

void omegar_PD(PID_Structure* pid,float gx)  // 角速度环
{
    static float error;
    error = pid->omegar_expect_value - gx;
    pid->Pid_omegar_out = pid->Kp_omegar*error + pid->Kd_omegar*gx;
    Limit_Out(&pid->Pid_omegar_out, 9000, -9000);
}

//角度环
void Angle_PID(PID_Structure* pid,float Angle,float Gyro_x)
{
    static float Error_Integral;
    float Error;
//    if(pid->Front_expect_value != 0)
//    {
//        /////dongtailingdian动态零点，//转固定半径的圆
//        Dynamic_zero_Pitch = atan((0.000001 * EncVal_F * EncVal_F)*0.6)*180/3.14159;
//        //Limit_Out(&Dynamic_zero_Pitch,2.5,-2.5);
//        pid->Angle_expect_value -= Dynamic_zero_Pitch;
//    }
    Error = Angle - pid->Angle_expect_value;
    Error_Integral += Error;
    Limit_Out(&Error_Integral,100,-100); //如果系统存在较大的干扰或扰动，可以设置较小的积分限幅，以减小积分项的作用，防止系统的超调和不稳定。
    //if(Stop_Flag == 1) {Error = 0;Error_Integral = 0;}
    pid->Pid_Angle_out = pid->Kp_Angle*Error+pid->Ki_Angle*Error_Integral+Gyro_x*pid->Kd_Angle/10;
    Limit_Out(&pid->Pid_Angle_out,9000,-9000); //Pid_Angle_out 限幅
}

// 速度环
void Speed_PI(PID_Structure* pid,float Enc_Left,float Enc_Right)
{
    static float Encoder,Encoder_last,Encoder_Err;
    static float Encoder_Integral;
    float a = 0.7;
    //测量速度（左右编码器之和）- 目标速度（此处为零）
    Encoder_Err = (float)(Enc_Left - Enc_Right);
    Encoder = (1 - a) * Encoder_Err + a * Encoder_last; // 使得波形更加平滑，滤除高频干扰，防止速度突变
    Encoder_last = Encoder;   // 防止速度过大影响直立环的正常工作
    Encoder_Integral += Encoder;
    Limit_Out(&Encoder_Integral,1000,-1000);
    //if(Stop_Flag == 1) {Encoder = 0;Encoder_Integral = 0;}
    pid->Pid_Speed_out = Encoder*pid->Kp_Speed + Encoder_Integral*pid->Ki_Speed;    //===速度控制
}




//#########################前后平衡环#######################################//

void Front_Balance_PID(PID_Structure* pid,float Angle,float Gyro)
{
     float Error;
     static float Error_Integral=0;

     if(pid->Front_expect_value != 0)
     {
         /////dongtailingdian动态零点
         Dynamic_zero_Roll = atan((0.000001 * EncVal_F * EncVal_F)*0.0125)*180/3.14159;
         Limit_Out(&Dynamic_zero_Roll,5,-5);
         pid->Balance_expect_value -= Dynamic_zero_Roll;
     }
     Error = Angle - pid->Balance_expect_value;       //===求出平衡的角度中值 和机械相关
     Error_Integral += Error;
     Limit_Out(&Error_Integral,30, -30);
     //if(Stop_Flag == 1) {Error = 0;Error_Integral = 0;}
     pid->Pid_Balance_out = pid->Kp_Balance*Error +
                                 pid->Ki_Balance*Error_Integral +
                                     Gyro* pid->Kd_Balance/10;   //获取最终数值
}


void Front_Speed_PI(PID_Structure* pid,int Enc_Front)
{
    static float Encoder,Encoder_Integral;
    static float Encoder_Last;
    Encoder = (float)Enc_Front - pid->Front_expect_value;
    Encoder = 0.7 * Encoder + Encoder_Last * 0.3;
    Encoder_Integral += Encoder;
    Encoder_Last = Encoder;

    Limit_Out(&Encoder_Integral, 1000, -1000);
    //if(Stop_Flag == 1) {Encoder = 0; Encoder_Integral = 0;}
    pid->Pid_Front_Speed_out = Encoder * pid->Kp_Front_Speed +
                                    Encoder_Integral *  pid->Ki_Front_Speed;   //获取最终数值
}



// ################        转向环      ##################
void Turn_P(PID_Structure* pid, float Angle,float Gyro)
{
    static float Error_Integral;
    float Error;
    Error = Angle - Exp_Angle; //实际-期望
    Error_Integral += Error ;
    Limit_Out(&Error_Integral, 30,-30);
    Limit_Out(&Gyro, 1000, -1000);

    pid->Pid_Turn_out = pid->Turn_Kp * Error + pid->Turn_Ki * Error_Integral+pid->Turn_Kd * Gyro;
    Limit_Out(&pid->Pid_Turn_out, 3000, -3000);
}



float Motor_Ctrl(float motorA, float motorB)
{
    float temp;
    if (motorA > 0)         Motor_SetDir(P21_3,1); //MT3N
    else                    Motor_SetDir(P21_3,0); //MT3N
    Pid_Out = 10000.0-My_Abs(motorA);
    Motor_SetDuty(MOTOR3_P,Pid_Out);

    if (motorB > 0)         Motor_SetDir(P21_5,0); //MT4N
    else                    Motor_SetDir(P21_5,1); //MT4N
    temp =  10000.0-My_Abs(motorB);
    Motor_SetDuty(MOTOR4_P,temp);

    return Pid_Out;
}


float Front_Motor_Ctrl (float *motorC)
{
    if (*motorC > 0)         Motor_SetDir(P32_4,1); //MT1N
    else                    Motor_SetDir(P32_4,0); //MT1N

    *motorC = My_Abs(*motorC);
    Limit_Out(motorC,3100,260);
    if(Pitch > Safe_Angle || Pitch < -Safe_Angle)
        *motorC = 0;
    Motor_SetDuty(MOTOR1_P,*motorC);
    return *motorC;
}




