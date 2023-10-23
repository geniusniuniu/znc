#include <stdio.h>
#include <stdint.h>
#include "include.h"
#include "LQ_STM.h"
#include "LQ_GPIO.h"
#include "LQ_TFT18.h"
#include "LQ_GPIO_LED.h"
#include "LQ_TFTPicFont.h"

#include "LQ_CAMERA.h"
#include "LQ_TFT18.h"
#include "LQ_DMA.h"
#include "LQ_UART.h"
#include "My_Camera.h"


int my_abs(int value)
{
    if(value>=0) return value;
    else return -value;
}

int16_t my_abs_int16(int16_t temp)
{
  if(temp >= 0)  return temp;
  else           return -temp;
}

sint16 limit_a_b(sint16 x, int a, int b)
{
    if(x<a) x = (sint16)a;
    if(x>b) x = (sint16)b;
    return x;
}

/*
函数名称：int16 limit(int16 x, int16 y)
功能说明：求x,y中的最小值
参数说明：
函数返回：返回两值中的最小值
修改时间：2022年9月8日
备    注：
example：  limit( x,  y)
 */
sint16 limit1(sint16 x, sint16 y)
{
    if (x > y)             return y;
    else if (x < -y)       return -y;
    else                return x;
}

/*
函数名称：void get_start_point(uint8 start_row)
功能说明：寻找两个边界的边界点作为八邻域循环的起始点
参数说明：输入任意行数
函数返回：无
修改时间：2022年9月8日
备    注：
example：  get_start_point(LCDH-2)
 */
uint8 start_point_l[2] = { 0 };//左边起点的x，y值
uint8 start_point_r[2] = { 0 };//右边起点的x，y值
uint8 get_start_point(uint8 start_row)
{
    uint8 i = 0,l_found = 0,r_found = 0;
    //清零
    start_point_l[0] = 0;//x
    start_point_l[1] = 0;//y

    start_point_r[0] = 0;//x
    start_point_r[1] = 0;//y

    //从中间往左边，先找起点
    for (i = LCDW / 2; i > border_min; i--)
    {
        start_point_l[0] = i;//x
        start_point_l[1] = start_row;//y
        if (Bin_Image[start_row][i] == 255 && Bin_Image[start_row][i - 1] == 0)
        {
            //printf("找到左边起点image[%d][%d]\n", start_row,i);
            l_found = 1;
            break;
        }
    }

    for (i = LCDW / 2; i < border_max; i++)
    {
        start_point_r[0] = i;//x
        start_point_r[1] = start_row;//y
        if (Bin_Image[start_row][i] == 255 && Bin_Image[start_row][i + 1] == 0)
        {
            //printf("找到右边起点image[%d][%d]\n",start_row, i);
            r_found = 1;
            break;
        }
    }

    if(l_found&&r_found)return 1;
    else {
        //printf("未找到起点\n");
        return 0;
    }
}

/*
左右线一次性找完。
参数说明：
break_flag_r            ：最多需要循环的次数
(*image)[LCDW]       ：需要进行找点的图像数组，必须是二值图,填入数组名称即可
                       特别注意，不要拿宏定义名字作为输入参数，否则数据可能无法传递过来
*l_stastic              ：统计左边数据，用来输入初始数组成员的序号和取出循环次数
*r_stastic              ：统计右边数据，用来输入初始数组成员的序号和取出循环次数
l_start_x               ：左边起点横坐标
l_start_y               ：左边起点纵坐标
r_start_x               ：右边起点横坐标
r_start_y               ：右边起点纵坐标
hightest                ：循环结束所得到的最高高度
 */
#define USE_num LCDH*3   //定义找点的数组成员个数按理说300个点能放下，但是有些特殊情况确实难顶，多定义了一点

 //存放点的x，y坐标
