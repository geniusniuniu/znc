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
�������ƣ�int16 limit(int16 x, int16 y)
����˵������x,y�е���Сֵ
����˵����
�������أ�������ֵ�е���Сֵ
�޸�ʱ�䣺2022��9��8��
��    ע��
example��  limit( x,  y)
 */
sint16 limit1(sint16 x, sint16 y)
{
    if (x > y)             return y;
    else if (x < -y)       return -y;
    else                return x;
}

/*
�������ƣ�void get_start_point(uint8 start_row)
����˵����Ѱ�������߽�ı߽����Ϊ������ѭ������ʼ��
����˵����������������
�������أ���
�޸�ʱ�䣺2022��9��8��
��    ע��
example��  get_start_point(LCDH-2)
 */
uint8 start_point_l[2] = { 0 };//�������x��yֵ
uint8 start_point_r[2] = { 0 };//�ұ�����x��yֵ
uint8 get_start_point(uint8 start_row)
{
    uint8 i = 0,l_found = 0,r_found = 0;
    //����
    start_point_l[0] = 0;//x
    start_point_l[1] = 0;//y

    start_point_r[0] = 0;//x
    start_point_r[1] = 0;//y

    //���м�����ߣ��������
    for (i = LCDW / 2; i > border_min; i--)
    {
        start_point_l[0] = i;//x
        start_point_l[1] = start_row;//y
        if (Bin_Image[start_row][i] == 255 && Bin_Image[start_row][i - 1] == 0)
        {
            //printf("�ҵ�������image[%d][%d]\n", start_row,i);
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
            //printf("�ҵ��ұ����image[%d][%d]\n",start_row, i);
            r_found = 1;
            break;
        }
    }

    if(l_found&&r_found)return 1;
    else {
        //printf("δ�ҵ����\n");
        return 0;
    }
}

/*
������һ�������ꡣ
����˵����
break_flag_r            �������Ҫѭ���Ĵ���
(*image)[LCDW]       ����Ҫ�����ҵ��ͼ�����飬�����Ƕ�ֵͼ,�����������Ƽ���
                       �ر�ע�⣬��Ҫ�ú궨��������Ϊ����������������ݿ����޷����ݹ���
*l_stastic              ��ͳ��������ݣ����������ʼ�����Ա����ź�ȡ��ѭ������
*r_stastic              ��ͳ���ұ����ݣ����������ʼ�����Ա����ź�ȡ��ѭ������
l_start_x               �������������
l_start_y               ��������������
r_start_x               ���ұ���������
r_start_y               ���ұ����������
hightest                ��ѭ���������õ�����߸߶�
 */
#define USE_num LCDH*3   //�����ҵ�������Ա��������˵300�����ܷ��£�������Щ�������ȷʵ�Ѷ����ඨ����һ��

 //��ŵ��x��y����
uint16 points_l[(uint16)USE_num][2] = { {  0 } };//����
uint16 points_r[(uint16)USE_num][2] = { {  0 } };//����
uint16 dir_r[(uint16)USE_num] = { 0 };//�����洢�ұ���������
uint16 dir_l[(uint16)USE_num] = { 0 };//�����洢�����������
uint16 data_stastics_l = 0;//ͳ������ҵ���ĸ���
uint16 data_stastics_r = 0;//ͳ���ұ��ҵ���ĸ���
uint8 hightest = 0;//��ߵ�
void search_l_r(uint16 break_flag, uint8(*image)[LCDW], uint16 *l_stastic, uint16 *r_stastic, uint8 l_start_x, uint8 l_start_y, uint8 r_start_x, uint8 r_start_y, uint8*hightest)
{

    uint8 i = 0, j = 0;

    //��߱���
    uint8 search_filds_l[8][2] = { {  0 } };
    uint8 index_l = 0;
    uint8 temp_l[8][2] = { {  0 } };
    uint8 center_point_l[2] = {  0 };
    uint16 l_data_statics;//ͳ�����
    //����˸�����
    static sint8 seeds_l[8][2] = { {0,  1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,  0},{1, 1}, };
    //{-1,-1},{0,-1},{+1,-1},
    //{-1, 0},       {+1, 0},
    //{-1,+1},{0,+1},{+1,+1},
    //�����˳ʱ��

    //�ұ߱���
    uint8 search_filds_r[8][2] = { {  0 } };
    uint8 center_point_r[2] = { 0 };//���������
    uint8 index_r = 0;//�����±�
    uint8 temp_r[8][2] = { {  0 } };
    uint16 r_data_statics;//ͳ���ұ�
    //����˸�����
    static sint8 seeds_r[8][2] = { {0,  1},{1,1},{1,0}, {1,-1},{0,-1},{-1,-1}, {-1,  0},{-1, 1}, };
    //{-1,-1},{0,-1},{+1,-1},
    //{-1, 0},       {+1, 0},
    //{-1,+1},{0,+1},{+1,+1},
    //�������ʱ��

    l_data_statics = *l_stastic;//ͳ���ҵ��˶��ٸ��㣬��������ѵ�ȫ��������
    r_data_statics = *r_stastic;//ͳ���ҵ��˶��ٸ��㣬��������ѵ�ȫ��������

    //��һ�θ��������  ���ҵ������ֵ������
    center_point_l[0] = l_start_x;//x
    center_point_l[1] = l_start_y;//y
    center_point_r[0] = r_start_x;//x
    center_point_r[1] = r_start_y;//y

        //��������ѭ��
    while (break_flag--)
    {

        //���
        for (i = 0; i < 8; i++)//����8F����
        {
            search_filds_l[i][0] = center_point_l[0] + seeds_l[i][0];//x
            search_filds_l[i][1] = center_point_l[1] + seeds_l[i][1];//y
        }
        //�����������䵽�Ѿ��ҵ��ĵ���
        points_l[l_data_statics][0] = center_point_l[0];//x
        points_l[l_data_statics][1] = center_point_l[1];//y
        l_data_statics++;//������һ

        //�ұ�
        for (i = 0; i < 8; i++)//����8F����
        {
            search_filds_r[i][0] = center_point_r[0] + seeds_r[i][0];//x
            search_filds_r[i][1] = center_point_r[1] + seeds_r[i][1];//y
        }
        //�����������䵽�Ѿ��ҵ��ĵ���
        points_r[r_data_statics][0] = center_point_r[0];//x
        points_r[r_data_statics][1] = center_point_r[1];//y

        index_l = 0;//�����㣬��ʹ��
        for (i = 0; i < 8; i++)
        {
            temp_l[i][0] = 0;//�����㣬��ʹ��
            temp_l[i][1] = 0;//�����㣬��ʹ��
        }

        //����ж�
        for (i = 0; i < 8; i++)
        {
            if (image[search_filds_l[i][1]][search_filds_l[i][0]] == 0
                && image[search_filds_l[(i + 1) & 7][1]][search_filds_l[(i + 1) & 7][0]] == 255)
            {
                temp_l[index_l][0] = search_filds_l[(i)][0];
                temp_l[index_l][1] = search_filds_l[(i)][1];
                index_l++;
                dir_l[l_data_statics - 1] = (i);//��¼��������
            }

            if (index_l)
            {
                //���������
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
            //printf("���ν���ͬһ���㣬�˳�\n");
            break;
        }
        if (my_abs(points_r[r_data_statics][0] - points_l[l_data_statics - 1][0]) < 2
            && my_abs(points_r[r_data_statics][1] - points_l[l_data_statics - 1][1] < 2)
            )
        {
            //printf("\n���������˳�\n");
            *hightest = (points_r[r_data_statics][1] + points_l[l_data_statics - 1][1]) >> 1;//ȡ����ߵ�
            //printf("\n��y=%d���˳�\n",*hightest);
            break;
        }
        if ((points_r[r_data_statics][1] < points_l[l_data_statics - 1][1]))
        {
            continue;//�����߱��ұ߸��ˣ���ߵȴ��ұ�
        }
        if (dir_l[l_data_statics - 1] == 7
            && (points_r[r_data_statics][1] > points_l[l_data_statics - 1][1]))//��߱��ұ߸����Ѿ�����������
        {
            //printf("\n��߿�ʼ�����ˣ��ȴ��ұߣ��ȴ���... \n");
            center_point_l[0] = (uint8)points_l[l_data_statics - 1][0];//x
            center_point_l[1] = (uint8)points_l[l_data_statics - 1][1];//y
            l_data_statics--;
        }
        r_data_statics++;//������һ

        index_r = 0;//�����㣬��ʹ��
        for (i = 0; i < 8; i++)
        {
            temp_r[i][0] = 0;//�����㣬��ʹ��
            temp_r[i][1] = 0;//�����㣬��ʹ��
        }

        //�ұ��ж�
        for (i = 0; i < 8; i++)
        {
            if (image[search_filds_r[i][1]][search_filds_r[i][0]] == 0
                && image[search_filds_r[(i + 1) & 7][1]][search_filds_r[(i + 1) & 7][0]] == 255)
            {
                temp_r[index_r][0] = search_filds_r[(i)][0];
                temp_r[index_r][1] = search_filds_r[(i)][1];
                index_r++;//������һ
                dir_r[r_data_statics - 1] = (i);//��¼��������
                //printf("dir[%d]:%d\n", r_data_statics - 1, dir_r[r_data_statics - 1]);
            }
            if (index_r)
            {

                //���������
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


    //ȡ��ѭ������
    *l_stastic = l_data_statics;
    *r_stastic = r_data_statics;

}
/*
���ܣ��Ӱ�����߽�����ȡ��Ҫ�������
������total_L ���ҵ��ĵ������
*/
uint8 l_border[LCDH];//��������
uint8 r_border[LCDH];//��������
uint8 center_line[LCDH];//��������
void get_left(uint16 total_L)
{
    uint8 i = 0;
    uint16 j = 0;
    uint8 h = 0;
    //��ʼ��
    for (i = 0;i<LCDH;i++)
    {
        l_border[i] = border_min;
    }
    h = LCDH - 2;
    //���
    for (j = 0; j < total_L; j++)
    {
        if (points_l[j][1] == h)
        {
            l_border[h] = points_l[j][0]+1;
        }
        else continue; //ÿ��ֻȡһ���㣬û����һ�оͲ���¼
        h--;
        if (h == 0)
        {
            break;//�����һ���˳�
        }
    }
}
/*
���ܣ��Ӱ�����߽�����ȡ��Ҫ���ұ���
������total_R  ���ҵ��ĵ������
*/
void get_right(uint16 total_R)
{
    uint8 i = 0;
    uint16 j = 0;
    uint8 h = 0;
    for (i = 0; i < LCDH; i++)
    {
        r_border[i] = border_max;//�ұ��߳�ʼ���ŵ����ұߣ�����߷ŵ�����ߣ�����������պ�����������߾ͻ����м䣬������ŵõ�������
    }
    h = LCDH - 2;
    //�ұ�
    for (j = 0; j < total_R; j++)
    {
        if (points_r[j][1] == h)
        {
            r_border[h] = points_r[j][0] - 1;
        }
        else continue;//ÿ��ֻȡһ���㣬û����һ�оͲ���¼
        h--;
        if (h == 0)break;//�����һ���˳�
    }
}


//�˲�����
//�������ͺ͸�ʴ����ֵ����
#define threshold_max   255*5//�˲����ɸ����Լ����������
#define threshold_min   255*2//�˲����ɸ����Լ����������
void image_filter(uint8(*Bin_Image)[LCDW])//��̬ѧ�˲�
{
    uint16 i, j;
    uint32 num = 0;


    for (i = 1; i < LCDH - 1; i++)
    {
        for (j = 1; j < (LCDW - 1); j++)
        {
            //ͳ�ư˸����������ֵ
            num =
                Bin_Image[i - 1][j - 1] + Bin_Image[i - 1][j] + Bin_Image[i - 1][j + 1]
                + Bin_Image[i][j - 1] + Bin_Image[i][j + 1]
                + Bin_Image[i + 1][j - 1] + Bin_Image[i + 1][j] + Bin_Image[i + 1][j + 1];


            if (num >= threshold_max && Bin_Image[i][j] == 0)
            {

                Bin_Image[i][j] = 255;//��

            }
            if (num <= threshold_min && Bin_Image[i][j] == 255)
            {

                Bin_Image[i][j] = 0;//��

            }

        }
    }

}

//��ͼ��һ���ڿ�
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
    int16_t mid_line_error = 0;      //������֮���е�ĺ��������
    uint8_t mid_diuxian_flag = 0;    //�G�߱�־λ
    uint16_t weight_midline_sum = 0;

    for (uint8_t i = LCDH-2 ; i>38 ; i--)
    {
        if (i == LCDH-2)
        {
            weight_midline_sum += center_line[i] * weight_list[(LCDH-2)-i];  // ����
            weight_sum += weight_list[(LCDH-2)-i];                           // ��ĸ
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
* @brief ��С���˷�
* @param uint8 begin                �������
* @param uint8 end                  �����յ�
* @param uint8 *border              ������Ҫ����б�ʵı߽��׵�ַ
*  @see CTest       Slope_Calculate(start, end, border);//б��
* @return ����˵��
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
    if ((end - begin)*x2sum - xsum * xsum) //�жϳ����Ƿ�Ϊ��
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
* @brief ����б�ʽؾ�
* @param uint8 start                �������
* @param uint8 end                  �����յ�
* @param uint8 *border              ������Ҫ����б�ʵı߽�
* @param float *slope_rate          ����б�ʵ�ַ
* @param float *intercept           ����ؾ��ַ
*  @see CTest       calculate_s_i(start, end, r_border, &slope_l_rate, &intercept_l);
* @return ����˵��
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

    //�������ƽ����
    if (num)
    {
        x_average = (float)(xsum / num);
        y_average = (float)(ysum / num);

    }

    /*����б��*/
    *slope_rate = Slope_Calculate(start, end, border);//б��
    *intercept = y_average - (*slope_rate)*x_average;//�ؾ�
}

/**
* @brief ʮ�ֲ��ߺ���
* @param uint8(*image)[image_w]     �����ֵͼ��
* @param uint8 *l_border            ������߽��׵�ַ
* @param uint8 *r_border            �����ұ߽��׵�ַ
* @param uint16 total_num_l         �������ѭ���ܴ���
* @param uint16 total_num_r         �����ұ�ѭ���ܴ���
* @param uint16 *dir_l              ����������������׵�ַ
* @param uint16 *dir_r              �����ұ����������׵�ַ
* @param uint16(*points_l)[2]       ������������׵�ַ
* @param uint16(*points_r)[2]       �����ұ������׵�ַ
*  @see CTest       cross_fill(image,l_border, r_border, data_statics_l, data_statics_r, dir_l, dir_r, points_l, points_r);
* @return ����˵��
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
    //��ʮ��
    for (i = 1; i < total_num_l; i++)
    {
        if (dir_l[i - 1] == 4 && dir_l[i] == 4 && dir_l[i + 3] == 6 && dir_l[i + 5] == 6 && dir_l[i + 7] == 6)
        {
            break_num_l = (uint8)points_l[i][1];//����y����
//            printf("brea_knum-L:%d\n", break_num_l);
//            printf("I:%d\n", i);
//            printf("ʮ�ֱ�־λ��1\n");
            break;
        }
    }
    for (i = 1; i < total_num_r; i++)
    {
        if (dir_r[i - 1] == 4 && dir_r[i] == 4 && dir_r[i + 3] == 6 && dir_r[i + 5] == 6 && dir_r[i + 7] == 6)
        {
            break_num_r = (uint8)points_r[i][1];//����y����
//            printf("brea_knum-R:%d\n", break_num_r);
//            printf("I:%d\n", i);
//            printf("ʮ�ֱ�־λ��1\n");
            break;
        }
    }
    if (break_num_l&&break_num_r&&image[LCDH - 25][4] && image[LCDH - 25][LCDW - 4])//�����������򶼷�������
    {
        //����б��
        start = break_num_l - 15;
        start = (uint8)limit_a_b(start, 0, LCDH);
        end = break_num_l - 5;
        calculate_s_i(start, end, l_border, &slope_l_rate, &intercept_l);
//        printf("slope_l_rate:%d\nintercept_l:%d\n", slope_l_rate, intercept_l);
        for (i = break_num_l - 5; i < LCDH - 1; i++)
        {
            l_border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
            l_border[i] = (uint8)limit_a_b(l_border[i], border_min, border_max);//�޷�
        }

        //����б��
        start = break_num_r - 15;//���
        start = (uint8)limit_a_b(start, 0, LCDH);//�޷�
        end = break_num_r - 5;//�յ�
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
                        break_numl_y = (uint8)points_l[i][1];//����y����
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
                l_border[i] = (uint8)limit_a_b(l_border[i], border_min, border_max);//�޷�
            }
        }
        if (stepl == 2 && break_numl_y)
        {
            slope_r_rate = 1.0*(break_numl_x-97)/(break_numl_y-78);
            intercept_r = break_numl_x - slope_r_rate*break_numl_y;
            for (int i = break_numl_y; i<LCDH-1; i++)
            {
                r_border[i] = slope_r_rate * (i)+intercept_r;//y = kx+b
                r_border[i] = (uint8)limit_a_b(r_border[i], border_min, border_max);//�޷�
            }
        }

        if (stepl==3 && break_numr_y)
           {
                slope_r_rate = 1.0*(break_numr_x-20)/(break_numr_y-30);
                intercept_r = break_numr_x - slope_r_rate*break_numr_y;
                for (int i = break_numr_y; i>30; i--)
                {
                    r_border[i] = slope_r_rate * (i)+intercept_r;;//y = kx+b
                    r_border[i] = (uint8)limit_a_b(r_border[i], border_min, border_max);//�޷�
                }
           }
        if (stepl==4 && break_numl_y)
            {
                slope_l_rate = 1.0*(break_numl_x-0)/(break_numl_y-80);
                intercept_l = break_numl_x - slope_l_rate*break_numl_y;
                for (int i = break_numl_y; i<LCDH-1; i++)
                {
                    l_border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    l_border[i] = (uint8)limit_a_b(l_border[i], border_min, border_max);//�޷�
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

unsigned char left_line_list[LCDH];   //�����
unsigned char right_line_list[LCDH];  //�ұ���
unsigned char midline[LCDH];          //����
uint8_t mid_line=0;                   //���ļ���ļ�Ȩƽ���������ߵĺ�����
uint8 OpenFlag,BarrierFlag=0,Process_flag=0;         //�ϰ�ʶ���־λ
unsigned char Miss=0,Crossroadflag=0,ringflag=0; //ʮ��ʶ���־λ
unsigned char garageout_flag=0;       //����ʶ���־λ
uint16 edgeL[LCDH];
uint16 edgeR[LCDH];
float error_Camera;
uint8 WHITE=0;
int  C_R_Flag[6]={0,0,1,1,1,0},Num=0;
char txt1[32];

float slope;//����б��
unsigned char flag_centre_right_point;//�ҹյ��־λ
unsigned char flag_centre_left_point;//��յ��־λ
//static uint8 meetRingFlag=0;//������־λ����һ��A�ֱ�־�㡪����һ��V�ֱ�־��
//static uint8 enterRingFlag=0;//�뻷��־λ����һ��V�ֱ�־�㡪���ڶ���A�ֱ�־��
//static uint8 leaveRingFlag=0;//������־λ���ڶ���A�ֱ�־�㡪���ڶ���V�ֱ�־��
//static uint8 passRingFlag=1;//������־λ���ڶ���V�ֱ�־�㡪����һ��A�ֱ�־��
//static uint8 ringSide=0;//�������ͣ�1��ʾ��2��ʾ��

int jumpFlagL=0;
int jumpFlagR=0,lefty[2],righty[2],l_down_flag=0,r_down_flag=0,r_up_flag=0,l_up_flag=0,flagl=0,flagr=0;
float parameterB,parameterA;
uint16 pointLX=0,pointLUX=0;//����������X����
uint16 pointLY=0,pointLUY=0;//����������Y����
uint16 pointRX=0,pointRUX=0;//�ұ��������X����
uint16 pointRY=0,pointRUY=0;//�ұ��������Y����
uint16 pointX=0;//�����X����
uint16 pointY=0;//�����Y����

uint8 pointSide=0;//����㷽��1��ʾ��2��ʾ��
uint8 pointType=0;//�������࣬1��ʾA������㣬2��ʾV�������

void Image(void)
{
    if (Camera_Flag == 2)
    {
        Get_Use_Image();              // ��ȡ����ʹ�õ�����
        Camera_Flag = 0;              // �������ͷ�ɼ���ɱ�־λ  �����������򲻻��ٴβɼ�����
    }
}


//Ѱ�ұ���Ѱ����������
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

        //�������֮һ��Ϊ�׵㣬�Ӷ���֮һ����
        if (index[i][LCDW/2] == 1)
        {
            //Ѱ����ߵ�
            for (unsigned char j = LCDW/2-1 ; j > 0 ; j--)
            {
                if (index[i][j] == 0 && index[i][j+1] == 1)
                {
                    left_point = j;
                    break;
                }
            }
            //Ѱ���ұߵ�
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
            //Ѱ����ߵ�
            for (unsigned char j = LCDW/4-1 ; j > 0 ; j--)
            {
                if (index[i][j] == 0 && index[i][j+1] == 1)
                {
                    left_point = j;
                    break;
                }
            }
            //Ѱ���ұߵ�
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
            //Ѱ����ߵ�
            for (unsigned char j = LCDW/4*3-1 ; j > 0 ; j--)
            {
                if (index[i][j] == 0 && index[i][j+1] == 1)
                {
                    left_point = j;
                    break;
                }
            }
            //Ѱ���ұߵ�
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


//�Ź��񻭵�
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

//���Ʊ���
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
    int16_t mid_line_error = 0;      //������֮���е�ĺ��������
    uint8_t mid_diuxian_flag = 0;    //�G�߱�־λ
    uint16_t weight_midline_sum = 0;

    for (uint8_t i = LCDH-1 ; i>49 ; i--)
    {
        if (i == LCDH-1)
        {
            weight_midline_sum += midline[i] * mid_meight_list[79-i];  // ����
            weight_sum += mid_meight_list[79-i];                              // ��ĸ
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

           midline[i]=(edgeL[i]+edgeR[i])/2;//���ǻ������ߺ��ܱ��жϱ��߱߼������ߣ��������ֻ������һ��Ѱ�ұ߽�

           if(i==0){
             break;
           }
         }

         float stepLength;//�������ֵ����
         //�Ĳ��������������뻷������������

//         uint16 pointLLCX=0;
//         uint16 pointLLCY=LCDH-1;
//         uint16 pointLRCX=LCDW-1;
//         uint16 pointLRCY=LCDH-1;
//         uint16 pointTMX=0;
//         uint16 pointTMY=0;
         /*----�������������￪ʼ----*/




         for(i=10;i<LCDH-1;i++){//ֻ�ж���2/3����ͼ��������
           //���A�������
           if(edgeL[i]-edgeL[i-1]>5&&edgeL[i-5]==0){
             jumpFlagL=1;
             pointLX=edgeL[i];
             pointLY=i;
             pointSide=1;
             pointType=1;
           }
           //���V�������
           else if(edgeL[i-1]-edgeL[i]>10&&edgeL[i+5]==0){
             jumpFlagL=2;
             pointLUX=edgeL[i-1];
             pointLUY=i-1;
             pointSide=1;
             pointType=2;
           }
           //�Ҳ�A�������
           if(edgeR[i-1]-edgeR[i]>5&&edgeR[i-5]==LCDW-1){
             jumpFlagR=1;
             pointRX=edgeR[i];
             pointRY=i;
             pointSide=2;
             pointType=1;
           }
           //�Ҳ�V�������
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

//         uint8 jumpFlag=(jumpFlagL^jumpFlagR)&&((pointLY>LCDH/2)||(pointRY>LCDH/2));//��ֹʮ������
//         if((pointLX==LCDW/2)&&(pointRX==LCDW/2)&&(pointLY>LCDH/2)&&(pointRY>LCDH/2)){
//           jumpFlag=1;//�뻷V���������
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
//         if(jumpFlag){//������Ч�����
//           if(pointType==1){//��ΪA������㣬˵������������
//             if(passRingFlag==1){//����//�������жϷ�����
//               meetRingFlag=1;
//               passRingFlag=0;
//             }
//             else if(enterRingFlag==1){//����
//               leaveRingFlag=1;
//               enterRingFlag=0;
//             }
//           }
//           else if(pointType==2){//��ΪV������㣬˵���뻷�����
//             if(meetRingFlag==1){//�뻷
//               enterRingFlag=1;
//               meetRingFlag=0;
//             }
//             else if(leaveRingFlag==1){//����
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
//             //�뻷����
//             //�뻷��־�㣨pointAX��pointAY����ͼ�����½ǣ�pointLLCX��pointLLCY����ͼ�����½ǣ�pointLRCX��pointLRCY��
//             if(ringSide==1){//�뻷��־������ߣ������½ǵ㵽�뻷��־�����ߣ����ұ���
//               stepLength=(float)(pointLRCX-pointX)/(float)(pointLRCY-pointY);
//               for(i=0;i<pointLRCY-pointY;i++){
//                 edgeR[pointLRCY-i]=pointLRCX-(int)(i*stepLength);
//                 Bin_Image[pointLRCY-i][edgeR[pointLRCY-i]]=0;
//               }
//             }
//             else if(ringSide==2){//�뻷��־�����ұߣ������½ǵ㵽�뻷��־�����ߣ��������
//               stepLength=(float)(pointX-pointLLCX)/(float)(pointLLCY-pointY);
//               for(i=0;i<pointLLCY-pointY;i++){
//                 edgeL[pointLLCY-i]=pointLLCX+(int)(i*stepLength);
//                 Bin_Image[pointLLCY-i][edgeL[pointLLCY-i]]=0;
//               }
//             }
//           }
//         }
//         else if(leaveRingFlag==1){
//           if(ringSide==1){//����Բ��
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
//         /*----�����������������----*/

         for(i=0;i<LCDH;i++){//���¼�������
           midline[i]=(edgeL[i]+edgeR[i])/2;
           Bin_Image[i][midline[i]]=Bin_Image[i][midline[i]-1]=Bin_Image[i][midline[i]+1]=0;//����
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

/************************************���Իع��������б��************************************/
// y = Ax+B
void regression(int startline,int endline)
{

    int i=0,SumX=0,SumY=0,SumLines = 0;
    float SumUp=0,SumDown=0,avrX=0,avrY=0;
    SumLines=endline-startline;   // startline Ϊ��ʼ�У� //endline ������ //SumLines

    for(i=startline;i<endline;i++)
    {
        SumX+=i;
        SumY+=midline[i];    //����Middle_blackΪ������ߵ�����
    }
    avrX=(float)(SumX/SumLines);     //X��ƽ��ֵ
    avrY=(float)(SumY/SumLines);     //Y��ƽ��ֵ
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
    parameterA=(SumY-parameterB*SumX)/SumLines;  //�ؾ�
//    return parameterB;  //����б��
}

//S���ʶ����S��СS�䣩
//������Ѱ��������յ���ҹյ����ʼ�к��յ���
void find_guaidian(int start_line,int end_line)
{
    int i = 0;
    uint8_t centre_left_point[2];
    uint8_t centre_right_point[2];
    //�ӳ���־λ,�����һ������ɨ�������߹յ����ӳ���־λ����������ҹս���ֱ��
    int S_wandao_T;
    //��־λ����
    flag_centre_right_point = 0;
    flag_centre_left_point = 0;
    //�������
    centre_left_point[0] = 0;
    centre_left_point[1] = 0;
    centre_right_point[0] = 0;
    centre_right_point[1] = 0;
    //��б��ȡ����ֵ
    if(slope<0)
    {
        slope=-(slope);
    }
    //���������ң�80�е�20��
    for (i = end_line; i>=start_line; i--)
    {
        //�����ҹյ�,��б�ʺ͹յ��������丨���жϣ��������м���
     if (midline[i-1]-midline[i]>=1&&midline[i+1]-midline[i]>=1&&
             midline[i+5]-midline[i]>=5&&midline[i-5]-midline[i]>=5
        && midline[i] < 50 && midline[i] > 0 && slope >= 0.2)
        {
         //�ҵ��ҹյ�󣬼�¼�ҹյ����꣬���ұ�־λ��1
            centre_right_point[0] = (unsigned char)i;//y
            centre_right_point[1] = midline[i];//x
            flag_centre_right_point = 1;
        }
        //������յ�
     if (midline[i]-midline[i-1]>=1&&midline[i]-midline[i+1]>=1&&
             midline[i]-midline[i+5]>=5&&midline[i]-midline[i-5]>=5
        && midline[i] > 50 && midline[i] < 100 && slope>= 0.2)
        {
            centre_left_point[0] = (unsigned char)i;//y
            centre_left_point[1] = midline[i];//x
            flag_centre_left_point = 1;
            //�ӳ���־λ��1
            S_wandao_T=1;
        }
    }
    //������S�����ʱ�����ӳ���־λ�ӳ�һ�³�S���ʱ�䣬��ȥ��ʱ��ֻ��ɨ���ҹյ㣬ɨ������յ�,
    //�ӳ�S�����־λ��������ٹ�С�ղ���ȥ
    if(S_wandao_T==1&&flag_centre_right_point==1)
    {
        //����յ��趨Ϊɨ�ߵ�һ�е���������
        centre_left_point[0] = 79;//y
        centre_left_point[1] = midline[79];//x
        flag_centre_left_point = 1;

    }

     int Middle_S_flag = 0;//��S���־λ
     int flag_small_S = 0;//СS���־λ
     //����ҵ���յ���ҹյ��ж��������
    if (flag_centre_left_point == 1 && flag_centre_right_point == 1 )
    {
        //��������յ㶼�ҵ���������յ���ҹյ�Ĳ�ֵ����һ����Χ����ô��Ϊ����СS�����������S
        if ((abs_int16(centre_left_point[0] - centre_right_point[0]) >= 8)
                && (abs_int16(centre_left_point[0] - centre_right_point[0]) <= 28)
           && abs_int16(centre_left_point[1] - centre_right_point[1]) >= 8
           && abs_int16(centre_left_point[1] - centre_right_point[1]) <= 35)
        {
            flag_small_S = 1;//СS��־λ
        }
        else
        {
            Middle_S_flag = 1;//��S��־λ
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

        for(int i=LCDH*1/3;i<LCDH-1;i++)//ֻ�ж���2/3����ͼ��������
        {
            if(Num==2||Num==4)
            {
                 //���A�������
                 if(left_line_list[i]-left_line_list[i-1]>5&&left_line_list[i-5]==0&&i>50){
                   ringflag=1;
                 }
                 //���V�������
                 else if(left_line_list[i-1]-left_line_list[i]>5&&left_line_list[i+5]==0){
                   ringflag=2;
                 }
            }
            else
            {
                 //�Ҳ�A�������
                 if(right_line_list[i-1]-right_line_list[i]>5&&right_line_list[i-5]==LCDW-1&&i>50){
                   ringflag=3;
                 }
                 //�Ҳ�V�������
                 else if(right_line_list[i]-right_line_list[i-1]>5&&right_line_list[i+5]==LCDW-1){
                   ringflag=4;
                 }
            }
        }

    }
}




//���ֱ�ߺ���
void line(int type, int startline, int endline, float parameterB, float parameterA)
        {
            if (type == 1) //��
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
            else if (type == 2)            //��
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
            else if (type == 0)             //��
            {
                for (int i = startline; i < endline; i++)
                {
                    midline[i] =  ((left_line_list[i] / 2 + right_line_list[i] / 2));
                }
            }
        }


void buxian (void){

// �ҹյ�
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
// ��ʮ��ǰ����* regression����С���˷���б�ʽؾ� *run�����ֱ�ߺ��� 1Ϊ�� 2Ϊ�� 0Ϊ��
//abu�Ƕ���ɨ�ߵļ���λ�����Ⱥ���
if (((lefty[0] > 0 && lefty[1] > 0) || (righty[0] > 0 && righty[1] > 0)))
         {

             if (lefty[0] >= 2 && lefty[1] >= 2)                                    //������              ��ʮ��ǰ
             {
                 regression(lefty[0], lefty[1]);
                 line(1, lefty[0], lefty[1], parameterB, parameterA);
                 flagl = 1;                                                                                      //
             }
             if (righty[0] >= 2 && righty[1] >= 2 )                                         //������
             {
                 regression(righty[0] ,righty[1]);
                 line(2, righty[0], righty[1], parameterB, parameterA);
                 flagr = 1;
             }
             if ((r_down_flag == 1 || l_down_flag == 1) )
             {
                 if (flagl == 1 && flagr == 0)
                 {
                     line(0, lefty[0], lefty[1], parameterB, parameterA);                 //�������
//                     if (abu >= 3)
//                     {
//                         run(0, 0, lefty[0], parameterB, parameterA);
//                         run(0, lefty[1], 50, parameterB, parameterA);
//                     }
                 }
                 else
                 {
                     line(0, righty[0], righty[1], parameterB, parameterA);                 //�������
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
    uint8 hightest = 0;//����һ������У�tip����������ָ����yֵ����С

    //�����ж�����ͷ���ݲɼ���ɱ�־λ
    if (Camera_Flag==0)
    {
        Get_Bin_Image(0);// ��ֵ��
        image_filter(Bin_Image);//�˲�
        image_draw_rectan(Bin_Image);//Ԥ����

        data_stastics_l = 0;//����
        data_stastics_r = 0;
        if (get_start_point(LCDH - 2))//�ҵ�����ˣ���ִ�а�����û�ҵ���һֱ��
        {
            search_l_r((uint16)USE_num, Bin_Image, &data_stastics_l, &data_stastics_r, start_point_l[0], start_point_l[1], start_point_r[0], start_point_r[1], &hightest);
            //����ȡ�ı߽�������ȡ����
            get_left(data_stastics_l);
            get_right(data_stastics_r);
            //������
            //cross_fill(Bin_Image,l_border, r_border, data_stastics_l, data_stastics_r, dir_l, dir_r, points_l, points_r);
            Round_fill(Bin_Image,l_border, r_border, data_stastics_l, data_stastics_r, dir_l, dir_r, points_l, points_r);
         }
        TFTSPI_BinRoad(0, 0, LCDH, LCDW, (unsigned char *) Bin_Image);
        //��������ѭ�����������߽��
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
                 center_line[i] = (l_border[i] + r_border[i]) >> 1;//������
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
        Get_Bin_Image(0);             // ��ֵ��
        Bin_Image_Filter();           // �˲������汻Χ�����ݽ����޸�Ϊͬһ��ֵ
//        ImageProcess();
        find_mid_line(Bin_Image);     // Ѱ�ұ��ߺ���������
//        barrier_judge();              // �ϰ�����·ʶ��
//        garage_judge();               // ����ʶ��
//        Corss_Ring();                 // ������ʮ���ж�
        find_mid_line_weight();       // ������ֵ
        draw_line();                  // ����
    }
    TFTSPI_BinRoad(0, 0, LCDH, LCDW, (unsigned char *)Bin_Image); // ��ʾ����ͷͼ��
    TFTSPI_P8X16Str(1,5,txt1,u16YELLOW, u16BLUE);
    LED_Ctrl(LED0, RVS);
}

