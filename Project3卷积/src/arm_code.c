#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include <arm_neon.h>
#include <sys/time.h> //引入头文件

#define byte unsigned char

int channel = 3; // 默认RGB三通道输入

// 定义颜色表
typedef struct _RGBQUAD
{
    // public:
    byte rgbBlue;     // 蓝色分量
    byte rgbGreen;    // 绿色分量
    byte rgbRed;      // 红色分量
    byte rgbReserved; // 保留值,必须为0
} RGBQUAD;

// ImgData = (row,column,channel) example:1920x1080x3
typedef struct image
{
    int row;
    int col;
    int channel;
    float *ImgData;
} image;

// Data = (size,row,column)
typedef struct coved_data
{
    int row;
    int col;
    int size;
    float *Data;
} coved_data;

// dimension of kernel: (size, row, column, channel)
typedef struct kernel
{
    int row;
    int col;
    int size;
    int channel;
    float *Data;
} kernel;

image initialImage(image image, const char *filename)
{                                     // 这里也许可以用内存一致连续优化
    FILE *fp = fopen(filename, "rb"); // 打开文件。
    if (fp == NULL)
    { // 打开文件失败
        printf("fp is null!");
        image.row = 0; // 若打开失败，则令图像的行数为0，在主程序中判断行数是否为0来判断image初始化是否失败
        return image;
    }
    // 得到图片长和宽，这里图片是一个像素3byte，1980x1080x3应该是
    int row, col;
    fseek(fp, 18, SEEK_CUR);
    fread(&col, sizeof(int), 1, fp);
    fread(&row, sizeof(int), 1, fp);
    image.row = row;
    image.col = col;
    image.channel = 3;
    printf("row=%d\ncol=%d\n", row, col);
    // 得到图片大小 = 54(定义文件) + 1920x1080x3 = 6220854
    fseek(fp, 0, SEEK_END); // 到文档末尾
    int len = ftell(fp);
    printf("len = %d\n", len);
    byte *data = malloc(sizeof(byte) * (len - 54)); // data中包含填充的0数据
    int gap = ((len - col * row * 3 - 54) / row);   // 计算图片自动填充0的个数
    printf("gap=%d\n", gap);
    fseek(fp, 0, SEEK_SET);         // 先把指针指回0
    fseek(fp, 54, SEEK_CUR);        // 再加上54偏移，到达数据位
    fread(data, 1, (len - 54), fp); // 把图片里的数据全部导入data,len-54的数据量包含补0,3*row*col不包含补0
    // 将填充0前的数据导出
    byte *real_data = malloc(sizeof(byte) * row * col * 3);
    int move = 0;
    for (int i = 0; i < len - 54; i++)
    {
        if ((i - move) % (3 * col) == (3 * col - 1))
        {
            real_data[i - move] = data[i];
            // printf("1real_data[%d] = data[%d] = %d\n",i-move,i,data[i]);  //这里是对的！
            move = move + gap;
            i = i + gap;
            continue;
        }
        real_data[i - move] = data[i];
        // printf("2real_data[%d] = data[%d] = %d\n",i-move,i,data[i]);
    }
    image.ImgData = (float *)malloc(row * col * 3 * sizeof(float)); // 分配row*col个空间
    for (int i = 0; i < row * col * 3; i++)
    {
        image.ImgData[i] = (float)real_data[i];
    }

    fclose(fp); // 清除文件指针
    free(data);
    free(real_data);
    return image;
}

kernel initialKernel(kernel kernel, int size, int row, int col)
{
    kernel.row = row;
    kernel.col = col;
    kernel.size = size;
    kernel.channel = 3;
    kernel.Data = (float *)malloc(kernel.size * kernel.row * kernel.col * kernel.channel * sizeof(float)); // 分配kernel的个数个空间
    for (int i = 0; i < kernel.size * kernel.row * kernel.col * kernel.channel; i++)
    {
        kernel.Data[i] = 0.01f;
        // printf("kernel.Data[%d] = %f\n",i,kernel.Data[i]);
    }
    return kernel;
}