uint16 points_l[(uint16)USE_num][2] = { {  0 } };//左线
uint16 points_r[(uint16)USE_num][2] = { {  0 } };//右线
uint16 dir_r[(uint16)USE_num] = { 0 };//用来存储右边生长方向
uint16 dir_l[(uint16)USE_num] = { 0 };//用来存储左边生长方向
uint16 data_stastics_l = 0;//统计左边找到点的个数
uint16 data_stastics_r = 0;//统计右边找到点的个数
uint8 hightest = 0;//最高点
void search_l_r(uint16 break_flag, uint8(*image)[LCDW], uint16 *l_stastic, uint16 *r_stastic, uint8 l_start_x, uint8 l_start_y, uint8 r_start_x, uint8 r_start_y, uint8*hightest)
{

    uint8 i = 0, j = 0;

    //左边变量
    uint8 search_filds_l[8][2] = { {  0 } };
    uint8 index_l = 0;
    uint8 temp_l[8][2] = { {  0 } };
    uint8 center_point_l[2] = {  0 };
    uint16 l_data_statics;//统计左边
    //定义八个邻域
    static sint8 seeds_l[8][2] = { {0,  1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,  0},{1, 1}, };
    //{-1,-1},{0,-1},{+1,-1},
    //{-1, 0},       {+1, 0},
    //{-1,+1},{0,+1},{+1,+1},
    //这个是顺时针

    //右边变量
    uint8 search_filds_r[8][2] = { {  0 } };
    uint8 center_point_r[2] = { 0 };//中心坐标点
    uint8 index_r = 0;//索引下标
    uint8 temp_r[8][2] = { {  0 } };
    uint16 r_data_statics;//统计右边
    //定义八个邻域
    static sint8 seeds_r[8][2] = { {0,  1},{1,1},{1,0}, {1,-1},{0,-1},{-1,-1}, {-1,  0},{-1, 1}, };
    //{-1,-1},{0,-1},{+1,-1},
    //{-1, 0},       {+1, 0},
    //{-1,+1},{0,+1},{+1,+1},
    //这个是逆时针

    l_data_statics = *l_stastic;//统计找到了多少个点，方便后续把点全部画出来
    r_data_statics = *r_stastic;//统计找到了多少个点，方便后续把点全部画出来

    //第一次更新坐标点  将找到的起点值传进来
    center_point_l[0] = l_start_x;//x
    center_point_l[1] = l_start_y;//y
    center_point_r[0] = r_start_x;//x
    center_point_r[1] = r_start_y;//y

        //开启邻域循环
    while (break_flag--)
    {

        //左边
        for (i = 0; i < 8; i++)//传递8F坐标
        {
            search_filds_l[i][0] = center_point_l[0] + seeds_l[i][0];//x
            search_filds_l[i][1] = center_point_l[1] + seeds_l[i][1];//y
        }
        //中心坐标点填充到已经找到的点内
        points_l[l_data_statics][0] = center_point_l[0];//x
        points_l[l_data_statics][1] = center_point_l[1];//y
        l_data_statics++;//索引加一

        //右边
        for (i = 0; i < 8; i++)//传递8F坐标
        {
            search_filds_r[i][0] = center_point_r[0] + seeds_r[i][0];//x
            search_filds_r[i][1] = center_point_r[1] + seeds_r[i][1];//y
        }
        //中心坐标点填充到已经找到的点内
        points_r[r_data_statics][0] = center_point_r[0];//x
        points_r[r_data_statics][1] = center_point_r[1];//y

        index_l = 0;//先清零，后使用
        for (i = 0; i < 8; i++)
        {
            temp_l[i][0] = 0;//先清零，后使用
            temp_l[i][1] = 0;//先清零，后使用
        }

        //左边判断
        for (i = 0; i < 8; i++)
        {
            if (image[search_filds_l[i][1]][search_filds_l[i][0]] == 0
                && image[search_filds_l[(i + 1) & 7][1]][search_filds_l[(i + 1) & 7][0]] == 255)
            {
                temp_l[index_l][0] = search_filds_l[(i)][0];
                temp_l[index_l][1] = search_filds_l[(i)][1];
                index_l++;
                dir_l[l_data_statics - 1] = (i);//记录生长方向
            }

            if (index_l)
            {
                //更新坐标点
                center_point_l[0] = temp_l[0][0];//x
                center_point_l[1] = temp_l[0][1];//y
                for (j = 0; j < index_l; j++)
                {
                    if (center_point_l[1] > temp_l[j][1])
                    {
                        center_point_l[0] = temp_l[j][0];//x
                        center_point_l[1] = temp_l[j][1];//y
                    }
                }
            }

        }
        if ((points_r[r_data_statics][0]== points_r[r_data_statics-1][0]&& points_r[r_data_statics][0] == points_r[r_data_statics - 2][0]
            && points_r[r_data_statics][1] == points_r[r_data_statics - 1][1] && points_r[r_data_statics][1] == points_r[r_data_statics - 2][1])
            ||(points_l[l_data_statics-1][0] == points_l[l_data_statics - 2][0] && points_l[l_data_statics-1][0] == points_l[l_data_statics - 3][0]
                && points_l[l_data_statics-1][1] == points_l[l_data_statics - 2][1] && points_l[l_data_statics-1][1] == points_l[l_data_statics - 3][1]))
        {
            //printf("三次进入同一个点，退出\n");
            break;
        }
        if (my_abs(points_r[r_data_statics][0] - points_l[l_data_statics - 1][0]) < 2
            && my_abs(points_r[r_data_statics][1] - points_l[l_data_statics - 1][1] < 2)
            )
        {
            //printf("\n左右相遇退出\n");
            *hightest = (points_r[r_data_statics][1] + points_l[l_data_statics - 1][1]) >> 1;//取出最高点
            //printf("\n在y=%d处退出\n",*hightest);
            break;
        }
        if ((points_r[r_data_statics][1] < points_l[l_data_statics - 1][1]))
        {
            continue;//如果左边比右边高了，左边等待右边
        }
        if (dir_l[l_data_statics - 1] == 7
            && (points_r[r_data_statics][1] > points_l[l_data_statics - 1][1]))//左边比右边高且已经向下生长了
        {
            //printf("\n左边开始向下了，等待右边，等待中... \n");
            center_point_l[0] = (uint8)points_l[l_data_statics - 1][0];//x
            center_point_l[1] = (uint8)points_l[l_data_statics - 1][1];//y
            l_data_statics--;
        }
        r_data_statics++;//索引加一

        index_r = 0;//先清零，后使用
        for (i = 0; i < 8; i++)
        {
            temp_r[i][0] = 0;//先清零，后使用
            temp_r[i][1] = 0;//先清零，后使用
        }

        //右边判断
        for (i = 0; i < 8; i++)
        {
            if (image[search_filds_r[i][1]][search_filds_r[i][0]] == 0
                && image[search_filds_r[(i + 1) & 7][1]][search_filds_r[(i + 1) & 7][0]] == 255)
            {
                temp_r[index_r][0] = search_filds_r[(i)][0];
                temp_r[index_r][1] = search_filds_r[(i)][1];
                index_r++;//索引加一
                dir_r[r_data_statics - 1] = (i);//记录生长方向
                //printf("dir[%d]:%d\n", r_data_statics - 1, dir_r[r_data_statics - 1]);
            }
            if (index_r)
            {

                //更新坐标点
                center_point_r[0] = temp_r[0][0];//x
                center_point_r[1] = temp_r[0][1];//y
                for (j = 0; j < index_r; j++)
                {
                    if (center_point_r[1] > temp_r[j][1])
                    {
                        center_point_r[0] = temp_r[j][0];//x
                        center_point_r[1] = temp_r[j][1];//y
                    }
                }

            }
        }


    }


    //取出循环次数
    *l_stastic = l_data_statics;
    *r_stastic = r_data_statics;

}
/*
功能：从八邻域边界里提取需要的左边线
参数：total_L ：找到的点的总数
*/
uint8 l_border[LCDH];//左线数组
uint8 r_border[LCDH];//右线数组
uint8 center_line[LCDH];//中线数组
void get_left(uint16 total_L)
{
    uint8 i = 0;
    uint16 j = 0;
    uint8 h = 0;
    //初始化
    for (i = 0;i<LCDH;i++)
    {
        l_border[i] = border_min;
    }
    h = LCDH - 2;
    //左边
    for (j = 0; j < total_L; j++)
    {
        if (points_l[j][1] == h)
        {
            l_border[h] = points_l[j][0]+1;
        }
        else continue; //每行只取一个点，没到下一行就不记录
        h--;
        if (h == 0)
        {
            break;//到最后一行退出
        }
    }
}
/*
功能：从八邻域边界里提取需要的右边线
参数：total_R  ：找到的点的总数
*/
void get_right(uint16 total_R)
{
    uint8 i = 0;
    uint16 j = 0;
    uint8 h = 0;
    for (i = 0; i < LCDH; i++)
    {
        r_border[i] = border_max;//右边线初始化放到最右边，左边线放到最左边，这样八邻域闭合区域外的中线就会在中间，不会干扰得到的数据
    }
    h = LCDH - 2;
    //右边
    for (j = 0; j < total_R; j++)
    {
        if (points_r[j][1] == h)
        {
            r_border[h] = points_r[j][0] - 1;
        }
        else continue;//每行只取一个点，没到下一行就不记录
        h--;
        if (h == 0)break;//到最后一行退出
    }
}


//滤波函数
//定义膨胀和腐蚀的阈值区间
#define threshold_max   255*5//此参数可根据自己的需求调节
#define threshold_min   255*2//此参数可根据自己的需求调节
void image_filter(uint8(*Bin_Image)[LCDW])//形态学滤波
{
    uint16 i, j;
    uint32 num = 0;


    for (i = 1; i < LCDH - 1; i++)
    {
        for (j = 1; j < (LCDW - 1); j++)
        {
            //统计八个方向的像素值
            num =
                Bin_Image[i - 1][j - 1] + Bin_Image[i - 1][j] + Bin_Image[i - 1][j + 1]
                + Bin_Image[i][j - 1] + Bin_Image[i][j + 1]
                + Bin_Image[i + 1][j - 1] + Bin_Image[i + 1][j] + Bin_Image[i + 1][j + 1];


            if (num >= threshold_max && Bin_Image[i][j] == 0)
            {

                Bin_Image[i][j] = 255;//白

            }
            if (num <= threshold_min && Bin_Image[i][j] == 255)
            {

                Bin_Image[i][j] = 0;//黑

            }

        }
    }

}

