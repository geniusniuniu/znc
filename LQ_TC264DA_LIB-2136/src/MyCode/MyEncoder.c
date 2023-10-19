#include "LQ_GPT12_ENC.H"
#include "LQ_TFT18.h"
#include "stdio.h"
#include "My_Timer.h"
#include "PID.h"

char txt[64];

volatile float EncVal_L,EncVal_R,EncVal_F;

void My_EncInit (void)
{
    ENC_InitConfig(ENC4_InPut_P02_8, ENC4_Dir_P33_5);//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷稟閿熸枻鎷烽敓鏂ゆ嫹鍊�
    ENC_InitConfig(ENC5_InPut_P10_3, ENC5_Dir_P10_1);//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷稡閿熸枻鎷烽敓鏂ゆ嫹鍊�
    ENC_InitConfig(ENC6_InPut_P20_3, ENC6_Dir_P20_0);//閿熷彨鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓琛楋拷
}


void Get_EncVal(void)
{
    /* 閿熸枻鎷峰彇閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰�� */
    EncVal_R = ENC_GetCounter(ENC5_InPut_P10_3); //閸欏疇绔�
    EncVal_L = ENC_GetCounter(ENC4_InPut_P02_8); //瀹革箒绔�
    EncVal_F = ENC_GetCounter(ENC6_InPut_P20_3); //瀹革箒绔�
}

void Show_EncVal(void)
{
   // 鏄剧ずPID
//   sprintf(txt, "%.0f", PID_Struct.Kp_Angle);
//   TFTSPI_P8X16Str(1, 0, txt, u16WHITE, u16BLACK);
   sprintf(txt, "%.2f", PID_Struct.Ki_Balance);
   TFTSPI_P8X16Str(1, 0, txt, u16WHITE, u16BLACK);
   sprintf(txt, "%.1f", PID_Struct.Kp_Angle);
   TFTSPI_P8X16Str(9, 0, txt, u16WHITE, u16BLACK);

   //鏄剧ず  鍔ㄩ噺杞� 鍜� 琛岃繘鐢垫満 PWM
   sprintf(txt, "PWM: %05.2f", Pid_Out);
   TFTSPI_P8X16Str(1, 1, txt, u16WHITE, u16BLACK);       //閿熻鍑ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹绀�
   sprintf(txt, "PWM: %05.2f", Pid_Out_F);
   TFTSPI_P8X16Str(1, 2, txt, u16WHITE, u16BLACK);       //閿熻鍑ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹绀�


   //鏄剧ず缂栫爜鍣ㄧ殑鍊�
//   sprintf(txt, "E1: %05.0f", EncVal_R);
//   TFTSPI_P8X16Str(1, 1, txt, u16WHITE, u16BLACK);
//   sprintf(txt, "E2: %05.0f", EncVal_L);
//   TFTSPI_P8X16Str(1, 2, txt, u16WHITE, u16BLACK);
   sprintf(txt, "E3: %05.0f", EncVal_F);
   TFTSPI_P8X16Str(1, 6, txt, u16WHITE, u16BLACK);



}
