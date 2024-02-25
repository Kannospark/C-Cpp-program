#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>




int main()
{	
	timeval starttime,endtime;
    start = clock();
    sleep(1);
    finish = clock();
    printf("start is %ld\n",start);
    printf("finish is %ld\n",finish);
    printf("time is %ld\n",finish-start);
    return 0;
}