//给图像画一个黑框
void image_draw_rectan(uint8(*image)[LCDW])
{

    uint8 i = 0;
    for (i = 0; i < LCDH; i++)
    {
        image[i][0] = 0;
        image[i][1] = 0;
        image[i][LCDW - 1] = 0;
        image[i][LCDW - 2] = 0;

    }
    for (i = 0; i < LCDW; i++)
    {
        image[0][i] = 0;
        image[1][i] = 0;
//        image[LCDH-1][i] = 0;
    }
}

int median=0;

unsigned char weight_list[40]={
        2,2,2,2,2,
        4,4,4,4,4,
        6,6,6,6,6,
        10,10,10,10,10,
        10,10,10,10,10,
        7,7,7,7,7,
        5,5,5,5,5,
        3,3,3,3,3,
};

void Calculates_median(void)
{
    uint16_t weight_sum = 0;
    int16_t mid_line_error = 0;      //行与行之间中点的横坐标误差
    uint8_t mid_diuxian_flag = 0;    //丟线标志位
    uint16_t weight_midline_sum = 0;

    for (uint8_t i = LCDH-2 ; i>38 ; i--)
    {
        if (i == LCDH-2)
        {
            weight_midline_sum += center_line[i] * weight_list[(LCDH-2)-i];  // 分子
            weight_sum += weight_list[(LCDH-2)-i];                           // 分母
        }
        else
        {
            mid_line_error = my_abs_int16( (int16_t) (center_line[i] - center_line[i+1]) );

            if (mid_diuxian_flag == 1)
            {
                continue;
            }
            else if (mid_line_error > 34 && mid_diuxian_flag == 0)
            {
                mid_diuxian_flag = 1;
                continue;
            }
            else
            {
                weight_midline_sum += center_line[i] * weight_list[(LCDH-2)-i];
                weight_sum += weight_list[(LCDH-2)-i];
            }
        }
    }
    median = (uint8_t)(weight_midline_sum / weight_sum);
}



/**
* @brief 最小二乘法
* @param uint8 begin                输入起点
* @param uint8 end                  输入终点
* @param uint8 *border              输入需要计算斜率的边界首地址
*  @see CTest       Slope_Calculate(start, end, border);//斜率
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
*/
float Slope_Calculate(uint8 begin, uint8 end, uint8 *border)
{
    float xsum = 0, ysum = 0, xysum = 0, x2sum = 0;
    sint16 i = 0;
    float result = 0;
    static float resultlast;

    for (i = begin; i < end; i++)
    {
        xsum += i;
        ysum += border[i];
        xysum += i * (border[i]);
        x2sum += i * i;

    }
    if ((end - begin)*x2sum - xsum * xsum) //判断除数是否为零
    {
        result = ((end - begin)*xysum - xsum * ysum) / ((end - begin)*x2sum - xsum * xsum);
        resultlast = result;
    }
    else
    {
        result = resultlast;
    }
    return result;
}

/**
* @brief 计算斜率截距
* @param uint8 start                输入起点
* @param uint8 end                  输入终点
* @param uint8 *border              输入需要计算斜率的边界
* @param float *slope_rate          输入斜率地址
* @param float *intercept           输入截距地址
*  @see CTest       calculate_s_i(start, end, r_border, &slope_l_rate, &intercept_l);
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
*/
void calculate_s_i(uint8 start, uint8 end, uint8 *border, float *slope_rate, float *intercept)
{
    uint16 i, num = 0;
    uint16 xsum = 0, ysum = 0;
    float y_average, x_average;

    num = 0;
    xsum = 0;
    ysum = 0;
    y_average = 0;
    x_average = 0;
    for (i = start; i < end; i++)
    {
        xsum += i;
        ysum += border[i];
        num++;
    }

    //计算各个平均数
    if (num)
    {
        x_average = (float)(xsum / num);
        y_average = (float)(ysum / num);

    }

    /*计算斜率*/
    *slope_rate = Slope_Calculate(start, end, border);//斜率
    *intercept = y_average - (*slope_rate)*x_average;//截距
}

/**
* @brief 十字补线函数
* @param uint8(*image)[image_w]     输入二值图像
* @param uint8 *l_border            输入左边界首地址
* @param uint8 *r_border            输入右边界首地址
* @param uint16 total_num_l         输入左边循环总次数
* @param uint16 total_num_r         输入右边循环总次数
* @param uint16 *dir_l              输入左边生长方向首地址
* @param uint16 *dir_r              输入右边生长方向首地址
* @param uint16(*points_l)[2]       输入左边轮廓首地址
* @param uint16(*points_r)[2]       输入右边轮廓首地址
*  @see CTest       cross_fill(image,l_border, r_border, data_statics_l, data_statics_r, dir_l, dir_r, points_l, points_r);
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
 */
void cross_fill(uint8(*image)[LCDW], uint8 *l_border, uint8 *r_border, uint16 total_num_l, uint16 total_num_r,
                                         uint16 *dir_l, uint16 *dir_r, uint16(*points_l)[2], uint16(*points_r)[2])
{
    uint8 i;
    uint8 break_num_l = 0;
    uint8 break_num_r = 0;
    uint8 start, end;
    float slope_l_rate = 0, intercept_l = 0;
    //出十字
    for (i = 1; i < total_num_l; i++)
    {
        if (dir_l[i - 1] == 4 && dir_l[i] == 4 && dir_l[i + 3] == 6 && dir_l[i + 5] == 6 && dir_l[i + 7] == 6)
        {
            break_num_l = (uint8)points_l[i][1];//传递y坐标
//            printf("brea_knum-L:%d\n", break_num_l);
//            printf("I:%d\n", i);
//            printf("十字标志位：1\n");
            break;
        }
    }
    for (i = 1; i < total_num_r; i++)
    {
        if (dir_r[i - 1] == 4 && dir_r[i] == 4 && dir_r[i + 3] == 6 && dir_r[i + 5] == 6 && dir_r[i + 7] == 6)
        {
            break_num_r = (uint8)points_r[i][1];//传递y坐标
//            printf("brea_knum-R:%d\n", break_num_r);
//            printf("I:%d\n", i);
//            printf("十字标志位：1\n");
            break;
        }
    }
    if (break_num_l&&break_num_r&&image[LCDH - 25][4] && image[LCDH - 25][LCDW - 4])//两边生长方向都符合条件
    {
        //计算斜率
        start = break_num_l - 15;
        start = (uint8)limit_a_b(start, 0, LCDH);
        end = break_num_l - 5;
        calculate_s_i(start, end, l_border, &slope_l_rate, &intercept_l);
//        printf("slope_l_rate:%d\nintercept_l:%d\n", slope_l_rate, intercept_l);
        for (i = break_num_l - 5; i < LCDH - 1; i++)
        {
            l_border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
            l_border[i] = (uint8)limit_a_b(l_border[i], border_min, border_max);//限幅
        }

        //计算斜率
        start = break_num_r - 15;//起点
        start = (uint8)limit_a_b(start, 0, LCDH);//限幅
        end = break_num_r - 5;//终点
        calculate_s_i(start, end, r_border, &slope_l_rate, &intercept_l);
        //printf("slope_l_rate:%d\nintercept_l:%d\n", slope_l_rate, intercept_l);
        for (i = break_num_r - 5; i < LCDH - 1; i++)
        {
            r_border[i] = slope_l_rate * (i)+intercept_l;
            r_border[i] = (uint8)limit_a_b(r_border[i], border_min, border_max);
        }


    }

}


