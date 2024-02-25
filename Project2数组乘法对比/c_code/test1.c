#include <stdio.h>
#include <stdlib.h>
#include <time.h>
clock_t start, finish;

int main()
{
	// unsigned n = 214748364;   //修改数组长度
	srand(1000); // 修改随机数种子
	unsigned long n = 10000000; //7个0
	//以下四行计算时间
	start = clock();
	calculate_time(n);
	finish = clock();
	double time = (double)(finish - start) / CLOCKS_PER_SEC * 1000000;
	printf("time is %lfus\n", time);
}

void calculate_time(unsigned int n)
{
	for (int j = 0; j < 10; j++)
	{
		int *a = (int *)malloc(sizeof(int) * n);
		int *b = (int *)malloc(sizeof(int) * n);
		int *c = (int *)malloc(sizeof(int) * n);
		// int a[n];
		// int b[n];
		// int c[n];
		// printf("ckeck1\n");
		for (long i = 0; i < n; i++)
		{
			a[i] = 1;
			b[i] = 2;
			if ((i % 100000000) == 0)
				printf("1finished: %ld0 percent\n", i);
		}
		// start = clock();
		for (long i = 0; i < n; i++)
		{
			c[i] = a[i] * b[i];
			if ((i % (n / 10)) == 0)
				printf("2finished: %ld0 percent\n", i);
		}
		// finish = clock();
		free(a);
		free(b);
		free(c);
	}
}
