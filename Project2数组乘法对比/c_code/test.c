#include <stdio.h>
#include <stdlib.h>
#include <time.h>
clock_t start, finish;

int main()
{
	srand(1000);	  // 修改随机数种子
	long n = 1000000; // 6个0
	// 以下三行计算时间
	for(int i = 0 ; i < 1; i++)
	calculate_time(n);
	printf("\n");
	// printf("time is %lfus\n", time);
}

int calculate_time(int n)
{
	int *a = (int *)malloc(sizeof(int) * n);
	int *b = (int *)malloc(sizeof(int) * n);
	int *c = (int *)malloc(sizeof(int) * n);
	for (int i = 0; i < n; i++) // 对每一个a和b进行赋值
	{
		a[i] = rand() % 65536+0;  //生成随机数范围0-2^16-1
		b[i] = rand() % 65536+0;
		// if(i % (n/10) == 0) printf("running %d times, max is %d\n",i, n);
	}
	printf("finished valuing\n");

	start = clock(); // 记录开启的时间
	for (int i = 0; i < n; i++)
	{
		c[i] = a[i] * b[i];
		// if(i % (n/10) == 0) printf("running %d times, max is %d\n",i, n);
	}
	finish = clock();//记录结束的时间

	free(a);
	free(b);
	free(c);
	double time = (double)(finish - start) / CLOCKS_PER_SEC * 1000000; // 计算us
	printf("%lg ", time);  //输出时间，单位us
	return 0;
}