void Round_fill(uint8(*image)[LCDW], uint8 *l_border, uint8 *r_border, uint16 total_num_l, uint16 total_num_r,
                                         uint16 *dir_l, uint16 *dir_r, uint16(*points_l)[2], uint16(*points_r)[2])
{

    uint8 break_numl_y = 0,break_numl_x = 0;
        uint8 break_numr_y = 0,break_numr_x = 0;
        float slope_l_rate = 0, intercept_l = 0;
        float slope_r_rate = 0, intercept_r = 0;
        static uint8 stepl=0;
        //static uint8 stepr=0;
    for (int i = 1; i < total_num_l; i++)
        {
            if (points_l[i][1] > 20)
            {
                if(stepl == 0||stepl == 1)
                {
                    if ( (dir_l[i-2]==5) && (dir_l[i-1]==5) && (dir_l[i]==4)  && (dir_l[i+3]==3||dir_l[i+3]==4) &&
                            (dir_l[i+4]==3||dir_l[i+4]==4) )
                    {
                        break_numl_y = (uint8)points_l[i][1];//传递y坐标
                        break_numl_x = (uint8)points_l[i][0];

                        stepl = 1;
                    }
                }
                if(stepl == 1||stepl == 2)
                {
                    if ( (dir_l[i-4]==7) && (dir_l[i]==6||dir_l[i]==7) && (dir_l[i+4]==5||dir_l[i+4]==4) )
                    {
                        break_numl_y = (uint8)points_l[i][1];
                        break_numl_x = (uint8)points_l[i][0];

                        stepl = 2;
                    }
                }
                if(stepl == 2||stepl == 3)
                {
                    if (i < 30)
                    {
                        if((dir_r[i]==2||dir_r[i]==3))
                        {
                            break_numr_y = (uint8)points_r[i][1];
                            break_numr_x = (uint8)points_r[i][0];
                            stepl = 3;
                        }
                    }

                }
                if(stepl == 3||stepl == 4)
                {
                   if ( (dir_l[i-4]==7) && (dir_l[i]==6||dir_l[i]==7) && (dir_l[i+4]==5||dir_l[i+4]==4) )
                   {
                       break_numl_y = (uint8)points_l[i][1];
                       break_numl_x = (uint8)points_l[i][0];

                       stepl = 4;
                   }
               }
            }
            else
                break;
            }
    if (stepl==1 && break_numl_y)
        {
            slope_l_rate = 1.0*(break_numl_x-2)/(break_numl_y-78);
            intercept_l = break_numl_x - slope_l_rate*break_numl_y;
            for (int i = break_numl_y; i<LCDH-1; i++)
            {
                l_border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                l_border[i] = (uint8)limit_a_b(l_border[i], border_min, border_max);//限幅
            }
        }
        if (stepl == 2 && break_numl_y)
        {
            slope_r_rate = 1.0*(break_numl_x-97)/(break_numl_y-78);
            intercept_r = break_numl_x - slope_r_rate*break_numl_y;
            for (int i = break_numl_y; i<LCDH-1; i++)
            {
                r_border[i] = slope_r_rate * (i)+intercept_r;//y = kx+b
                r_border[i] = (uint8)limit_a_b(r_border[i], border_min, border_max);//限幅
            }
        }

        if (stepl==3 && break_numr_y)
           {
                slope_r_rate = 1.0*(break_numr_x-20)/(break_numr_y-30);
                intercept_r = break_numr_x - slope_r_rate*break_numr_y;
                for (int i = break_numr_y; i>30; i--)
                {
                    r_border[i] = slope_r_rate * (i)+intercept_r;;//y = kx+b
                    r_border[i] = (uint8)limit_a_b(r_border[i], border_min, border_max);//限幅
                }
           }
        if (stepl==4 && break_numl_y)
            {
                slope_l_rate = 1.0*(break_numl_x-0)/(break_numl_y-80);
                intercept_l = break_numl_x - slope_l_rate*break_numl_y;
                for (int i = break_numl_y; i<LCDH-1; i++)
                {
                    l_border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    l_border[i] = (uint8)limit_a_b(l_border[i], border_min, border_max);//限幅
                }
            }
}

void UartSendReport(unsigned char img[LCDH][LCDW])
{
    UART_PutChar(UART0,0xFC);
    UART_PutChar(UART0,0xCF);
    for(int i = 0;i<LCDH;i++)
    for(int j = 0;j<LCDW;j++)
    {
        if(img[i][j]==0) img[i][j]=255;
        else if(img[i][j]==255) img[i][j]=0;
        UART_PutChar(UART0,img[i][j]);
    }
    UART_PutChar(UART0,0xCF);
    UART_PutChar(UART0,0xFC);
}

unsigned char left_line_list[LCDH];   //左边线
unsigned char right_line_list[LCDH];  //右边线
unsigned char midline[LCDH];          //中线
uint8_t mid_line=0;                   //最后的计算的加权平均赛道中线的横坐标
uint8 OpenFlag,BarrierFlag=0,Process_flag=0;         //障碍识别标志位
unsigned char Miss=0,Crossroadflag=0,ringflag=0; //十字识别标志位
unsigned char garageout_flag=0;       //车库识别标志位
uint16 edgeL[LCDH];
uint16 edgeR[LCDH];
float error_Camera;
uint8 WHITE=0;
int  C_R_Flag[6]={0,0,1,1,1,0},Num=0;
char txt1[32];

float slope;//中线斜率
unsigned char flag_centre_right_point;//右拐点标志位
unsigned char flag_centre_left_point;//左拐点标志位
//static uint8 meetRingFlag=0;//遇环标志位，第一个A字标志点——第一个V字标志点
//static uint8 enterRingFlag=0;//入环标志位，第一个V字标志点——第二个A字标志点
//static uint8 leaveRingFlag=0;//出环标志位，第二个A字标志点——第二个V字标志点
//static uint8 passRingFlag=1;//过环标志位，第二个V字标志点——下一个A字标志点
//static uint8 ringSide=0;//环岛类型，1表示左，2表示右

int jumpFlagL=0;
int jumpFlagR=0,lefty[2],righty[2],l_down_flag=0,r_down_flag=0,r_up_flag=0,l_up_flag=0,flagl=0,flagr=0;
float parameterB,parameterA;
uint16 pointLX=0,pointLUX=0;//左边线跳变点X坐标
uint16 pointLY=0,pointLUY=0;//左边线跳变点Y坐标
uint16 pointRX=0,pointRUX=0;//右边线跳变点X坐标
uint16 pointRY=0,pointRUY=0;//右边线跳变点Y坐标
uint16 pointX=0;//跳变点X坐标
uint16 pointY=0;//跳变点Y坐标

uint8 pointSide=0;//跳变点方向，1表示左，2表示右
uint8 pointType=0;//跳变点分类，1表示A字跳变点，2表示V字跳变点

void Image(void)
{
    if (Camera_Flag == 2)
    {
        Get_Use_Image();              // 提取部分使用的数据
        Camera_Flag = 0;              // 清除摄像头采集完成标志位  如果不清除，则不会再次采集数据
    }
}


