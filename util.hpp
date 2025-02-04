#ifndef __UTIL_H__
#define __UTIL_H__

#include <pthread.h>
#include <semaphore.h>
#include <exception>

/*
	存放 自己重新封装的一些轮子
*/

/*	Mutex类 - 封装后的互斥锁	*/
class Mutex
{
public:
	Mutex()
	{
		if(pthread_mutex_init(&m_mutex, NULL) != 0)
		{
			printf("Error: Mutex::Mutex: pthread_mutex_init failed");
			throw std::exception();
		}
	}
	~Mutex()
	{
		pthread_mutex_destroy(&m_mutex);
	}
	bool lock()
	{
		return pthread_mutex_lock(&m_mutex) == 0;
	}
	bool unlock()
	{
		return pthread_mutex_unlock(&m_mutex) == 0;
	}
	pthread_mutex_t *get()	//给条件变量调用用的
	{
		return &m_mutex;
	}
private:
	pthread_mutex_t m_mutex;
};


/*	Sem类 - 封装后的信号量	*/
//	信号量用于限制临界资源个数
class Sem
{
public:
	Sem()
	{
		if(sem_init(&m_sem, 0, 0) != 0)
		{
			printf("Error: Sem::Sem: sem_init failed");
			throw std::exception();
		}
	}
	Sem(int num)
	{
		if(sem_init(&m_sem, 0, num) != 0)
		{
			printf("Error: Sem::Sem: sem_init failed");
			throw std::exception();
		}
	}
	~Sem()
	{
		sem_destroy(&m_sem);
	}
	bool wait()	//做一次--，<=0时调用则阻塞
	{
		return sem_wait(&m_sem) == 0;
	}
	bool post()	//做一次++，>=n时调用则阻塞
	{
		return sem_post(&m_sem) == 0;
	}
private:
	sem_t m_sem;
};

/*	Cond类 - 封装好的条件变量	*/
//	用于线程间相互告知，以达成线程间同步的效果
class Cond
{
public:
	Cond()
	{
		if(pthread_cond_init(&m_cond, NULL) != 0)
		{
			printf("Error: Cond::Cond: pthread_cond_init failed");
			throw std::exception();
		}
	}
	~Cond()
	{
		pthread_cond_destroy(&m_cond);
	}
	bool wait(pthread_mutex_t *mtx)
	{	
		return pthread_cond_wait(&m_cond, mtx) == 0;
	}
	bool timedwait(pthread_mutex_t *mtx, struct timespec t)
	{
		return pthread_cond_timedwait(&m_cond, mtx, &t) == 0;
	}
	bool signal()
	{
		return pthread_cond_signal(&m_cond) == 0;
	}
	bool broadcast()
	{
		return pthread_cond_broadcast(&m_cond) == 0;
	}
private:
	pthread_cond_t m_cond;
};



#endif
