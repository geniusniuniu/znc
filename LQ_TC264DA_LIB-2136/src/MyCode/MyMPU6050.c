#include "LQ_SOFTI2C.h"
#include "LQ_IIC_Gyro.h"
#include "LQ_STM.h"
#include "LQ_MPU6050_DMP.h"
#include "LQ_TFT18.h"
#include "stdio.h"
#include "My_Timer.h"
#include "MyEncoder.h"
#include "PID.h"

short gyro0_last;
short gyro1_last;
short gyro2_last;

void MPU_Init(void)
{
    unsigned char res;
    IIC_Init();//ģ��IIC��ʼ��
    delayms(100);
    IIC_ReadMultByteFromSlave(0x68,WHO_AM_I,1,&res);
    LQ_DMP_Init();
    delayms(100);
    MPU6050_Init();//��ʼ��MPU6050
}

void Show_MPUVal(void)
{
   char txt[32];

   sprintf(txt, "Pitch: %.2f", Pitch);
   TFTSPI_P8X16Str(1, 3, txt, u16WHITE, u16BLACK);       //�ַ�����ʾ
   sprintf(txt, "Roll: %.2f", Roll);
   TFTSPI_P8X16Str(1, 4, txt, u16WHITE, u16BLACK);       //�ַ�����ʾ
   sprintf(txt, "Yaw: %.2f", Yaw);//500
   TFTSPI_P8X16Str(1, 5, txt, u16WHITE, u16BLACK);       //�ַ�����ʾ

}

void Stability_Judge(void)
{
    short Stability_Value = 0;

    if(My_Abs(gyro[2]-gyro2_last)>300)
        Stability_Value++;
    else if(My_Abs(gyro[0]-gyro0_last)>200)
        Stability_Value++;
    else if(My_Abs(gyro[1]-gyro1_last)>350)
        Stability_Value++;

    if(Stability_Value != 0)
    {
        Stability_Flag = 1;
        Stability_Value = 0;
    }
    else
        Stability_Flag = 0;

    gyro0_last = gyro[0];
    gyro1_last = gyro[1];
    gyro2_last = gyro[2];
}