//寻找边线寻找赛道中线
void find_mid_line(unsigned char index[LCDH][LCDW])
{
    unsigned char left_point = 0;
    unsigned char right_point = LCDW-1;

    left_line_list[0] = 0;
    right_line_list[0] = 0;
    midline[0] = (left_point + right_point) / 2;

    for (unsigned char i = LCDH-1 ; i>0 ; i--)
    {
        left_point = 0;
        right_point = LCDW-1;

        //如果二分之一处为白点，从二分之一处找
        if (index[i][LCDW/2] == 1)
        {
            //寻找左边点
            for (unsigned char j = LCDW/2-1 ; j > 0 ; j--)
            {
                if (index[i][j] == 0 && index[i][j+1] == 1)
                {
                    left_point = j;
                    break;
                }
            }
            //寻找右边点
            for (unsigned char j = LCDW/2+1 ; j < LCDW ; j++)
            {
                if (index[i][j] == 0 && index[i][j-1] == 1)
                {
                    right_point = j;
                    break;
                }
            }
        }
        else if (index[i][LCDW/4] == 1)
        {
            //寻找左边点
            for (unsigned char j = LCDW/4-1 ; j > 0 ; j--)
            {
                if (index[i][j] == 0 && index[i][j+1] == 1)
                {
                    left_point = j;
                    break;
                }
            }
            //寻找右边点
            for (unsigned char j = LCDW/4+1 ; j < LCDW ; j++)
            {
                if (index[i][j] == 0 && index[i][j-1] == 1)
                {
                    right_point = j;
                    break;
                }
            }
        }
        else if (index[i][LCDW/4*3] == 1)
        {
            //寻找左边点
            for (unsigned char j = LCDW/4*3-1 ; j > 0 ; j--)
            {
                if (index[i][j] == 0 && index[i][j+1] == 1)
                {
                    left_point = j;
                    break;
                }
            }
            //寻找右边点
            for (unsigned char j = LCDW/4*3+1 ; j < LCDW ; j++)
            {
                if (index[i][j] == 0 && index[i][j-1] == 1)
                {
                    right_point = j;
                    break;
                }
            }
        }
        left_line_list[i] = left_point;
        right_line_list[i] = right_point;
        midline[i] = (left_point + right_point) / 2;
    }

//    if ( garage_out_flag == 1 )
//    {
//        for(int i=35;i<55;i++)
//        {
//            if( left_line_list[i]==0&&right_line_list[i]==LCDW-1 )
//            {
//                Miss++;
//                if(Miss>=15)
//                {
//                    Crossroadflag=1;
//                    Miss=0;
//                }
//
//            }
//        }
//    }
}


//九宫格画点
void draw_point(unsigned char x,unsigned char y,unsigned short color)
{
    if (x != LCDW-1)
    {
        TFTSPI_Draw_Dot(x+1,y,color);
        if (y != LCDH-1)
        {
            TFTSPI_Draw_Dot(x,y+1,color);
            TFTSPI_Draw_Dot(x+1,y+1,color);
        }
        if (y != 0)
        {
            TFTSPI_Draw_Dot(x,y-1,color);
            TFTSPI_Draw_Dot(x+1,y-1,color);
        }
    }
    if (x != 0)
    {
        TFTSPI_Draw_Dot(x-1,y,color);
        if (y != LCDH-1)
        {
            TFTSPI_Draw_Dot(x,y+1,color);
            TFTSPI_Draw_Dot(x-1,y+1,color);
        }
        if (y != 0)
        {
            TFTSPI_Draw_Dot(x,y-1,color);
            TFTSPI_Draw_Dot(x-1,y-1,color);
        }
    }
    TFTSPI_Draw_Dot(x,y,color);
}

int16_t abs_int16(int16_t temp)
{
  if(temp >= 0)  return temp;
  else           return -temp;
}

unsigned char mid_meight_list[30]={
        3,3,3,3,3,
        5,5,5,5,5,
        9,9,9,9,9,
        10,10,10,10,10,
        8,8,8,8,8,
        5,5,5,5,5
};

//绘制边线
void draw_line(void)
{
    for (unsigned char i = LCDH-1 ; i>0 ; i--)
    {
        draw_point(left_line_list[i],i,u16BLUE);
        draw_point(right_line_list[i],i,u16GREEN);
        draw_point(midline[i],i,u16RED);
    }
//    for (unsigned char i = LCDH-1 ; i>0 ; i--)
//    {
//        draw_point((unsigned char) edgeL[i],i,u16BLUE);
//        draw_point((unsigned char) edgeR[i],i,u16GREEN);
//    }
}

void find_mid_line_weight(void)
{
    uint16_t weight_sum = 0;
    int16_t mid_line_error = 0;      //行与行之间中点的横坐标误差
    uint8_t mid_diuxian_flag = 0;    //丟线标志位
    uint16_t weight_midline_sum = 0;

    for (uint8_t i = LCDH-1 ; i>49 ; i--)
    {
        if (i == LCDH-1)
        {
            weight_midline_sum += midline[i] * mid_meight_list[79-i];  // 分子
            weight_sum += mid_meight_list[79-i];                              // 分母
        }
        else
        {
            mid_line_error = abs_int16( (int16_t) (midline[i] - midline[i+1]) );

            if (mid_diuxian_flag == 1)
            {
                continue;
            }
            else if (mid_line_error > 34 && mid_diuxian_flag == 0)
            {
                mid_diuxian_flag = 1;
                continue;
            }
            else
            {
                weight_midline_sum += midline[i] * mid_meight_list[79-i];
                weight_sum += mid_meight_list[79-i];
            }
        }
    }
    mid_line = (uint8_t)(weight_midline_sum / weight_sum);
}


