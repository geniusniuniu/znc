#ifndef MY_CAMERA_H_
#define MY_CAMERA_H_

#define uesr_RED     0XF800    //红色
#define uesr_GREEN   0X07E0    //绿色
#define uesr_BLUE    0X001F    //蓝色


#define bin_jump_num    1//跳过的点数
#define border_max  LCDW-2 //边界最大值
#define border_min  1   //边界最小值

extern uint8_t mid_line;
extern float Middle;
extern unsigned char BarrierFlag,Process_flag;    //障碍识别标志位
extern unsigned char Crossroadflag;  //十字识别标志位
extern unsigned char garageout_flag; //车库识别标志位
//extern uint16 edgeL[LCDH];
extern int Num;
//extern uint16 edgeR[LCDH];
extern float error_Camera;
extern float slope;//中线斜率
extern unsigned char flag_centre_right_point;//右拐点标志位
extern unsigned char flag_centre_left_point;//左拐点标志位
extern unsigned char ringflag;

extern uint8_t median;

///////////////////////     要用的函数      ////////////////////////////////
void Image(void);
void Calculates_median(void);
void My_Camera(void);
///////////////////////     要用的函数      ////////////////////////////////



#endif



































