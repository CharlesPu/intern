/*******************************
@@Author     : Charles
@@Date       : 2017-10-18
@@Mail       : pu17rui@sina.com
@@Description: declaration of tasks            
*******************************/
#ifndef __TASK_H
#define __TASK_H

class Task
{
public:
    Task();
    ~Task();
	
	static void* CanReceive(void* can_eth_para);
	static void* ServerReceive(void* can_eth_para);
	static void* CanSend(void* can);
	static void* ServerSend(void* s);
};
bool IsBelongTo(unsigned short canfr_id, void* owner);
void CreatAllRecvFilter(void);
void BubbleSort(unsigned short arr[],int size);

#endif

