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

	static void* CanReceive(void* can_eth_para);
	static void* ClientReceive(void* can_eth_para);
	static void* CanSend(void* can);
	static void* ClientSend(void* c);
	static void* CanSend_test(void* can_eth_para);
	static void* test(void*);
	static void* SendAll(void* can_eth_para);
};
bool IsBelongTo(unsigned short canfr_id, void* owner);
void CreatAllRecvFilter(void);
void BubbleSort(unsigned short arr[],int size);

#endif
