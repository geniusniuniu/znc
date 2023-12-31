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
#include "src/MyCode/PID.h"
#include "LQ_MPU6050_DMP.h"

#pragma section all "cpu1_dsram"

int core1_main ()
{

    IfxCpu_enableInterrupts();
    IfxScuWdt_disableCpuWatchdog (IfxScuWdt_getCpuWatchdogPassword ());
    while(!IfxCpu_acquireMutex(&mutexCpu0InitIsOk));
//#################################################################################################
//#################################################################################################
//#################################################################################################
//
//////char txt2[64];
//    TFTSPI_Init(0);        //LCD初始化  0:横屏  1：竖屏
//    TFTSPI_CLS(u16BLUE);   //蓝色屏幕
//
//    CCU6_InitConfig(CCU61, CCU6_Channel1, 2000);//定时器中断2ms
//    CCU6_EnableInterrupt(CCU61,CCU6_Channel1);//中断使能
//
//    CAMERA_Init(60);

    while(1)//主循环
    {
//        My_Camera();
//        sprintf(txt2, "%.2f",image_Error);
//        TFTSPI_P8X16Str(1, 7, txt2, u16WHITE, u16BLACK);
//        sprintf(txt2, "%d",median);
//        TFTSPI_P8X16Str(1, 6, txt2, u16WHITE, u16BLACK);
//        sprintf(txt2, "%.2f",PID_Struct.Pid_Turn_out);
//        TFTSPI_P8X16Str(5, 6, txt2, u16WHITE, u16BLACK);
    }
    return 0;
}


void CCU61_CH1_IRQHandler (void)
{
    /* 开启CPU中断  否则中断不可嵌套 */
    IfxCpu_enableInterrupts();

    // 清除中断标志
    IfxCcu6_clearInterruptStatusFlag(&MODULE_CCU61, IfxCcu6_InterruptSource_t13PeriodMatch);

    Image();
}


#pragma section all restore