// 卷积操作
coved_data cov(int stride_x, int stride_y, int padding_x, int padding_y, kernel kernel, image image)
{
    coved_data data;
    data.row = (image.row + 2 * padding_x - kernel.row + 1) / stride_x;
    data.col = (image.col + 2 * padding_y - kernel.col + 1) / stride_y;
    data.size = kernel.size;
    unsigned int len = kernel.size * data.row * data.col * kernel.row * kernel.col * 3;
    float *p1 = (float *)malloc(len * sizeof(float));
    float *p2 = (float *)malloc(len * sizeof(float));
    float *p3 = (float *)malloc(len * sizeof(float));
    unsigned int position = 0; // 用于记录指针当前位置
    // printf("multiply times = %d,data.row = %d, data.col = %d\n",len,data.row,data.col);
    data.Data = (float *)malloc(data.size * sizeof(float) * data.row * data.col);
    int x, y, xx, yy; // 只是用于避免重复运算
    for (int s = 0; s < kernel.size; s++)
    { // 每一个kernel循环
        // 遍历coved_data里的每一位数据
        for (int i = 0; i < data.row; i++)
        {
            for (int j = 0; j < data.col; j++)
            {
                // 计算coved_data中每一位的数据，具体方式为对应位置相乘求和
                for (int ii = 0; ii < kernel.row; ii++)
                {
                    x = -padding_x + stride_x * i + ii;
                    for (int jj = 0; jj < kernel.col; jj++)
                    {
                        y = -padding_y + stride_y * j + jj;
                        // printf("imagedata[%d][%d],kernedata[%d][%d][%d]\n", x, y, s, ii, jj);
                        if (x < 0 || x >= image.row || y < 0 || y >= image.col) // 判断是否越界
                        {
                            // printf(":continue\n");
                            p1[position] = 0;
                            p1[position + 1] = 0;
                            p1[position + 2] = 0;
                            p2[position] = 0;
                            p2[position + 1] = 0;
                            p2[position + 2] = 0;
                            position += 3;
                            // printf("1p1[%d] = %f\n",position, p1[position] + p1[position + 1] + p1[position + 2]);
                            continue; // 判断数字是否取padding值，若是padding则0
                        }
                        xx = image.col * 3 * x + 3 * y;
                        yy = s * kernel.row * kernel.col * 3 + ii * kernel.col * 3 + jj * 3;
                        p1[position] = image.ImgData[xx];
                        p1[position + 1] = image.ImgData[xx + 1];
                        p1[position + 2] = image.ImgData[xx + 2];
                        p2[position] = kernel.Data[yy];
                        p2[position + 1] = kernel.Data[yy + 1];
                        p2[position + 2] = kernel.Data[yy + 2];
                        // printf("2p1[%d] = %f\n",position, p1[position] + p1[position + 1] + p1[position + 2]);
                        position += 3;
                    }
                }
            }
        }
    }
    int e = len % 4;
    float32x4_t a, b, c;
#pragma omp parallel for
    for (int i = 0; i < len - e; i += 4)
    {
        a = vld1q_f32(p1 + i);
        b = vld1q_f32(p2 + i);
        c = vmulq_f32(a, b);
        vst1q_f32(p3 + i, c);
    }
    for (int i = 1; i <= e; i++)
    {
        p3[len - e] = p1[len - e] * p2[len - e];
    }
    // 将p3指针划分为三块，方便后续累加
    unsigned int len_cut = len / 3;
    // printf("len_cut = %d\n",len_cut);
    unsigned int ee = len_cut % 4;
    // printf("ee = %d\n",ee);
    float *p30 = (float *)malloc((len_cut) * sizeof(float));
    float *p31 = (float *)malloc((len_cut) * sizeof(float));
    float *p32 = (float *)malloc((len_cut) * sizeof(float));
    float *sum = (float *)malloc((len_cut) * sizeof(float)); // sum是kernel里一个元素和image里一个元素的向量乘积
    position = 0;
    for (int i = 0; i < len_cut; i++)
    {
        p30[i] = p3[3 * i];
        p31[i] = p3[3 * i + 1];
        p32[i] = p3[3 * i + 2];
        // printf("one_cut = %f\n",p30[i]+p31[i]+p32[i]);
    }
    float32x4_t aa, bb, cc, m ,n ;
    #pragma omp parallel for
    for (int i = 0; i < len_cut - ee; i += 4)
    {
        aa = vld1q_f32(p30 + i);
        bb = vld1q_f32(p31 + i);
        cc = vld1q_f32(p32 + i);
        m = vaddq_f32(aa, bb);
        n = vaddq_f32(m, cc);
        vst1q_f32(sum + i, n);
    }
    for (int i = 1; i <= ee; i++)
    {
        sum[len_cut - ee] = p30[len_cut - ee] + p31[len_cut - ee] + p32[len_cut - ee];
    }

    // 现在需要将每一次卷积的向量乘法结果再求和
    unsigned int pointnum = kernel.col * kernel.row; // 每一步卷积运算需要指针的数量
    unsigned int movenum = len_cut / pointnum;       // 卷积运算需要移动的次数，相当于data.col*data.row
    // 根据空间声明point和finaldata的内存
    float *finaldata = (float *)malloc(sizeof(float) * movenum);
    float **point = (float **)malloc(sizeof(float) * pointnum);
    for (int i = 0; i < pointnum; i++)
    {
        point[i] = (float *)malloc(sizeof(float) * movenum);
        for (int j = 0; j < movenum; j++)
        {
            point[i][j] = sum[pointnum * j + i]; // point[i]保存了原矩阵某个点的全部数据
        }
    }
    unsigned int eee = movenum % 4;
    // float *finaldata = (float *)malloc(sizeof(float) * movenum);
    for (int i = 0; i < movenum - eee; i += 4)
    {
        float32x4_t cc;  //重定义以清零
        #pragma omp parallel for
        for (int j = 0; j < pointnum; j++)
        {
            aa = vld1q_f32(point[j] + i);
            cc = vaddq_f32(cc, aa);
        }
        vst1q_f32(finaldata + i, cc);
    }
    for (int i = 1; i <= eee; i++)
    {
        finaldata[movenum - eee] = 0;
        for (int j = 0; j < pointnum; j++)
        {
            finaldata[movenum - eee] += point[j][movenum - eee];
        }
    }
    // 将finaldata的数据存入data中
    for (int i = 0; i < movenum; i++)
    {
        data.Data[i] = finaldata[i];
        // printf("finalData[%d] = %f\n",i,finaldata[i]);
    }
    // 释放内存
    for (int i = 0; i < pointnum; i++)
    {
        free(point[i]);
    }
    free(p1);
    free(p2);
    free(p3);
    free(p30);
    free(p31);
    free(p32);
    free(sum);
    free(finaldata);
    free(point);
    return data;
}