void ImageProcess(void)
{
    uint16 i,j;
    edgeL[LCDH-1]=0;
    edgeR[LCDH-1]=LCDW-1;
    midline[LCDH-1]=LCDW/2;

         for(i=LCDH-2;;i--)
         {
           edgeL[i]=0;
           edgeR[i]=LCDW-1;

           for(j=midline[i+1];j>0;j--){
             if(Bin_Image[i][j]==0){
               edgeL[i]=j;
               break;
             }
           }

           for(j=midline[i+1];j<LCDW-1;j++)
           {
             if(Bin_Image[i][j]==0){
               edgeR[i]=j;
               break;
             }
           }

           midline[i]=(edgeL[i]+edgeR[i])/2;//考虑环岛补线后不能边判断边线边计算中线，这个中线只用来下一步寻找边界

           if(i==0){
             break;
           }
         }

         float stepLength;//横坐标插值步长
         //四步过环，遇环、入环、出环、过环

//         uint16 pointLLCX=0;
//         uint16 pointLLCY=LCDH-1;
//         uint16 pointLRCX=LCDW-1;
//         uint16 pointLRCY=LCDH-1;
//         uint16 pointTMX=0;
//         uint16 pointTMY=0;
         /*----环岛处理在这里开始----*/




         for(i=10;i<LCDH-1;i++){//只判断下2/3部分图像的跳变点
           //左侧A字跳变点
           if(edgeL[i]-edgeL[i-1]>5&&edgeL[i-5]==0){
             jumpFlagL=1;
             pointLX=edgeL[i];
             pointLY=i;
             pointSide=1;
             pointType=1;
           }
           //左侧V字跳变点
           else if(edgeL[i-1]-edgeL[i]>10&&edgeL[i+5]==0){
             jumpFlagL=2;
             pointLUX=edgeL[i-1];
             pointLUY=i-1;
             pointSide=1;
             pointType=2;
           }
           //右侧A字跳变点
           if(edgeR[i-1]-edgeR[i]>5&&edgeR[i-5]==LCDW-1){
             jumpFlagR=1;
             pointRX=edgeR[i];
             pointRY=i;
             pointSide=2;
             pointType=1;
           }
           //右侧V字跳变点
           else if(edgeR[i]-edgeR[i-1]>10&&edgeR[i+5]==LCDW-1){
             jumpFlagR=2;
             pointRUX=edgeR[i-1];
             pointRUY=i-1;
             pointSide=2;
             pointType=2;
           }
         }

       if(jumpFlagL==1)
       {
             stepLength=0.5;
             for(i=pointLUY;i<pointLUY-pointLY;i++)
              {
                  edgeL[i]=(pointLX+5)+(int)(i*stepLength);
                  Bin_Image[i][edgeL[i]]=0;
              }

       }
     if(jumpFlagR==1)
         {
             stepLength=0.5;
             for(i=pointRUY;i<pointRUY-pointRY;i++)
              {
                  edgeL[i]=(pointRX+5)+(int)(i*stepLength);
                  Bin_Image[i][edgeR[i]]=0;
              }
         }

//         uint8 jumpFlag=(jumpFlagL^jumpFlagR)&&((pointLY>LCDH/2)||(pointRY>LCDH/2));//防止十字误判
//         if((pointLX==LCDW/2)&&(pointRX==LCDW/2)&&(pointLY>LCDH/2)&&(pointRY>LCDH/2)){
//           jumpFlag=1;//入环V形特殊情况
//         }
//
//         if(jumpFlagL==1){
//           pointX=pointLX;
//           pointY=pointLY;
//         }
//         else if(jumpFlagR==1){
//           pointX=pointRX;
//           pointY=pointRY;
//         }
//
//
//
//
//
//         if(jumpFlag){//若有有效跳变点
//           if(pointType==1){//若为A字跳变点，说明出环或遇环
//             if(passRingFlag==1){//遇环//遇环仅判断方向用
//               meetRingFlag=1;
//               passRingFlag=0;
//             }
//             else if(enterRingFlag==1){//出环
//               leaveRingFlag=1;
//               enterRingFlag=0;
//             }
//           }
//           else if(pointType==2){//若为V字跳变点，说明入环或过环
//             if(meetRingFlag==1){//入环
//               enterRingFlag=1;
//               meetRingFlag=0;
//             }
//             else if(leaveRingFlag==1){//过环
//               passRingFlag=1;
//               leaveRingFlag=0;
//             }
//           }
//         }
//
//         if(meetRingFlag==1)
//         {
//           if(jumpFlag==1)
//           {
//             ringSide=pointSide;
//             if(ringSide==1)
//             {
//                 stepLength=0.5;
//                 for(i=0;i<LCDH;i++)
//                  {
//                      edgeL[pointLLCY-i]=(pointLLCX+5)+(int)(i*stepLength);
//                      Bin_Image[pointLLCY-i][edgeL[pointLLCY-i]]=0;
//                  }
//             }
//             else if(ringSide==2)
//             {
//                 stepLength=0.5;
//                 for(i=0;i<LCDH;i++)
//                  {
//                      edgeR[pointLRCY-i]=(pointLRCX-5)-(int)(i*stepLength);
//                      Bin_Image[pointLRCY-i][edgeR[pointLRCY-i]]=0;
//                  }
//             }
//           }
//         }
//         else if(enterRingFlag==1){
//           if(jumpFlag==1){
//             //入环补线
//             //入环标志点（pointAX，pointAY），图像左下角（pointLLCX，pointLLCY），图像右下角（pointLRCX，pointLRCY）
//             if(ringSide==1){//入环标志点在左边，从右下角点到入环标志点连线，补右边线
//               stepLength=(float)(pointLRCX-pointX)/(float)(pointLRCY-pointY);
//               for(i=0;i<pointLRCY-pointY;i++){
//                 edgeR[pointLRCY-i]=pointLRCX-(int)(i*stepLength);
//                 Bin_Image[pointLRCY-i][edgeR[pointLRCY-i]]=0;
//               }
//             }
//             else if(ringSide==2){//入环标志点在右边，从左下角点到入环标志点连线，补左边线
//               stepLength=(float)(pointX-pointLLCX)/(float)(pointLLCY-pointY);
//               for(i=0;i<pointLLCY-pointY;i++){
//                 edgeL[pointLLCY-i]=pointLLCX+(int)(i*stepLength);
//                 Bin_Image[pointLLCY-i][edgeL[pointLLCY-i]]=0;
//               }
//             }
//           }
//         }
//         else if(leaveRingFlag==1){
//           if(ringSide==1){//若左圆环
//               stepLength=0.5;
//              for(i=0;i<60;i++){
//             edgeR[pointLRCY-i]=pointLRCX-(int)(i*stepLength);
//              Bin_Image[pointLRCY-i][edgeR[pointLRCY-i]]=0;
//             }
//           }
//
//         else if(ringSide==2){
//             stepLength=0.5;
//          for(i=0;i<60;i++){
//            edgeL[pointLLCY-i]=pointLLCX+(int)(i*stepLength);
//         Bin_Image[pointLLCY-i][edgeL[pointLLCY-i]]=0;
//           }
//           }
//         }
//
//         else if(passRingFlag==1){
//           if(jumpFlag==1){
//             if(ringSide==1){
//               stepLength=(float)(pointX-pointLLCX)/(float)(pointLLCY-pointY);
//               for(i=0;i<pointLLCY-pointY;i++){
//                 edgeL[pointLLCY-i]=pointLLCX+(int)(i*stepLength);
//                 Bin_Image[pointLLCY-i][edgeL[pointLLCY-i]]=0;
//               }
//             }
//             else if(ringSide==2){
//               stepLength=(float)(pointLRCX-pointX)/(float)(pointLRCY-pointY);
//               for(i=0;i<pointLRCY-pointY;i++){
//                 edgeR[pointLRCY-i]=pointLRCX-(int)(i*stepLength);
//                 Bin_Image[pointLRCY-i][edgeR[pointLRCY-i]]=0;
//               }
//             }
//           }
//         }
//
//         /*----环岛处理在这里结束----*/

         for(i=0;i<LCDH;i++){//重新计算中线
           midline[i]=(edgeL[i]+edgeR[i])/2;
           Bin_Image[i][midline[i]]=Bin_Image[i][midline[i]-1]=Bin_Image[i][midline[i]+1]=0;//粗线
         }
   }


void garage_judge(void)
{
    unsigned char region= 0;
    unsigned char white_black=0,black_white=0;
    for(uint8 hang = 20;hang<35;hang++)
    {
        unsigned char garage_count= 0;
        for(uint8 lie = 10;lie<100;lie++)
        {
            if(Bin_Image[hang][lie] == 1)
            {
                white_black=1;
            }
            else
            {
                white_black=0;
            }

            if(white_black!=black_white)
            {
                black_white = white_black;
                garage_count++;
            }
            if(garage_count>11)
            {
                region++;
            }
        }
        if(region>2)
        {
            garageout_flag=1;
            break;
        }
     }
}

