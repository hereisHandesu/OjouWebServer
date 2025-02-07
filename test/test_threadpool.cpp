#include <iostream>
#include "../threadpool/threadpool_v1.hpp"

struct Args
{
	/*
		request 必须得有 空白的构造函数&copy_it函数
	*/
	Args() : m_cnt(0) {}
	static void copy_it(Args *dst, Args *src)
	{
		dst->m_p_sum = src->m_p_sum;
		dst->m_num = src->m_num;	
	}
	int *m_p_sum;
	int m_num;
	int m_cnt;
};

void ninbu(void *request)
{
	Args *args = (Args*)request;
	//*args->m_p_sum -= args->m_num*args->m_num*args->m_num;
	while(args->m_cnt < 10){
		*args->m_p_sum += args->m_num*args->m_num*args->m_num;
		args->m_cnt++;
		printf("id:%#x - cnt:%d,sum:%d\n", pthread_self(), args->m_cnt, *args->m_p_sum);
	}
	if(args->m_cnt >= 10){
		printf("id:%#x 进来了\n", pthread_self());
		while(true);
	}
	printf("id:%d 出去了\n", pthread_self());
}

int main()
{
	int sum = 0;
	Args args;
	args.m_p_sum = &sum;
	args.m_num = 77;
	static ThreadPool<Args> threadpool(ninbu);

	printf("zheshi main: %#x\n", pthread_self());

	threadpool.append_request(&args);
	threadpool.append_request(&args);
	threadpool.append_request(&args);

	//这个机制还真不好用。。。如果是那种必须等到future的情况
	while(true)
	{
	}
	
	return 0;
}
