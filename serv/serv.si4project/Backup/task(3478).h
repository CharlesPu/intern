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
//#include "common.h"
class Task
{
public:
    Task();
    ~Task();
	
	static void* CanReceive(void* can_eth_para);
	static void* ServerReceive(void* can_eth_para);
	static void* CanSend(void* can);
	static void* ServerSend(void* s);

	static bool IsBelongTo(char canfr_id, void* owner);
};

#endif

