/*******************************
@@Author     : Charles
@@Date       : 2017-10-18
@@Mail       : pu17rui@sina.com
@@Description: declaration of tasks            
*******************************/
#ifndef __TASK_H
#define __TASK_H

//#include <netinet/in.h>
//#include <sys/epoll.h>

class Task
{
public:
    Task();
    ~Task();	

	static void* WaitingForClients(void* sock);
	static void* WaitingForCan(void* ctp);
	static void* MonitorRecvBuffer(void* stp);
	static void* MonitorSendBuffer(void* stp);
	
	static void* setTimer(void* tim);
	static void* Test_uart(void* uart);

};

#endif

