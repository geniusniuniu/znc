#include "LQ_GPIO.h"
#include "LQ_GTM.h"
#include "MyPWM.h"
#include "My_Timer.h"
#include "LQ_MPU6050_DMP.h"
#include "PID.h"

char Stop_Flag;

void Motor_Init (void)
{
    ATOM_PWM_InitConfig(MOTOR2_N, 10000, MOTOR_FREQUENCY);//
    ATOM_PWM_InitConfig(MOTOR1_P, 0, MOTOR_FREQUENCY);//    行进电机驱动
    ATOM_PWM_InitConfig(MOTOR4_P, 10000, MOTOR_FREQUENCY);//左 动量轮驱动 接受PWM
    ATOM_PWM_InitConfig(MOTOR3_P, 10000, MOTOR_FREQUENCY);//右 动量轮驱动


    PIN_InitConfig(P32_4, PIN_MODE_OUTPUT, 0);  //行进电机方向
    PIN_InitConfig(P21_3, PIN_MODE_OUTPUT, 0);   // MOTO3N  方向 0 : 逆时针 1:顺时针
    PIN_InitConfig(P21_5, PIN_MODE_OUTPUT, 0);  // MOTOR4N 方向 1 :顺时针转 0:逆时针
}

/*
 * 控制电机PWM占空比
 *
 * MOTORx_P : 电机PWM引脚,x=1,2,3,4
 *
 * Duty     : 占空比
 *
 */
void Motor_SetDuty(IfxGtm_Atom_ToutMap MOTORx_P,int Duty)
{
    ATOM_PWM_SetDuty(MOTORx_P, Duty, MOTOR_FREQUENCY);
}

/*
 * 设置电机转动方向
 * pin : 引脚可以是  P32_4 MT1N  行进电机
 *                P21_5 MT4N
 *                P21_3 MT3N
 * Direction : 方向: 0 / 1
 */
void Motor_SetDir(GPIO_Name_t pin,uint8 Direction)
{
    PIN_Write(pin,Direction);
}


/*
 *动量轮停止
 */
void Motor_Stop(void)
{

    if(Pitch > Danger_Angle || Pitch < -Danger_Angle)
        ATOM_PWM_SetDuty(IfxGtm_ATOM0_4_TOUT50_P22_3_OUT,0,10000);// 刹车线给高，停止
    else
        ATOM_PWM_SetDuty(IfxGtm_ATOM0_4_TOUT50_P22_3_OUT,10000,10000);
}

