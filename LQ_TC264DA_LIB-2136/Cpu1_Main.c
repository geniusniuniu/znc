#include <IfxCpu.h>
#include <IfxScuWdt.h>
#include <Platform_Types.h>
#include "LQ_CCU6.h"


#include "src/Mycode/My_Camera.h"
#include "src/APP/LQ_TFT18.h"
#include "src/MyCode/MyMPU6050.h"
#include <stdio.h>
#include "src/Main/Main.h"
#include "LQ_CAMERA.h"

#pragma section all "cpu1_dsram"

int core1_main ()
{
    char txt2[64];
    IfxCpu_enableInterrupts();
    IfxScuWdt_disableCpuWatchdog (IfxScuWdt_getCpuWatchdogPassword ());
    while(!IfxCpu_acquireMutex(&mutexCpu0InitIsOk));
//#################################################################################################
//#################################################################################################
//#################################################################################################
//
//
//    TFTSPI_Init(0);        //LCD��ʼ��  0:����  1������
//    TFTSPI_CLS(u16BLUE);   //��ɫ��Ļ
//
//    CCU6_InitConfig(CCU61, CCU6_Channel1, 2000);//��ʱ���ж�2ms
//    CCU6_EnableInterrupt(CCU61,CCU6_Channel1);//�ж�ʹ��
//
//    CAMERA_Init(60);

    while(1)//��ѭ��
    {
//        My_Camera();
//        sprintf(txt2, "%.2f",median);
//        TFTSPI_P8X16Str(1, 7, txt2, u16WHITE, u16BLACK);
    }
    return 0;
}


void CCU61_CH1_IRQHandler (void)
{
    /* ����CPU�ж�  �����жϲ���Ƕ�� */
    IfxCpu_enableInterrupts();

    // ����жϱ�־
    IfxCcu6_clearInterruptStatusFlag(&MODULE_CCU61, IfxCcu6_InterruptSource_t13PeriodMatch);

    Image();
}


#pragma section all restore