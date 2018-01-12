#ifndef __TASK_H
#define __TASK_H

#include <netinet/in.h>
#include <sys/epoll.h>

class Task
{
public:
    Task();
    ~Task();	

	void* WaitingForServer(void* csock);
	void* WaitingForCan(void* ctp);
	void* MonitorBuffer(void* cltp);
	
	void* setTimer(void* tim);
	void* Test_uart(void* uart);

	int task_sum=0;

};

#endif
