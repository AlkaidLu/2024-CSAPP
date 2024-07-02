/*
	某个应用程序的随机数产生机制是这样的：
	1、首先产生一个0~0xFFFFFFFF之间的数，称为“密码”X
	2、用(X,0x29A)调用随机数更新函数GenerateRandomNumber，产生新的(Y,Z)
	3、用(Y,Z)调用随机数更新函数GenerateRandomNumber，产生新的(A,B)
	4、把B传给用户
	
	本程序（你的）目的，就是在知道这套机制、知道B的前提下，计算出X是多少？
	
	非常简单的思路：
	1、循环，从i=0~0xFFFFFFFF
	2、GenerateRandomNumber(i,0x29A)
	3、GenerateRandomNumber(A,B)
	4、判断B是否等于想要找的值
	
*/
#include "csapp.h"
#include <stdio.h>
#include <time.h>

int flag=0;
sem_t mutex;
//虚拟机有 2 个 CPU 核,故2线程加速比最高
#define NUM 2
/* 更新随机数 */

void GenerateRandomNumber(unsigned int *rand1_h,unsigned int *rand1_l)
{
	/*请忽略下列代码，知道是产生更新随机数即可，原理不必弄清楚*/
	unsigned long long x = (unsigned long long)*rand1_h;
	x *= 0x6AC690C5;
	x += *rand1_l;

	*rand1_h = (unsigned int)x;
	*rand1_l = (unsigned int)(x>>32);
}
void SearchRandomNumber(unsigned int begin_h,unsigned int begin_l, unsigned int search_val)
{
	/* 在begin_l~begin_h之间，计算B的值，看看是否和search_val相等*/
	unsigned int i;
	unsigned int h,l;

	for (i=begin_l;i<begin_h;i++)
	{
		h = i;
		l = 0x29A;
		GenerateRandomNumber(&h,&l);	// 第一次调用
		GenerateRandomNumber(&h,&l);	// 第二次调用
		if (l==search_val)						// 判断B是否和search_val相等
		{
			printf("找到啦~! 密码是%08X\n",i);	// 相等，找到了~~
		}
	}
}
void MySearch(unsigned int begin_h,unsigned int begin_l, unsigned int search_val)
{
	/* 在begin_l~begin_h之间，计算B的值，看看是否和search_val相等*/
	unsigned int i;
	unsigned int h,l;

	for (i=begin_l;i<begin_h;i++)
	{
		h = i;
		l = 0x29A;
        //P(&mutex);
		GenerateRandomNumber(&h,&l);	// 第一次调用
		GenerateRandomNumber(&h,&l);	// 第二次调用
		if (l==search_val)						// 判断B是否和search_val相等
		{
			printf("找到啦~! 密码是%08X\n",i);	// 相等，找到了~~
		}
        //V(&mutex);
	}
}

void *thread(void* vargp){
    int i=*((int *)vargp);
    unsigned int low = 0xFFFFFFFF / NUM * i;
    unsigned int high = 0xFFFFFFFF / NUM * (i+1);
    SearchRandomNumber(high,low,0x39A6FFBB);
}

int main()
{
	time_t begin_time,end_time;
    time_t serial_time, parrel_time;

    printf("这是一个标准、最慢的搜索程序，拼主频！\n");
	
	begin_time = time(NULL);							// 记录开始时刻
	SearchRandomNumber(0xFFFFFFFF,0,0x39A6FFBB);	// 在0~0xFFFFFFFF之间进行搜索
	// 0x39A6FFBB => 0x70FFFFFF
	end_time = time(NULL);								// 记录结束时刻
    serial_time=end_time-begin_time;
	printf("耗时%ld秒~~\n",serial_time);	// 结束-开始=耗时

    Sem_init(&mutex,0,1);
    int index;
    int indices[NUM];
    pthread_t tid[NUM];
	begin_time = time(NULL);							// 记录开始时刻
    for(index=0;index<NUM;index++){
        indices[index]=index;
        Pthread_create(&tid[index],NULL,thread,&indices[index]);
    }
	// 在0~0xFFFFFFFF之间进行搜索
	// 0x39A6FFBB => 0x70FFFFFF
    for(index=0;index<NUM;index++){
        Pthread_join(tid[index],NULL);
    }
	end_time = time(NULL);								// 记录结束时刻
    parrel_time=end_time-begin_time;
	printf("耗时%ld秒~~\n",parrel_time);	// 结束-开始=耗时
    sem_destroy(&mutex);
    double speedup=(double)serial_time/(double)parrel_time;
    printf("加速比：%.2f\n",speedup);
    return 0;
}
