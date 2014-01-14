//#include "linuxadd.h"
//#include <time.h>
//
//int gettimeofday(struct timeval* tv,struct timezone* tz) 
//{
//	time_t long_time;
//
//	time( &long_time );
//	tv->tv_usec =long_time;
//	tv->tv_sec=100;
//
//	return (0);
//}

#include "linuxadd.h"
#include <time.h>
#include <windows.h>


int gettimeofday(struct timeval* tv,struct timezone* tz) 
{
	//time_t long_time;
	//long_time=clock()

	//time( &long_time );
	LARGE_INTEGER t1,feq;   
	QueryPerformanceFrequency(&feq);//每秒跳动次数   
	QueryPerformanceCounter(&t1);//测前跳动次数  
	double d=((double)t1.QuadPart)/((double)feq.QuadPart);
	tv->tv_sec =d;
	tv->tv_usec=(d-tv->tv_sec)*100000000;

	return (0);
}

//double erfc (double x)
//{
//	for (int i=0;i<x;i++)
//	{
//		;
//	}
//	return x;
//}