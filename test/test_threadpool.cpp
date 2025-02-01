#include <iostream>
#include "../threadpool_v1.hpp"

struct Args
{
	int *m_p_sum;
	int m_num;
};

void ninbu(void *request)
{
	Args *args = (Args*)request;
	*args->p_sum += args->num*args->num*args->num;
	*args->p_sum -= args->num*args->num*args->num;
}

int main()
{
	int sum = 0;
	Args args = {&sum, 777};
	static ThreadPool threadpool(ninbu);

	threadpool.append_request(&args);
	threadpool.append_request(&args);
	threadpool.append_request(&args);

	//这个机制还真不好用。。。如果是那种必须等到future的情况
	while(true)
	{
		printf("%d\n", sum);
	}
	
	return 0;
}
