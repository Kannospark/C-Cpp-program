// 作者：Ethan-Q
// 链接：https://zhuanlan.zhihu.com/p/262047931
// 来源：知乎
// 著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include "mnist.h"
#define max(a,b) ( (a>b) ? a:b )
const int trainN = 200;
const int testN = 50;
const int out = 10;
const double alpha = 0.01; //learning rate
int step;
ImgArr trainImg;
ImgArr testImg;
double labels[60000];
double labels1[10000];

struct conLayer
{
    int L,W,H;
    double m[30][30][5];
    double b[30][30][5];
    double delta[30][30][5];

}conLayer;

struct fconLayer
{
    int length;
    double m[1000];
    double b[1000];
    double delta[1000];
    double w[20][1000];
}fconLayer;
struct fconLayer init2(struct fconLayer A)
{
    for(int i=0;i<20;i++)
        for(int j=0;j<1000;j++)
            A.w[i][j]=0.01*(rand()%100);
    return A;
}
struct Network
{
    struct conLayer Input_layer;
    struct conLayer conv_layer1;
    struct conLayer pool_layer1;
    struct conLayer filter1[5];
    struct fconLayer fcnn_input;
    struct fconLayer fcnn_w;
    struct fconLayer fcnn_outpot;
}CNN;
struct conLayer init1(struct conLayer A)
{
    for(int i=0;i<30;i++)
    {
        for(int j=0;j<30;j++)
        {
            for(int k=0;k<5;k++)
            {
                A.m[i][j][k]=0.01*(rand()%100);
            }
        }
    }

    return A;
};
void init()
{
    CNN.Input_layer = init1(CNN.Input_layer);
    printf("%f\n",CNN.Input_layer.m[0][0][0]);
    CNN.conv_layer1=init1(CNN.conv_layer1);
    CNN.pool_layer1=init1(CNN.pool_layer1);
    for(int i=0;i<5;i++)
        CNN.filter1[i]=init1(CNN.filter1[i]);
    CNN.fcnn_input = init2(CNN.fcnn_input);
    CNN.fcnn_w = init2(CNN.fcnn_w);
    CNN.fcnn_outpot = init2(CNN.fcnn_outpot);
}



double Relu(double x)
{
    return max(0.0,x);
}


struct fconLayer softmax(struct fconLayer A)
{
    double sum=0.0;double maxx=-100000000;
    for(int i=0;i<out;i++)
        maxx=max(maxx,A.m[i]);
    for(int i=0;i<out;i++)
        sum+=exp(A.m[i]-maxx);
    for(int i=0;i<out;i++)
    {
        A.m[i]=exp(A.m[i]-maxx)/sum;
    }
    return A;
}

double sigmod(double x)
{
    return 1.0/(1.0+exp(-x));
}


double sum(struct conLayer A,int k)
{
    double a=0;
    for(int i=0;i<A.L;i++)
        for(int j=0;j<A.W;j++)
            a+=A.delta[i][j][k];
    return a;
}

double sum1(struct conLayer A,struct conLayer B,int x,int y,int aa)
{
    double a=0;
    for(int i=0;i<A.L;i++)
        for(int j=0;j<A.W;j++)
            a+=A.delta[i][j][aa]*B.m[i+x][j+y][0];
    return a;
}

struct conLayer Update(struct conLayer A,struct conLayer B,struct conLayer C,int aa)
{
    for(int i=0;i<A.L;i++)
        for(int j=0;j<A.W;j++)
            for(int k=0;k<A.H;k++)
            {
                A.m[i][j][k]-=alpha*sum1(C,B,i,j,aa);
                C.b[i][j][k]-=alpha*sum(C,aa);
            }
    return A;
}

struct conLayer CNN_Input(int num,struct conLayer A,int flag,ImgArr trainImg)
{
    A.L=A.W=28;
    A.H=1;
    for(int i=0;i<28;i++)
    {
        for(int j=0;j<28;j++)
        {
            for(int k=0;k<1;k++)
            {
                if(flag==0)
                {
                    A.m[i][j][k]=trainImg->ImgPtr[num].ImgData[i][j];
                }
                else
                {
                    A.m[i][j][k]=trainImg->ImgPtr[num].ImgData[i][j];
                }
            }
        }
    }
    return A;
}