// 输出第一张图片，以验证结果
void WriteBMP(coved_data img, const char *filename)
{
    int row = img.row;
    // 每一行需要考虑补0，让每一行是四字节的整数倍
    int gap = (img.col % 4 == 0) ? 0 : 4 - (img.col % 4);
    int col = img.col + gap;
    printf("img.row = %d, img.col = %d,gap = %d\n", img.row, img.col, gap);
    printf("row = %d, col = %d\n", row, col);
    int bmi[] = {
        // col*row+54,0,54,40,col,row,1|3*8<<16,0,l*h,0,0,100,0};
        col * row + 54 + 1024, 0, 54, 40, img.col, row, 1 | 8 << 16, 0, col * row, 0, 0, 256, 0};
    // 定义颜色表
    RGBQUAD rgbquad[256];
    for (int p = 0; p < 256; p++)
    {
        rgbquad[p].rgbBlue = (byte)p;
        rgbquad[p].rgbGreen = (byte)p;
        rgbquad[p].rgbRed = (byte)p;
        rgbquad[p].rgbReserved = (byte)0;
    }
    FILE *fp = fopen(filename, "wb");
    fprintf(fp, "BM");
    fwrite(&bmi, 52, 1, fp);
    fwrite(&rgbquad, 4, 256, fp);
    byte *array = (byte *)malloc(col * row * sizeof(byte));
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            if (j >= col - gap)
            {
                array[i * col + j] = (byte)0;
                // printf("2array[%d]=%d\n",i*col+j,array[i*col+j]);
                continue;
            }
            // 这里输出第一个kernel卷积的结果，用于验证结果是否正确
            else
                array[i * col + j] = (byte)img.Data[j + img.col * i];
            // printf("1array[%d]=%f\n",i*col+j,img.Data[j+img.col*i]);
        }
    }
    fwrite(array, 1, row * col, fp);
    free(array);
    fclose(fp);
}

int main()
{
    // 初始化图像
    struct timeval t1, t2;
    double timeuse;
    image image;
    image = initialImage(image, "input_image.bmp");
    printf("image initialized\n");
    kernel kernel;
    // kernel(kernel,size,row,col)
    kernel = initialKernel(kernel, 1, 5, 5);
    printf("kernel initialized\n");
    coved_data data;
    printf("begin time\n");
    gettimeofday(&t1, NULL);
    // 输入为(stride_x,stride_y,padding_x,padding_y,kernel,image)
    data = cov(1, 1, 0, 0, kernel, image);
    gettimeofday(&t2, NULL);
    timeuse = (t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec) / 1000000.0;
    printf("timeuse:%lf\n", timeuse);
    WriteBMP(data, "output_image.bmp");
    free(image.ImgData);
    free(kernel.Data);
    return 0;
}
