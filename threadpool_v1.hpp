#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <list>
#include <exception>
#include <unistd.h>
#include <pthread.h>
#include "./util.hpp"

/*
* v1版本线程池：	tested
	可读性高，基于信号量实现线程同步
v2版本线程池：
	性能尽我所能优化至最佳，基于条件变量实现线程同步
v3版本线程池：
	改用C++11标准库中的thread
*/

/*	SafeQueue类 - 线程安全的队列	*/
#define DEFAULT_SAFEQUEUE_MAX_NUM 10000

template <typename T>
class SafeQueue
{
public:
	SafeQueue(int max_num = DEFAULT_SAFEQUEUE_MAX_NUM) : m_max_num(max_num) {}
	~SafeQueue() {};
	bool change_max_num(int max_num)
	{
		if(max_num <= 0)
		{
			printf("Error: SafeQueue::change_max_num: max_num <= 0");
			throw std::exception();
		}
		m_max_num = max_num;
		return true;
	}
	bool push(T *request)
	{
		m_mtx.lock();
		//我先把空指针检测 放在push里，之后如果出什么问题再试试把空指针判定放在pop里
		if(m_queue.size() >= m_max_num || nullptr == request)
		{
			m_mtx.unlock();
			return false;
		}
		m_queue.push_back(request);
		m_mtx.unlock();
		//信号量与队列相互独立，故放在锁作用范围外
		m_queuestat.post();	//新增一个资源	
		return true;
	}
	//除非你的T有带指针的拷贝函数，且request原本就指向一个空间，否则你别这么用pop
	bool pop(T *request)
	{
		m_queuestat.wait();	//拿走一个资源
		m_mtx.lock();
		if(m_queue.empty())
		{
			m_mtx.unlock();
			return false;
		}
		T::copy_it(request, m_queue.front());	//用一个浅拷贝，将值拷到request指向的地方
		//*request = m_queue.front();//错误写法，会让其变成野指针
		m_queue.pop_front();
		m_mtx.unlock();
		return true;
	}

private:
	std::list<T*> m_queue;	//存储单位是 指针 —— 指针有效的前提是，其指向的空间有效
	int m_max_num;
	Mutex m_mtx;		//安全访问队列的根本保障
	Sem m_queuestat;	//队列内可用资源数量
};

template <typename T>	//放在queue里的任务请求的类型
//以后用到模板，就得在类后边加个<T>
class ThreadPool
{
public:
	ThreadPool(void (*task)(void*), int thread_num = 0, int max_request_num = 10000);
	~ThreadPool();
	bool append_request(T *request);

private:
	static void *worker(void *arg);	//单个任务 —— 不用开一个类了！
	void run();

private:
	void (*m_task)(void*);	//每个线程进行的任务
	void *m_args;			//传入上述m_task的参数
	pthread_t *m_threads;	//存线程池里所有可用线程的数组
	int m_thread_num;
	SafeQueue<T> m_request_queue;	//线程安全的 工作请求队列
	int m_max_request_num;
	//数据库连接池 *m_sql_conn_pool;	//数据库连接池	
};
template <typename T>
ThreadPool<T>::ThreadPool(void (*task)(void*), int thread_num, int max_request_num) : m_task(task), m_thread_num(thread_num), m_max_request_num(max_request_num)
{
	if(thread_num == 0)
	{
		//则按照当前服务器的CPU数量来指定线程数量
		m_thread_num = sysconf(_SC_NPROCESSORS_ONLN)-1;	//因为主线程得去掉一个
		printf("我选择的是: %d\n", m_thread_num);
	}
	else if(thread_num < 0) {
		printf("Error: ThreadPool::ThreadPool: m_thread_num < 0");
		throw std::exception();
	}
	//修改安全队列的请求容量
	m_request_queue.change_max_num(m_max_request_num);
	//保存 需要运行的回调函数
	m_task = task;	//task函数的参数以request的形式出现并被传入到task中
	//初始化线程池
	m_threads = new pthread_t[m_thread_num];
	if(!m_threads)
	{
		printf("Error: ThreadPool::THreadPool: ThreadPool::m_threads new failed");
		throw std::exception();	
	}
	for(int i = 0; i < m_thread_num; i++)
	{
		//worker是一个固定的工作
		if(pthread_create(m_threads + i, NULL, worker, this) != 0)
		{
			delete[] m_threads;
			printf("Error: ThreadPool::ThreadPool: pthread_create failed when i=%d");
			throw std::exception();
		}
		if(pthread_detach(m_threads[i]))
		{
			delete[] m_threads;
			printf("Error: ThreadPool::ThreadPool: pthread_detach failed when i=%d");
			throw std::exception();
		}
	}
}
template <typename T>
ThreadPool<T>::~ThreadPool()
{
	delete[] m_threads;
}
template <typename T>
bool ThreadPool<T>::append_request(T *request)
{
	return m_request_queue.push(request);
}
//worker作为一个线程内实际在干的事情，需要指认其自身工作在this线程池，并在worker内运行this->run函数
template <typename T>
void *ThreadPool<T>::worker(void *arg)	//给 pthread_create用的
{
	ThreadPool *pool = (ThreadPool *)arg;
	pool->run();
	return nullptr;	//你都detach了你还要啥返回值呀
}
template <typename T>
void ThreadPool<T>::run()	//每个线程内实际进行的工作
{
	//一次出队列就会运行一次task(request)
	while(true)	//这个while(true)本质是在 清空request_queue，直到queue空了就阻塞
	{
		//先从队列中取出一个任务请求
		printf("id:%#x\n", pthread_self());	
		T request;
		if(!m_request_queue.pop(&request))
		{
			printf("id:%#x damn\n");
			continue;
		}
		//解析任务请求，提供服务
		/*	怎么解析 看实际情况		*/
		m_task(&request);
	}
}

#endif
