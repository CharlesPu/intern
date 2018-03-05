/*******************************
@@Author     : Charles
@@Date       : 2017-11-29
@@Mail       : pu17rui@sina.com
@@Description: thread creat and init             
*******************************/
#include <pthread.h>
#include "client.h"
#include "common.h"
#include "cancomm.h"
#include "buffer.h"
#include "task.h"

int main(int argc,char* args[])
{
    pthread_t th[4];
	Client c;
    int port;
    if(argc == 1)
        port = PORT;
    else
        port = atoi(args[1]);

	CreatAllRecvFilter();
	
	if(!c.ClientInit(port))
         return -1;
	printf("ClientInit Succes\n");

	CanComm can3;
	if(!can3.CanInit(3))
		return -1;
	printf("Can3Init Success\n");
	CanComm can4;
	if(!can4.CanInit(4))
		return -1;
	printf("Can4Init Success\n");

	CanEthPara can_eth_para1;
	can_eth_para1._can=&can3;
	can_eth_para1._another_can=&can4;
	can_eth_para1._client=&c;
	CanEthPara can_eth_para2;
	can_eth_para2._can=&can4;
	can_eth_para2._another_can=&can3;
	can_eth_para2._client=&c;
	
	pthread_attr_t attr[TASK_NUM];
	int polic=SCHED_RR ; 
	struct sched_param sched_param[TASK_NUM];
	int prior[TASK_NUM]={98,97,99,88,87,89};

	for(int i=0;i<TASK_NUM;i++)
	{
		pthread_attr_init(&attr[i]);	
		pthread_attr_setschedpolicy( &attr[i], polic);
//		printf("task%d's policy now is :%d \n",i,polic);
//		pthread_attr_getschedparam(&attr[i],&sched_param[i]);
//		printf("task%d's param past is :%d \n",i,sched_param[i].sched_priority);	
		sched_param[i].sched_priority=prior[i];
		pthread_attr_setschedparam(&attr[i],&sched_param[i]);
//		printf("task%d's param now is :%d \n",i,sched_param[i].sched_priority); 		
	}

	int ret1 = pthread_create(&th[0],&attr[0],Task::CanReceive,(void*)&can_eth_para1);
	int ret2 = pthread_create(&th[1],&attr[1],Task::CanReceive,(void*)&can_eth_para2);
	int ret3 = pthread_create(&th[2],&attr[2],Task::ClientReceive,(void*)&can_eth_para1);
	int ret4 = pthread_create(&th[3],&attr[3],Task::CanSend,(void*)&can3);
	int ret5 = pthread_create(&th[4],&attr[4],Task::CanSend,(void*)&can4);
	int ret6 = pthread_create(&th[5],&attr[5],Task::ClientSend,(void*)&c);

//	int ret1 = pthread_create(&th[0],&attr[0],Task::CanReceive,(void*)&can_eth_para2);
//	int ret2 = pthread_create(&th[1],&attr[1],Task::CanReceive,(void*)&can_eth_para1);
//	int ret3 =0;// pthread_create(&th[2],&attr[2],Task::ClientSend,(void*)&c);
//	int ret4 =0;// pthread_create(&th[3],&attr[3],Task::ClientReceive,(void*)&can_eth_para1);
//	int ret5 =0;// pthread_create(&th[4],&attr[4],Task::CanSend,(void*)&can3);
//	int ret6=0;

	for(int i=0;i<TASK_NUM;i++)
		pthread_attr_destroy(&attr[i]); 		

	if (ret1 || ret2 || ret3 || ret4 || ret5 || ret6)
	{
		printf("ERROR:return code frome pthread_create is %d %d %d %d %d %d",ret1,ret2,ret3,ret4,ret5,ret6);
		exit(-1);
	}
	else
		printf("PthreadCreate Success\n");
	
	for(int i=0;i<TASK_NUM;i++)
		pthread_join(th[i],0);		

	return 0;	

}
