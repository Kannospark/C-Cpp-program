 
/*
 * 处理PNG图片的c文件
 * */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <windows.h>
#define byte unsigned char

//定义PNG图片体
typedef struct pictureTypedPNG{
    //存储图片路径
    char* path;
    //图像宽度、图像高度
    int width, height;
    //图片大小
    long long pictureSize;
    //图像深度、颜色类型、压缩方法、滤波器方法、隔行扫描方法
    byte depth, colorType, compressionMethod, filterMethod, interlaceMethod;
    //图像本体存储在这里
    byte* body;
}png;
 
//定义byte转int的联合体
typedef union byteToInt{
    byte b[4];//只做转换用，不直接调用
    int i;
}byteToInt;
 
//图片读取
png* getPNG(char* path){
    png* p = (png*)malloc(sizeof(png));
    FILE *fp = fopen(path, "rb");//打开文件。
    if (fp == NULL) // 打开文件失败
        return p;
    //存储路径
    p->path = (char*) malloc((sizeof(char) * strlen(path)));
    strcpy(p->path, path);
    //获取文件大小
    fseek(fp, 0, SEEK_END);//定位文件指针到文件尾。
    p->pictureSize = ftell(fp);//获取文件指针偏移量，即文件大小。
    fseek(fp, 0, SEEK_SET);//定位文件指针到文件头。
    //获取图片体
    p->body = (byte*) malloc(sizeof(byte) * p->pictureSize);//分配存储图片文件的内存
    fread(p->body, 1, p->pictureSize, fp);//读取图片体
    //多byte转int
    byteToInt bti;
    //获取图片部分信息。设置偏移值滤除文件头、IHDR标识信息
    int offset = 8 + 8;
    //获取图像宽度
    for(int i = 3, j = 0; i >= 0; i--, j++){
        bti.b[i] = p->body[j + offset];
    }
    p->width = bti.i;
    //获取图像高度
    for(int i = 3, j = 0; i >= 0; i--, j++){
        bti.b[i] = p->body[j + offset + 4];
    }
    p->height = bti.i;
    //获取图像深度
    p->depth = p->body[8 + offset];
    //获取颜色类型
    p->colorType = p->body[9 + offset];
    //获取压缩方法
    p->compressionMethod = p->body[10 + offset];
    //获取滤波器方法
    p->filterMethod = p->body[11 + offset];
    //获取隔行扫描方法
    p->interlaceMethod = p->body[12 + offset];
    fclose(fp);//关闭文件。
    return p;
}
void printPNG(const png* p){
    const char* colorType = NULL;
    const char* compressionMethod = NULL;
    const char* filterMethod = NULL;
    const char* interlaceMethod = NULL;
    switch(p->colorType){
        case 0: colorType = "灰度图像";break;
        case 2: colorType = "真彩色图像";break;
        case 3: colorType = "索引彩色图像";break;
        case 4: colorType = "带α通道的灰度图像";break;
        case 6: colorType = "带α通道的真彩色图像";break;
    }
    if(!p->compressionMethod){
        compressionMethod = "LZ77派生算法";
    }
    if(!p->filterMethod){
        filterMethod = "0";
    }
    if(p->interlaceMethod){
        interlaceMethod = "Adam7隔行扫描方法";
    }else{
        interlaceMethod = "非隔行扫描";
    }
    printf("图像路径：%s\n"
           "图像大小：%lld\n"
           "图像宽度(像素)：%d\n"
           "图像高度(像素)：%d\n"
           "图像深度：%hhu\n"
           "颜色类型：%s\n"
           "压缩方法：%s\n"
           "滤波器方法：%s\n"
           "隔行扫描方法：%s\n"
		   "图像主体部分：%c\n", p->path, p->pictureSize, p->width, p->height,
           p->depth, colorType, compressionMethod, filterMethod, interlaceMethod,p->body);
}
int main() {
    printf("请输入图片路径: \n");
    char* path = (char*)malloc(sizeof(char) * 100);
    fgets(path, 100, stdin);
    int len = (int)strlen(path);
    *(path + len - 1) = '\0';
    png* picture = getPNG(path);
    printPNG(picture);
    return 0;
}