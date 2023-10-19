#include <stdio.h>
#include "PID.h"
#include "LQ_MotorServo.h"
#include "LQ_GPIO.h"
#include "MyPWM.h"
#include "My_Timer.h"
#include "LQ_MPU6050_DMP.h"

PID_Structure PID_Struct;

float Pitch_Angle_Mid = -0.5;
float Roll_Angle_Mid = 4.4;
float image_Error = 0;
float Exp_MidLine = 50;

float Pid_Out;
float PID_Out_F;
char Flag_Status = 0;
float Target_Turn_Speed;


void PID_Init(PID_Structure *pid)
{

    //ֱ����
    pid->Kp_omegar = 0;
    pid->Kd_omegar = 0;
    pid->Pid_omegar_out = 0;
    pid->omegar_expect_value = 0;//����ֵ
    pid->omegar_now_value = 0;//��ǰֵ


    // �ǶȻ�
    pid->Kp_Angle = 0;
    pid->Kd_Angle = 0;
    pid->Ki_Angle = 0;
    pid->Pid_Angle_out = 0;
    pid->Angle_expect_value = 0;//����ֵ
    pid->Angle_now_value = 0;//��ǰֵ


    //�ٶȻ�
    pid->Kp_Speed = 0;
    pid->Ki_Speed = 0;
    pid->Pid_Speed_out = 0;
    pid->Speed_expect_value = 0;//����ֵ
    pid->Speed_now_value = 0;//��ǰֵ

    pid->Kp_Balance = 0;
    pid->Kd_Balance = 0;
    pid->Ki_Balance = 0;
    pid->Pid_Balance_out = 0;
    pid->Balance_expect_value = 0;//����ֵ
    pid->Balance_now_value = 0;//��ǰֵ

    pid->Kp_Front_Speed = 0;
    pid->Ki_Front_Speed = 0;
    pid->Pid_Front_Speed_out = 0;
    pid->Front_expect_value = 0;//����ֵ
    pid->Front_now_value = 0;//��ǰֵ

    pid->Turn_Kp = 0;
    pid->Turn_Ki = 0;
    pid->Turn_Kd = 0;
    pid->Pid_Turn_out = 0;

}

/*
 *
 * �޷�����
 * *Output  Ҫ�޷������ĵ�ַ��
 *  Max     ���ֵ��
 *  Min     ��Сֵ
 */
void Limit_Out(float *Output,float Max,float Min)
{
    if (*Output <= Min) *Output = Min;
    else if (*Output >= Max) *Output = Max;
}


//����ֵ����
float My_Abs(float a)
{
    if(a<=0) return -a;
    return a;
}


//############################����ƽ�⻷#######################################//

void omegar_PD(PID_Structure* pid,float gx)  // ���ٶȻ�
{
    static float error;
    error = pid->omegar_expect_value - gx;
    pid->Pid_omegar_out = pid->Kp_omegar*error + pid->Kd_omegar*gx;
    Limit_Out(&pid->Pid_omegar_out, 9000, -9000);
}

//�ǶȻ�
void Angle_PID(PID_Structure* pid,float Angle,float Gyro_x)
{
    static float Error_Integral;
    float Error;
    Error = Angle - pid->Angle_expect_value;
    Error_Integral += Error;
    Limit_Out(&Error_Integral,100,-100); //���ϵͳ���ڽϴ�ĸ��Ż��Ŷ����������ý�С�Ļ����޷����Լ�С����������ã���ֹϵͳ�ĳ����Ͳ��ȶ���
    //if(Stop_Flag == 1) {Error = 0;Error_Integral = 0;}
    pid->Pid_Angle_out = pid->Kp_Angle*Error+pid->Ki_Angle*Error_Integral+Gyro_x*pid->Kd_Angle/10;
    Limit_Out(&pid->Pid_Angle_out,9000,-9000); //Pid_Angle_out �޷�
}

// �ٶȻ�
void Speed_PI(PID_Structure* pid,float Enc_Left,float Enc_Right)
{
    static float Encoder,Encoder_last,Encoder_Err;
    static float Encoder_Integral;
    float a = 0.7;
    //�����ٶȣ����ұ�����֮�ͣ�- Ŀ���ٶȣ��˴�Ϊ�㣩
    Encoder_Err = (float)(Enc_Left - Enc_Right);
    Encoder = (1 - a) * Encoder_Err + a * Encoder_last; // ʹ�ò��θ���ƽ�����˳���Ƶ���ţ���ֹ�ٶ�ͻ��
    Encoder_last = Encoder;   // ��ֹ�ٶȹ���Ӱ��ֱ��������������
    Encoder_Integral += Encoder;
    Limit_Out(&Encoder_Integral,1000,-1000);
    //if(Stop_Flag == 1) {Encoder = 0;Encoder_Integral = 0;}
    pid->Pid_Speed_out = Encoder*pid->Kp_Speed + Encoder_Integral*pid->Ki_Speed;    //===�ٶȿ���
}




//#########################ǰ��ƽ�⻷#######################################//

void Front_Balance_PID(PID_Structure* pid,float Angle,float Gyro)
{
     float Error;
     static float Error_Integral=0;

     Error = Angle - pid->Balance_expect_value;       //===���ƽ��ĽǶ���ֵ �ͻ�е���
     Error_Integral += Error;
     Limit_Out(&Error_Integral, 30, -30);
     //if(Stop_Flag == 1) {Error = 0;Error_Integral = 0;}
     pid->Pid_Balance_out = pid->Kp_Balance*Error +
                                 pid->Ki_Balance*Error_Integral +
                                     Gyro* pid->Kd_Balance/10;   //��ȡ������ֵ
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
                                    Encoder_Integral *  pid->Ki_Front_Speed;   //��ȡ������ֵ
}



// ################        ת��      ##################
void Turn_P(PID_Structure* pid, uint8_t Act_mid_line,float Gyro)
{
    static float Error_Integral,Error_Last;
    image_Error = (float)Act_mid_line - Exp_MidLine; //ʵ��-����
    image_Error = 0.7 * image_Error + Error_Last * 0.3;                                              //һ�׵�ͨ�˲���
    Error_Integral += image_Error ;
    Error_Last = image_Error;
    Limit_Out(&Error_Integral, 40, -40);
    Limit_Out(&Gyro, 500, -500);
//    if(Flag_Status == 0)            {Target_Turn_Speed = 0 ;pid->Turn_Kd = 0;}
//    else if(Flag_Status == 2)        Target_Turn_Speed--;
//    else if(Flag_Status == 4)        Target_Turn_Speed++;
//    if(Flag_Status != 0)             pid->Turn_Kd = 0;
//    Limit_Out(&Target_Turn_Speed,20,-20);
    //Kd���ת��Լ����Kp��Կ���ת��,ǰ����Լ�������������ͷ���������Ϣ����
    pid->Pid_Turn_out = pid->Turn_Kp * image_Error + pid->Turn_Ki * Error_Integral+pid->Turn_Kd * Gyro;
    Limit_Out(&pid->Pid_Turn_out, 1500, -1500);
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
    Limit_Out(motorC,3000,200);
    if(Pitch > Safe_Angle || Pitch < -Safe_Angle)
        *motorC = 0;
    Motor_SetDuty(MOTOR1_P,*motorC);
    return *motorC;
}



