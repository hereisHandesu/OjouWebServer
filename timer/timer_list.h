#ifndef __TIMER_LIST_H__
#define __TIMER_LIST_H__

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*
	采用 信号SIGALRM机制 作为定时器的基本形式
	使用ET(边缘触发模式)
*/



//定时器处理时需要用到的客户端信息
typedef struct ClientInfo
{
	struct sockaddr_in addr;
	int sock_fd;
} ClientInfo;

//单个定时器的定义
class Timer
{
public:
	Timer() : m_prev(nullptr), m_next(nullptr) {}
	~Timer() {}

public:
	time_t expire;	//临期值，即ddl
	ClientInfo *user_info;
	void (* cb_func)(ClientInfo *);
	Timer *m_prev;
	Timer *m_next;
};

//定时器循环链表
class SortTimerList
{
public:
	SortTimerList();
	~SortTimerList();
	void add_timer(Timer *timer);
	void del_timer(Timer *timer);
	void sort_timer(Timer *timer);
	void tick();	//？

private:
	void add_timer(Timer *timer, Timer *lst_head);
	Timer *m_head;
	Timer *m_tail;
};

//一些工具 ？
class Utils
{
public:
	Utils() {};
	~Utils() {};
	void init();
	int set_nonblocking(int fd);
	void add_fd();	//？
	static void sig_handler(int sig);
	void add_sig(int sig, void(handler)(int), bool restart = true);
	void timer_handler();
	
public:
	static int *u_pipe_fd;
	SortTimerList m_timer_lst;
	static int u_epoll_fd;
	int m_timeslot;
};

void cb_func(ClientInfo *user_info);





#endif