/************************************线性回归计算中线斜率************************************/
// y = Ax+B
void regression(int startline,int endline)
{

    int i=0,SumX=0,SumY=0,SumLines = 0;
    float SumUp=0,SumDown=0,avrX=0,avrY=0;
    SumLines=endline-startline;   // startline 为开始行， //endline 结束行 //SumLines

    for(i=startline;i<endline;i++)
    {
        SumX+=i;
        SumY+=midline[i];    //这里Middle_black为存放中线的数组
    }
    avrX=(float)(SumX/SumLines);     //X的平均值
    avrY=(float)(SumY/SumLines);     //Y的平均值
    SumUp=0;
    SumDown=0;
    for(i=startline;i<endline;i++)
    {
        SumUp+=(midline[i]-avrY)*(i-avrX);
        SumDown+=(i-avrX)*(i-avrX);
    }
    if(SumDown==0)
        parameterB=0;
    else
        parameterB=(SumUp/SumDown);
    parameterA=(SumY-parameterB*SumX)/SumLines;  //截距
//    return parameterB;  //返回斜率
}

//S弯道识别（中S弯小S弯）
//参数：寻找中线左拐点和右拐点的起始行和终点行
void find_guaidian(int start_line,int end_line)
{
    int i = 0;
    uint8_t centre_left_point[2];
    uint8_t centre_right_point[2];
    //延长标志位,对最后一个不能扫到的中线拐点置延长标志位，比如最后右拐进入直道
    int S_wandao_T;
    //标志位清零
    flag_centre_right_point = 0;
    flag_centre_left_point = 0;
    //坐标归零
    centre_left_point[0] = 0;
    centre_left_point[1] = 0;
    centre_right_point[0] = 0;
    centre_right_point[1] = 0;
    //对斜率取绝对值
    if(slope<0)
    {
        slope=-(slope);
    }
    //从下往上找，80行到20行
    for (i = end_line; i>=start_line; i--)
    {
        //中线右拐点,用斜率和拐点所在区间辅助判断，减少误判几率
     if (midline[i-1]-midline[i]>=1&&midline[i+1]-midline[i]>=1&&
             midline[i+5]-midline[i]>=5&&midline[i-5]-midline[i]>=5
        && midline[i] < 50 && midline[i] > 0 && slope >= 0.2)
        {
         //找到右拐点后，记录右拐点坐标，并且标志位置1
            centre_right_point[0] = (unsigned char)i;//y
            centre_right_point[1] = midline[i];//x
            flag_centre_right_point = 1;
        }
        //中线左拐点
     if (midline[i]-midline[i-1]>=1&&midline[i]-midline[i+1]>=1&&
             midline[i]-midline[i+5]>=5&&midline[i]-midline[i-5]>=5
        && midline[i] > 50 && midline[i] < 100 && slope>= 0.2)
        {
            centre_left_point[0] = (unsigned char)i;//y
            centre_left_point[1] = midline[i];//x
            flag_centre_left_point = 1;
            //延长标志位置1
            S_wandao_T=1;
        }
    }
    //即将出S弯道的时候，用延长标志位延长一下出S弯的时间，出去的时候只能扫到右拐点，扫不到左拐点,
    //延长S弯道标志位，避免差速过小拐不过去
    if(S_wandao_T==1&&flag_centre_right_point==1)
    {
        //将左拐点设定为扫线第一行的中线坐标
        centre_left_point[0] = 79;//y
        centre_left_point[1] = midline[79];//x
        flag_centre_left_point = 1;

    }

     int Middle_S_flag = 0;//中S弯标志位
     int flag_small_S = 0;//小S弯标志位
     //如果找到左拐点和右拐点判断弯道属性
    if (flag_centre_left_point == 1 && flag_centre_right_point == 1 )
    {
        //如果两个拐点都找到，并且左拐点和右拐点的差值满足一定范围，那么认为满足小S，否则就是中S
        if ((abs_int16(centre_left_point[0] - centre_right_point[0]) >= 8)
                && (abs_int16(centre_left_point[0] - centre_right_point[0]) <= 28)
           && abs_int16(centre_left_point[1] - centre_right_point[1]) >= 8
           && abs_int16(centre_left_point[1] - centre_right_point[1]) <= 35)
        {
            flag_small_S = 1;//小S标志位
        }
        else
        {
            Middle_S_flag = 1;//中S标志位
        }
    }
}


void barrier_judge(void)
{
    unsigned short i, j;
    for (i=20;i<40;i++)
    {
        if( Bin_Image[i][LCDW/2+1]==0 && Bin_Image[i][LCDW/2-1]==0 )
        {
            for(j=0;j<LCDW;j++)
            {
                if(Bin_Image[i][j]==1&&Bin_Image[i-2][j]==1&&Bin_Image[i-5][j]==1)
                {
                    WHITE++;
                }
            }
            if( WHITE<10 && OpenFlag==0 )
            {
                BarrierFlag = 1;
                OpenFlag = 1;
            }
            else if ( OpenFlag==1 && BarrierFlag==1 && WHITE<20 )
            {
                BarrierFlag = 2;
            }
        }
    }
}

void Corss_Ring()
{
    uint16 count=0;
        if(C_R_Flag[Num]==0 )
        {
            for(int i=45;i<65;i++)
          {
            if(left_line_list[i]==0&&right_line_list[i]==LCDW-1)
            {
                count++;
                if(count>15)
                {

                    Crossroadflag=1;
                    count=0;
                }
            }

          }
        }
//        else if(C_R_Flag[Num]==0 && Num==1)
//        {
//           buxian();
//        }
        else{

        for(int i=LCDH*1/3;i<LCDH-1;i++)//只判断下2/3部分图像的跳变点
        {
            if(Num==2||Num==4)
            {
                 //左侧A字跳变点
                 if(left_line_list[i]-left_line_list[i-1]>5&&left_line_list[i-5]==0&&i>50){
                   ringflag=1;
                 }
                 //左侧V字跳变点
                 else if(left_line_list[i-1]-left_line_list[i]>5&&left_line_list[i+5]==0){
                   ringflag=2;
                 }
            }
            else
            {
                 //右侧A字跳变点
                 if(right_line_list[i-1]-right_line_list[i]>5&&right_line_list[i-5]==LCDW-1&&i>50){
                   ringflag=3;
                 }
                 //右侧V字跳变点
                 else if(right_line_list[i]-right_line_list[i-1]>5&&right_line_list[i+5]==LCDW-1){
                   ringflag=4;
                 }
            }
        }

    }
}




//拟合直线函数
void line(int type, int startline, int endline, float parameterB, float parameterA)
        {
            if (type == 1) //左
            {
                for (int i = startline; i < endline; i++)
                {
                    left_line_list[i] =  (parameterB * i + parameterA);
//                    if (left_line_list[i] < 0)
//                    {
//                        left_line_list[i] = 185;
//
//                    }
//                    if (left_line_list[i] > 185)
//                    {
//                        left_line_list[i] = 185;
//
//                    }
                }
            }
            else if (type == 2)            //右
            {
                for (int i = startline; i < endline; i++)
                {
                    right_line_list[i] = (parameterB * i + parameterA);
//                    if (right_line_list[i] < 0)
//                    {
//                        right_line_list[i] = 0;
//
//                    }
//                    if (right_line_list[i] > 185)
//                    {
//                        right_line_list[i] = 0;
//
//                    }
                }
            }
            else if (type == 0)             //中
            {
                for (int i = startline; i < endline; i++)
                {
                    midline[i] =  ((left_line_list[i] / 2 + right_line_list[i] / 2));
                }
            }
        }