struct conLayer conv(struct conLayer A,struct conLayer B[], int number,struct conLayer C)
{
    memset(C.m, 0, sizeof(C.m));
    for(int i=0;i<number;i++)
    {
        B[i].L=B[i].W=5;
        B[i].H=1;
    }
    C.L=A.L-B[0].L+1;
    C.W=A.W-B[0].W+1;
    C.H=number;
    for(int num=0;num<number;num++)
        for(int i=0;i<C.L;i++)
            for(int j=0;j<C.W;j++)
            {
                for(int a=0;a<B[0].L;a++)
                    for(int b=0;b<B[0].W;b++)
                        for(int k=0;k<A.H;k++)
                        {
                            C.m[i][j][num]+=A.m[i+a][j+b][k]*B[num].m[a][b][k];
                        }
                C.m[i][j][num]=Relu(C.m[i][j][num]+C.b[i][j][num]);
            }
    return C;
}

struct fconLayer Classify_input(struct conLayer A,struct fconLayer B)
{
    int x=0;
    for(int i=0;i<A.L;i++)
        for(int j=0;j<A.W;j++)
            for(int k=0;k<A.H;k++)
                B.m[x++]=sigmod(A.m[i][j][k]);
    B.length=x;
    return B;
}

struct conLayer pool_input(struct conLayer A,struct fconLayer B)
{
    int x=1;
    for(int i=0;i<A.L;i++)
        for(int j=0;j<A.W;j++)
            for(int k=0;k<A.H;k++)
            {
                A.delta[i][j][k]=B.delta[x++];
            }
    return A;
}

struct conLayer pool_delta(struct conLayer A,struct conLayer B)
{

        double aa[A.H][A.L][A.W];
        for(int i=0;i<A.H;i++)
            for(int j=0;j<A.L;j+=2)
                for(int k=0;k<A.W;k+=2)
                {
                        aa[j][k][i] = B.delta[j/2][k/2][i]/4.0;
                        aa[j+1][k][i] = B.delta[j/2][k/2][i]/4.0;
                        aa[j][k+1][i] = B.delta[j/2][k/2][i]/4.0;
                        aa[j+1][k+1][i] = B.delta[j/2][k/2][i]/4.0;
                }
    for(int k=0;k<A.H;k++)
        for(int i=0;i<A.L;i++)
        {
            for(int j=0;j<A.W;j++)
            {
                if(A.m[i][j][k]<0)
                    A.delta[i][j][k]=0;
                else
                    A.delta[i][j][k]=A.m[i][j][k]*aa[i][j][k];
            }
        }
    return A;
}
struct conLayer maxpooling(struct conLayer conv_layer,struct conLayer A)
{
    A.L=conv_layer.L/2;
    A.W=conv_layer.W/2;
    A.H=conv_layer.H;
    for(int k=0;k<conv_layer.H;k++)
        for(int i=0;i<conv_layer.L;i+=2)
            for(int j=0;j<conv_layer.W;j+=2)
                A.m[i/2][j/2][k]=(conv_layer.m[i][j][k]+conv_layer.m[i+1][j][k]+conv_layer.m[i][j+1][k],conv_layer.m[i+1][j+1][k])/4.0;
    return A;
}

struct fconLayer fcnn_Mul(struct fconLayer A,struct fconLayer B,struct fconLayer C)
{
    memset(C.m,0,sizeof(C.m));
    C.length=out;
    for(int i=0;i<C.length;i++)
    {
        for(int j=0;j<A.length;j++)
        {
            C.m[i]+=B.w[i][j]*A.m[j];
        }
        C.m[i]+=B.b[i];
    }
    return C;
}

