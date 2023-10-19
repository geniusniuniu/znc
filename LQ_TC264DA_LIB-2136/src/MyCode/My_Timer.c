#include "My_Timer.h"
#include "PID.h"
#include "LQ_GPT12_ENC.h"
#include "LQ_MPU6050_DMP.h"
#include "MyPWM.h"
#include "LQ_CCU6.h"
#include "MyEncoder.h"
#include "LQ_MotorServo.h"
#include "My_camera.h"

int Timer_Count = 0;
float Pid_Out_L;
float Pid_Out_R;
float Pid_Out_F;

int Timer_Count0;
int Timer_Count1;
int Timer_Count2;


void MyTimer_Init(void)
{
    CCU6_InitConfig(CCU60, CCU6_Channel1, 2000);//��ʱ���ж�2ms
    CCU6_EnableInterrupt(CCU60,CCU6_Channel1);//�ж�ʹ��
}

void CCU60_CH1_IRQHandler (void)
{
    /* ����CPU�ж�  �����жϲ���Ƕ�� */
    IfxCpu_enableInterrupts();
    // ����жϱ�־
    IfxCcu6_clearInterruptStatusFlag(&MODULE_CCU60, IfxCcu6_InterruptSource_t13PeriodMatch);
    /* �û����� */
    //BLDC_Motor_Hall_Run(BLDCduty);//�����и���ˢ�����뵽����
    LQ_DMP_Read();
    Get_EncVal();
    Timer_Count0++;
    Timer_Count1++;
    Timer_Count2++;

    // �������Ƕȶ����ֺ��н��������ֹͣ
    Motor_Stop();

///// ##########        ����ƽ�⻷       ##################
/////      #####  �ٶȻ� -> �ǶȻ� -> ���ٶ� #########

    PID_Struct.omegar_expect_value = PID_Struct.Pid_Angle_out;
    omegar_PD(&PID_Struct,gyro[0]);
    if(Timer_Count0 == 5)
    {
        Timer_Count0 = 0;
        PID_Struct.Angle_expect_value = PID_Struct.Pid_Speed_out + Pitch_Angle_Mid;
        Angle_PID(&PID_Struct,Pitch,gyro[0]);
        //Pitch_Angle_Mid = -0.85;
    }
    if(Timer_Count1 == 50)
    {
        Timer_Count1 = 0;
        Speed_PI(&PID_Struct,EncVal_L,EncVal_R);
        Turn_P(&PID_Struct,median,gyro[2]);
    }
    // ##########        ǰ��ƽ�⻷       ##################
    //       ########  �ٶȻ� -> �ǶȻ�  #########
    if(Timer_Count2 == 10)
    {
        Timer_Count2 = 0;
        PID_Struct.Balance_expect_value = PID_Struct.Pid_Front_Speed_out + Roll_Angle_Mid;
        Front_Balance_PID(&PID_Struct,Roll,gyro[1]);
        Front_Speed_PI(&PID_Struct,EncVal_F);
    }

    Pid_Out_L = PID_Struct.Pid_omegar_out + PID_Struct.Pid_Turn_out;
    Pid_Out_R = PID_Struct.Pid_omegar_out - PID_Struct.Pid_Turn_out;
    Pid_Out_F = PID_Struct.Pid_Balance_out;

    Motor_Ctrl(Pid_Out_L,Pid_Out_R);
    Front_Motor_Ctrl(&Pid_Out_F);

}