void buxian (void){

// 找拐点
if (l_down_flag == 0)
{
   for (int i = 1; i < 65; i++)
    {
        if (abs_int16(left_line_list[i] - left_line_list[i - 1]) < 5 && abs_int16(left_line_list[i + 1] - left_line_list[i]) < 5 && left_line_list[i + 2] - left_line_list[i + 1] > 3 && i + 1 >= 2 && left_line_list[i + 1] <LCDW-1 && i + 1 < 45)
        {
            lefty[0] =  (i + 1);
            l_down_flag = 1;
            break;
        }
    }

}
if (r_down_flag == 0)
            {

                    for (int i = 1; i < 65; i++)
                    {
                        if (abs_int16(right_line_list[i] - right_line_list[i - 1]) < 5 && abs_int16(right_line_list[i + 1] - right_line_list[i]) < 5 && right_line_list[i + 2] - right_line_list[i + 1] < -3 && i + 1 >= 2 && right_line_list[i + 1] > 2 && i + 1 < 45)
                        {
                            righty[0] =  (i + 1);
                            r_down_flag = 1;
                            break;
                        }
                    }
                }
if (l_up_flag == 0)
            {
                for (int i = 1; i < 65; i++)
                {
                    if (left_line_list[i] - left_line_list[i - 1] < -3 && abs_int16(left_line_list[i + 1] - left_line_list[i]) < 4 && abs_int16(left_line_list[i + 2] - left_line_list[i + 1]) < 4 && left_line_list[i] < LCDW-1 && (i > lefty[0]))
                    {
                        lefty[1] = i;
                        l_up_flag = 1;
                        break;
                    }
                }

            }
if (r_up_flag == 0)
            {
                for (int i = 1; i < 65; i++)
                {
                    if (right_line_list[i] - right_line_list[i - 1] > 3 && abs_int16(right_line_list[i + 1] - right_line_list[i]) < 4 && abs_int16(right_line_list[i + 2] - right_line_list[i + 1]) < 4 && right_line_list[i] > 2 && (i > righty[0]))
                    {
                        righty[1] =  i;
                        r_up_flag = 1;
                        break;
                    }
                }
            }
// 入十字前补线* regression是最小二乘法求斜率截距 *run是拟合直线函数 1为左 2为右 0为中
//abu是二次扫线的计数位，可先忽略
if (((lefty[0] > 0 && lefty[1] > 0) || (righty[0] > 0 && righty[1] > 0)))
         {

             if (lefty[0] >= 2 && lefty[1] >= 2)                                    //找左线              入十字前
             {
                 regression(lefty[0], lefty[1]);
                 line(1, lefty[0], lefty[1], parameterB, parameterA);
                 flagl = 1;                                                                                      //
             }
             if (righty[0] >= 2 && righty[1] >= 2 )                                         //找右线
             {
                 regression(righty[0] ,righty[1]);
                 line(2, righty[0], righty[1], parameterB, parameterA);
                 flagr = 1;
             }
             if ((r_down_flag == 1 || l_down_flag == 1) )
             {
                 if (flagl == 1 && flagr == 0)
                 {
                     line(0, lefty[0], lefty[1], parameterB, parameterA);                 //拟合中线
//                     if (abu >= 3)
//                     {
//                         run(0, 0, lefty[0], parameterB, parameterA);
//                         run(0, lefty[1], 50, parameterB, parameterA);
//                     }
                 }
                 else
                 {
                     line(0, righty[0], righty[1], parameterB, parameterA);                 //拟合中线
//                     if (abu >= 3)
//                     {
//                         run(0, 0, righty[0], parameterB, parameterA);
//                         run(0, righty[1],50, parameterB, parameterA);
//                     }
                 }

             }

         }Crossroadflag=2;
}

void My_Camera(void)
{
    uint16 i;
    uint8 hightest = 0;//定义一个最高行，tip：这里的最高指的是y值的最小

    //首先判断摄像头数据采集完成标志位
    if (Camera_Flag==0)
    {
        Get_Bin_Image(0);// 二值化
        image_filter(Bin_Image);//滤波
        image_draw_rectan(Bin_Image);//预处理

        data_stastics_l = 0;//清零
        data_stastics_r = 0;
        if (get_start_point(LCDH - 2))//找到起点了，再执行八领域，没找到就一直找
        {
            search_l_r((uint16)USE_num, Bin_Image, &data_stastics_l, &data_stastics_r, start_point_l[0], start_point_l[1], start_point_r[0], start_point_r[1], &hightest);
            //从爬取的边界线内提取边线
            get_left(data_stastics_l);
            get_right(data_stastics_r);
            //处理函数
            //cross_fill(Bin_Image,l_border, r_border, data_stastics_l, data_stastics_r, dir_l, dir_r, points_l, points_r);
            Round_fill(Bin_Image,l_border, r_border, data_stastics_l, data_stastics_r, dir_l, dir_r, points_l, points_r);
         }
        TFTSPI_BinRoad(0, 0, LCDH, LCDW, (unsigned char *) Bin_Image);
        //根据最终循环次数画出边界点
     //        for (i = 0; i < data_stastics_l; i++)
     //        {
     //            TFTSPI_Draw_Dot((unsigned char)points_l[i][0]+2, (unsigned char)points_l[i][1], uesr_GREEN);
     //        }
     //        for (i = 0; i < data_stastics_r; i++)
     //        {
     //            TFTSPI_Draw_Dot((unsigned char)points_r[i][0]-2, (unsigned char)points_r[i][1], uesr_GREEN);
     //        }
             for (i = 38; i < LCDH-1; i++)
             {
                 center_line[i] = (l_border[i] + r_border[i]) >> 1;//求中线
                 TFTSPI_Draw_Dot((unsigned char)center_line[i], (unsigned char)i, uesr_RED);
                 TFTSPI_Draw_Dot((unsigned char)l_border[i],(unsigned char) i, uesr_BLUE);
                 TFTSPI_Draw_Dot((unsigned char)r_border[i],(unsigned char) i, uesr_BLUE);
             }
             Calculates_median();
    }

}

void Process (void)
{
    if (Camera_Flag==0)
    {
        Get_Bin_Image(0);             // 二值化
        Bin_Image_Filter();           // 滤波，三面被围的数据将被修改为同一数值
//        ImageProcess();
        find_mid_line(Bin_Image);     // 寻找边线和赛道中线
//        barrier_judge();              // 障碍、断路识别
//        garage_judge();               // 车库识别
//        Corss_Ring();                 // 环岛，十字判断
        find_mid_line_weight();       // 计算中值
        draw_line();                  // 画线
    }
    TFTSPI_BinRoad(0, 0, LCDH, LCDW, (unsigned char *)Bin_Image); // 显示摄像头图像
    TFTSPI_P8X16Str(1,5,txt1,u16YELLOW, u16BLUE);
    LED_Ctrl(LED0, RVS);
}

