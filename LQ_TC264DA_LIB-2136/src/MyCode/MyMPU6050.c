#include "LQ_SOFTI2C.h"
#include "LQ_IIC_Gyro.h"
#include "LQ_STM.h"
#include "LQ_MPU6050_DMP.h"
#include "LQ_TFT18.h"
#include "stdio.h"


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
   sprintf(txt, "Yaw: %.2f", Yaw);
   TFTSPI_P8X16Str(1, 5, txt, u16WHITE, u16BLACK);       //�ַ�����ʾ

//   sprintf(txt, "Pitch: %.2f", (float)gyro[0]);
//   TFTSPI_P8X16Str(1, 3, txt, u16WHITE, u16BLACK);       //�ַ�����ʾ
//   sprintf(txt, "Roll: %.2f", (float)gyro[1]);
//   TFTSPI_P8X16Str(1, 4, txt, u16WHITE, u16BLACK);       //�ַ�����ʾ
//   sprintf(txt, "Yaw: %.2f", (float)gyro[2]);
//   TFTSPI_P8X16Str(1, 5, txt, u16WHITE, u16BLACK);       //�ַ�����ʾ
}