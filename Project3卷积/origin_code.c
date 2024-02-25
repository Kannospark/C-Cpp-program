#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include <immintrin.h>

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
    float ***ImgData;
} image;

// dimension of kernel: (size, row, column, channel)
typedef struct kernel
{
    int row;
    int col;
    int size;
    int channel;
    float ****Data;
} kernel;

// Data = (size,row,column)
typedef struct coved_data
{
    int row;
    int col;
    int size;
    float ***Data;
} coved_data;

image initialImage(image image,const char *filename)
{
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
    // printf("row=%d\ncol=%d\n", row, col);
    // 得到图片大小 = 54(定义文件) + 1920x1080x3 = 6220854
    fseek(fp, 0, SEEK_END); // 到文档末尾
    int len = ftell(fp);
    // printf("len = %d\n", len);
    byte *data = malloc(sizeof(byte) * (len - 54));
    fseek(fp, 0, SEEK_SET);  // 先把指针指回0
    fseek(fp, 54, SEEK_CUR); // 再加上54偏移，到达数据位
    fread(data, 1, (len - 54), fp); // 把图片里的数据全部导入data,len-54的数据量包含补0,3*row*col不包含补0
    // 将增添数值前的数据导出
    byte *real_data = malloc(sizeof(byte) * (len - 54));
    int move = 0;
    int gap = ((len - col * row * 3 - 54) / row); // 计算图片自动填充0的个数
    // printf("gap=%d\n", gap);
    for (int i = 0; i < len - 54; i++)
    {
        if ((i - move) % (3 * col) == (3 * col - 1))
        {
            real_data[i - move] = data[i];
            move = move + gap;
            i = i + gap;
            continue;
        }
        real_data[i - move] = data[i];
    }

    image.ImgData = (float ***)malloc(row * sizeof(float **)); // 先申请r个地址，每个地址存放行开始的位置
    for (int i = 0; i < image.row; i++)
    {
        image.ImgData[i] = (float **)malloc(col * sizeof(float *)); // 再申请l个地址，每个地址存放一个列向量
        for (int j = 0; j < image.col; j++)
        {
            image.ImgData[i][j] = (float *)malloc(image.channel * sizeof(float));
            for (int k = 0; k < image.channel; k++)
            {
                image.ImgData[i][j][k] = (float)real_data[i * col * image.channel + j * image.channel + k];
            }
        }
    }
    fclose(fp); // 清除文件指针
    free(data);
    free(real_data);
    return image;
}

kernel initialKernel(kernel kernel,int size, int row, int col)
{
    kernel.row = row;
    kernel.col = col;
    kernel.size = size;
    kernel.channel = 3;
    kernel.Data = (float ****)malloc(kernel.size * sizeof(float ***)); // 分配kernel的个数个空间
    for (int s = 0; s < kernel.size; s++)
    {
        kernel.Data[s] = (float ***)malloc(kernel.row * sizeof(float **)); // 申请每一行的指针
        for (int i = 0; i < kernel.row; i++)
        {
            kernel.Data[s][i] = (float **)malloc(kernel.col * sizeof(float *)); // 申请每一列的指针
            for (int j = 0; j < kernel.col; j++)
            {
                kernel.Data[s][i][j] = (float *)malloc(kernel.channel * sizeof(float));
                for (int k = 0; k < kernel.channel; k++)
                {
                    kernel.Data[s][i][j][k] = 0.03f; // 尝试让第一个kernel全为1，查看输出文件
                }
            }
        }
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
    data.Data = (float ***)malloc(data.size * sizeof(float **));
    float sum = 0;
    int x,y;
    for (int s = 0; s < kernel.size; s++)
    { // 每一个kernel循环
        data.Data[s] = (float **)malloc(data.row * sizeof(float *));
        // 遍历coved_data里的每一位数据
        for (int i = 0; i < data.row; i++)
        {
            data.Data[s][i] = (float *)malloc(data.col * sizeof(float));
            for (int j = 0; j < data.col; j++)
            {
                // 计算coved_data中每一位的数据，具体方式为对应位置相乘求和
                for (int ii = 0; ii < kernel.row; ii++)
                {
                    x = -padding_x + stride_x * i + ii;
                    for (int jj = 0; jj < kernel.col; jj++)
                    {
                        y = -padding_y + stride_y * j + jj;
                        if (x < 0 || x >= image.row || y < 0 || y >= image.col)
                        {
                            // printf(":continue\n");
                            continue; // 判断数字是否取padding值，若是padding则0
                        }
                        for (int kk = 0; kk < kernel.channel; kk++)
                        {
                            sum = sum + image.ImgData[x][y][kk] * kernel.Data[s][ii][jj][kk];
                        }
                    }
                }
                data.Data[s][i][j] = sum;
                sum = 0;
            }
        }
    }
    return data;
}

// 释放图片的内存
void free_image(image *image)
{
    for (int i = 0; i < image->row; i++)
    {
        for (int j = 0; j < image->col; j++)
        {
            free(image->ImgData[i][j]);
        }
    }
}

// 释放kernel的内存
void free_kernel(kernel *kernel)
{
    for (int i = 0; i < kernel->size; i++)
    {
        for (int j = 0; j < kernel->row; j++)
        {
            for (int k = 0; k < kernel->col; k++)
            {
                free(kernel->Data[i][j][k]);
            }
        }
    }
}

// 输出图片，以验证结果
void WriteBMP(coved_data img, const char *filename)
{
    int row = img.row;
    // 每一行需要考虑补0，让每一行是四字节的整数倍
    int gap = (img.col % 4 == 0)? 0 : 4 - (img.col % 4);
    int col = img.col + gap;
    FILE *fp = fopen(filename, "wb");
    fprintf(fp, "BM");
    int bmi[] = {col * row + 54 + 1024, 0, 54, 40, img.col, row, 1 | 8 << 16, 0, col * row, 0, 0, 256, 0};
    fwrite(&bmi, 52, 1, fp);
    // 定义颜色表
    RGBQUAD rgbquad[256];
    for (int p = 0; p < 256; p++)
    {
        rgbquad[p].rgbBlue = (byte)p;
        rgbquad[p].rgbGreen = (byte)p;
        rgbquad[p].rgbRed = (byte)p;
        rgbquad[p].rgbReserved = (byte)0;
    }
    fwrite(&rgbquad, 4, 256, fp);
    byte *array = (byte *)malloc(col * row * sizeof(byte));
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            if (j >= col - gap)
            {
                array[i * col + j] = (byte)0;
                continue;
            }
            else
                array[i * col + j] = (byte)img.Data[0][i][j];
        }
    }
    fwrite(array, 1, row * col, fp);
    free(array);
    fclose(fp);
}

int main()
{
    // 初始化图像
    clock_t start, finish;
    image image;
    image = initialImage(image,"input_image.bmp");
    kernel kernel;
    kernel = initialKernel(kernel,1,5,5);
    coved_data data;
    start = clock();
    // 输入为(stride_x,stride_y,padding_x,padding_y,kernel,image)
    data = cov(1, 1, 0, 0, kernel, image);
    finish = clock();
    printf("%lfs\n", (float)(finish - start) / (float)1000000);
    WriteBMP(data, "output_image.bmp");
    free_image(&image);
    free_kernel(&kernel);
    return 0;
}
