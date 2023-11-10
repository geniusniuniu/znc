#ifndef SRC_MYCODE_PID_H_
#define SRC_MYCODE_PID_H_

#include <stdio.h>
#include "stdint.h"

#define Pitch_Angle_Mid  -1.1
#define Roll_Angle_Mid  3.0

extern float Dynamic_zero_Roll ;
extern float Dynamic_zero_Pitch ;
extern float K;

extern char Flag_Stop;
extern char Flag_Status;
extern float Target_Turn_Speed;
extern float Pid_Out;
extern float Pid_Out_F;

typedef struct PID
{
    //���ٶȻ� (�����㵹)
    float Kp_omegar;
    float Kd_omegar;
    float Pid_omegar_out;
    float omegar_expect_value;//����ֵ
    float omegar_now_value;//��ǰֵ

    // �ǶȻ�
    float Kp_Angle;
    float Ki_Angle;
    float Kd_Angle;
    float Pid_Angle_out;
    float Angle_expect_value;//����ֵ
    float Angle_now_value;//��ǰֵ

    // �ٶȻ�
    float Kp_Speed;
    float Ki_Speed;
    float Pid_Speed_out;
    float Speed_expect_value;//����ֵ
    float Speed_now_value;//��ǰֵ

    // ǰ��ƽ��ǶȻ�
    float Kp_Balance;
    float Ki_Balance;
    float Kd_Balance;
    float Pid_Balance_out;
    float Balance_expect_value;//����ֵ
    float Balance_now_value;//��ǰֵ

    // �ٶȻ�
    float Kp_Front_Speed;
    float Ki_Front_Speed;
    float Pid_Front_Speed_out;
    float Front_expect_value;//����ֵ
    float Front_now_value;//��ǰֵ

    //ת��
    float Turn_Angle_Kp;
    float Turn_Angle_Ki;
    float Turn_Angle_Kd;
    float Pid_Turn_Angle_out;
    float Turn_Exp_Angle;


}PID_Structure;


extern PID_Structure PID_Struct;

void PID_Init(PID_Structure *pid);
void Limit_Out(float *Output,float Max,float Min);
float Motor_Ctrl(float motorA, float motorB);
float Front_Motor_Ctrl (float *motorC);
float My_Abs(float a);

void Param_Change(void);
//zuoyou
void omegar_PD(PID_Structure* pid,float gx);
void Angle_PID(PID_Structure* pid,float Angle,float Gyro_x);
void Speed_PI(PID_Structure* pid,float Enc_Left,float Enc_Right);


//qianhou
void Front_Balance_PID(PID_Structure* pid,float Angle,float Gyro);
void Front_Speed_PI(PID_Structure* pid,int Enc_Front);


//zhuanxiang
void Turn_Angle_PID(PID_Structure* pid,float Angle,short gyro);

#endif /* SRC_MYCODE_PID_H_ */



