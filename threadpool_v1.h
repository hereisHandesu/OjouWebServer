#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <list>
#include <pthread.h>
#include "./util.h"

/*
* v1版本线程池：
	可读性高
v2版本线程池：
	性能尽我所能优化至最佳	
*/

/*	SafeQueue类 - 线程安全的队列	*/
template <typename T>
class SafeQueue
{
public:
	SafeQueue(int max_num = 10000)
	{
		if(max_num <= 0)
		{
			报错:max_num <= 0;
		}	
	}
	~SafeQueue() {};
	bool push(const T &request)
	{
		m_mtx.lock();
		if(m_queue.size() >= m_max_num)
		{
			m_mtx.unlock();
			return false;
		}
		m_queue.push_back(request);
		m_mtx.unlock();
		return true;
	}
	bool pop(T &request)
	{
		m_mtx.lock();
		if(m_queue.size() <= 0)
		{
			m_mtx.unlock();
			return false;
		}
		request = m_queue.pop_front();
		m_mtx.unlock();
		return true;
	}

private:
	std::list<T> m_queue;
	int m_max_num;
	Mutex m_mtx;
};


class ThreadPool
{
public:
	ThreadPool(int thread_number = 0, int max_request_number = 10000);
	~ThreadPool();

private:
	static void *worker(void *arg);	//单个任务 —— 不用开一个类了！
	void run();

private:
	pthread_t *m_threads;	//存线程池里所有可用线程的数组
	int m_thread_number;
	SafeQueue m_request_queue;	//线程安全的 工作请求队列
	int m_max_request_number;
	数据库连接池 *m_sql_conn_pool;	//数据库连接池	
};
ThreadPool::ThreadPool(int thread_number, int max_request_number) : m_thread_number(thread_number), m_max_request_number(max_request_number)
{
	if(m_thread_number == 0)
	{
		//则按照当前服务器的CPU数量来指定线程数量
	}
	else if(m_thread_number < 0) {
		报错: m_thread_number < 0;
	}
	//初始化线程池
	m_threads = new pthread_t[m_thread_number];
	if(!m_threads)
	{
		报错:new ThreadPool::m_threads failed;
	}
	for(int i = 0; i < m_thread_number; i++)
	{
		if(pthread_create(m_threads + i, NULL, worker, this) != 0)
		{
			delete[] m_threads;
			报错:pthread_create failed when i=%d;
		}
		if(pthread_detach(m_threads[i]))
		{
			delete[] m_threads;
			报错:pthread_detach failed when i=%d;
		}
	}
}
ThreadPool::~ThreadPool()
{
	delete[] m_threads;
}
void *ThreadPool::worker(void *arg)	//没太看懂什么意思
{
	ThreadPool *pool = (ThreadPool *)arg;
	pool->run();
	return pool;
}
void ThreadPool::run()
{
	while(true)
	{
		m_
	}
}

#endif