void forward_propagation(int num,int flag,ImgArr trainImg,double* labels)//做一次前向输出
{
    CNN.Input_layer=CNN_Input(num,CNN.Input_layer,flag,trainImg);
    CNN.conv_layer1=conv(CNN.Input_layer,CNN.filter1,5,CNN.conv_layer1);
    CNN.pool_layer1=maxpooling(CNN.conv_layer1,CNN.pool_layer1);
    CNN.fcnn_input=Classify_input(CNN.pool_layer1,CNN.fcnn_input);
    CNN.fcnn_outpot=fcnn_Mul(CNN.fcnn_input,CNN.fcnn_w,CNN.fcnn_outpot);
    CNN.fcnn_outpot=softmax(CNN.fcnn_outpot);
    for(int i=0;i<out;i++)
    {
        if(i==(int)labels[num])
            CNN.fcnn_outpot.delta[i]=CNN.fcnn_outpot.m[i]-1.0;
        else
            CNN.fcnn_outpot.delta[i]=CNN.fcnn_outpot.m[i];
    }
}
void back_propagation()//反向传播算法
{
    memset(CNN.fcnn_input.delta,0,sizeof(CNN.fcnn_input.delta));
    for(int i=0;i<CNN.fcnn_input.length;i++)
    {
        for(int j=0;j<out;j++)
        {
            CNN.fcnn_input.delta[i]+=CNN.fcnn_input.m[i]*(1.0-CNN.fcnn_input.m[i])*CNN.fcnn_w.w[j][i]*CNN.fcnn_outpot.delta[j];

        }

    }
    for(int i=0;i<CNN.fcnn_input.length;i++)
    {

        for(int j=0;j<out;j++)
        {

            CNN.fcnn_w.w[j][i]-=alpha*CNN.fcnn_outpot.delta[j]*CNN.fcnn_input.m[i];
            CNN.fcnn_w.b[j]-=alpha*CNN.fcnn_outpot.delta[j];

        }
    }

    CNN.pool_layer1=pool_input(CNN.pool_layer1,CNN.fcnn_input);
    CNN.conv_layer1=pool_delta(CNN.conv_layer1,CNN.pool_layer1);//pooling误差传递
    for(int i=0;i<5;i++)
        CNN.filter1[i]=Update(CNN.filter1[i],CNN.Input_layer,CNN.conv_layer1,i);
}

int main()
{
    init();
    LabelArr trainLabel=read_Lable("train-labels.idx1-ubyte");
    ImgArr trainImg=read_Img("train-images.idx3-ubyte");
    LabelArr testLabel = read_Lable("t10k-labels.idx1-ubyte");
    ImgArr testImg = read_Img("t10k-images.idx3-ubyte");
    printf("%d\n",trainLabel->LabelNum);
    double labels[trainLabel->LabelNum];
    for(int i=0;i<trainLabel->LabelNum;i++)
    {
        for(int j=0;j<10;j++){
        if(trainLabel->LabelPtr[i].LabelData[j]==1.0)
            labels[i] = (double)j;
        }
    }
    double labels1[testLabel->LabelNum];
    for(int i=0;i<testLabel->LabelNum;i++)
    {
        for(int j=0;j<10;j++){
        if(testLabel->LabelPtr[i].LabelData[j]==1.0)
            labels1[i] = (double)j;
        }
    }

    step=0;
    for(int time=0;time<100;time++)
    {
        double err=0;
        for(int i=0;i<trainN;i++)
        {
            forward_propagation(i,0,trainImg, labels);
            err-=log(CNN.fcnn_outpot.m[(int)labels[i]]);
            back_propagation();
        }
        printf("\nconvdelta:\n");
        for(int m=0;m<24;m++)
            for(int n=0;n<24;n++)
                printf("%f\t",CNN.conv_layer1.delta[m][n][0]);
        printf("\n convll:\n");

        for(int m=0;m<5;m++)
            for(int n=0;n<5;n++)
                printf("%f\t",CNN.filter1[0].m[m][n][0]);
        printf("step: %d   loss:%.5f\n",time,1.0*err/trainN);//每次记录一遍数据集的平均误差
        int sum=0;
        for(int j=0;j<testN;j++)
        {
            forward_propagation(j,1,testImg,labels1);
            int ans=-1;
            double sign=-1;
            for(int i=0;i<out;i++)
            {
                if(CNN.fcnn_outpot.m[i]>sign)
                {
                    sign=CNN.fcnn_outpot.m[i];
                    ans=i;
                }
            }
            int ans1=ans;
            int label=(int)(labels1[j]);
            if(ans1==label) sum++;
        }

    printf("\n");
    printf("sum:%d\n",sum);
    printf("step:%d   precision: %.5f\n",++step,1.0*sum/testN);
    }
    return 0;
}