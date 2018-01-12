#ifndef __TASK_H
#define __TASK_H

//#include <netinet/in.h>
//#include <sys/epoll.h>

#define TASK_NUM 3
class Task
{
public:
    Task();
    ~Task();	

	static void* WaitingForServer(void* csock);
	static void* WaitingForCan(void* ctp);
	static void* MonitorBuffer(void* cltp);
	
	static void* setTimer(void* tim);
	static void* Test_uart(void* uart);

};

#endif
