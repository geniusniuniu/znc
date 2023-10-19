#include <IfxCpu.h>
#include <IfxScuCcu.h>
#include <IfxScuWdt.h>
#include <IfxStm.h>
#include <IfxStm_reg.h>
#include <stdio.h>


#include "LQ_UART.h"
#include "LQ_GPIO_KEY.h"
#include "src/APP/LQ_GPIO_LED.h"
#include "src/APP/LQ_TFT18.h"
#include "src/Driver/include.h"
#include "src/Driver/LQ_STM.h"
#include "src/User/LQ_MotorServo.h"
#include "src/MyCode/MyPWM.h"
#include "src/MyCode/MyEncoder.h"
#include "src/MyCode/MyMPU6050.h"
#include "../Driver/LQ_GPT12_ENC.h"
#include "src/MyCode/My_Timer.h"
#include "LQ_IIC_Gyro.h"
#include "src/MyCode/MyKalmanfilter.h"
#include "src/MyCode/PID.h"
#include "LQ_CAMERA.h"

#pragma section all "cpu0_dsram"

App_Cpu0 g_AppCpu0;                       // brief CPU 0 global data
IfxCpu_mutexLock mutexCpu0InitIsOk = 1;   // CPU0 ��ʼ����ɱ�־λ
volatile char mutexCpu0TFTIsOk=0;         // CPU1 0ռ��/1�ͷ� TFT

char k0,k1;

float PIDParam;

float P1 = 0.5;
float P2;

void CPU_Init(void);
void PIDparam_Init(void);
void Misc_Init(void);

int core0_main (void)
{
    Misc_Init();
    PIDparam_Init();
    //PIDParam = PID_Struct.Kd_omegar;
    while(1)    //��ѭ��
    {
        Show_EncVal();
        Show_MPUVal();

        k0 = KEY_Read(KEY0);
        k1 = KEY_Read(KEY1);
        if(k0==0)
        {
            PID_Struct.Ki_Angle += 0.05;
            //PIDParam = PID_Struct.Kd_omegar;
        }
        if(k1==0)
        {
            PID_Struct.Ki_Angle -= 0.05;
            //PIDParam = PID_Struct.Kd_omegar;
        }
        else
            Flag_Status = 0;
    }
    return 0;
}

void PIDparam_Init(void)
{
    PID_Struct.Kp_omegar = 50;//50;         +
    PID_Struct.Kd_omegar = 40;  //40        +
    PID_Struct.Kp_Angle = -100; //           -
    PID_Struct.Kd_Angle = -4.70; //-4.70      -
    PID_Struct.Ki_Angle = -1.825;//-1.825     -
    PID_Struct.Kp_Speed = -0.13;//           -
    PID_Struct.Ki_Speed = PID_Struct.Kp_Speed / 200;

    PID_Struct.Kp_Balance = 270;//273       +
    PID_Struct.Ki_Balance = 35;// 35        +
    PID_Struct.Kd_Balance = 17;//17.6     +
    PID_Struct.Kp_Front_Speed = -0.08;//    -
    PID_Struct.Ki_Front_Speed = PID_Struct.Kp_Front_Speed / 200;
    PID_Struct.Front_expect_value = 1.3;//;

    PID_Struct.Turn_Kp = 0;//              +;
    PID_Struct.Turn_Ki = 0;//-15;
    PID_Struct.Turn_Kd = 0;//-25;
}


void Misc_Init(void)
{
    CPU_Init();
    // ����P14.0�ܽ����,P14.1���룬������115200
    //UART_InitConfig(UART0_RX_P14_1,UART0_TX_P14_0, 115200);
    GPIO_KEY_Init();
    MPU_Init();
    PID_Init(&PID_Struct);
    Motor_Init();
    GPIO_LED_Init();
    My_EncInit();
    MyTimer_Init();
    TFTSPI_Init(0);        //LCD��ʼ�� 0������ 1:����
    TFTSPI_CLS(u16BLUE);   //��ɫ��Ļ
}

void CPU_Init(void)
{
    // �ر�CPU���ж�
    IfxCpu_disableInterrupts();

    // �رտ��Ź�����������ÿ��Ź�ι����Ҫ�ر�
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    // ��ȡ����Ƶ��
    g_AppCpu0.info.pllFreq = IfxScuCcu_getPllFrequency();
    g_AppCpu0.info.cpuFreq = IfxScuCcu_getCpuFrequency(IfxCpu_getCoreIndex());
    g_AppCpu0.info.sysFreq = IfxScuCcu_getSpbFrequency();
    g_AppCpu0.info.stmFreq = IfxStm_getFrequency(&MODULE_STM0);

    // ����CPU���ж�
    IfxCpu_enableInterrupts();

    // ֪ͨCPU1��CPU0��ʼ�����
    IfxCpu_releaseMutex(&mutexCpu0InitIsOk);
    // �м�CPU0,CPU1...������ͬʱ������Ļ��ʾ�������ͻ����ʾ
    mutexCpu0TFTIsOk=0;         // CPU1�� 0ռ��/1�ͷ� TFT
}

#pragma section all restore