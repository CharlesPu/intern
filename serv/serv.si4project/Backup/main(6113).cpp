/*******************************
@@Author     : Charles
@@Date       : 2017-11-29
@@Mail       : pu17rui@sina.com
@@Description: thread creat and init             
*******************************/
#include <pthread.h>
#include "server.h"
#include "common.h"
#include "cancomm.h"
#include "messages.h"
#include "buffer.h"
#include "task.h"

int main(int argc,char* args[])
{
	pthread_t th[6];
	Server s;
    int port;
    if(argc == 1)
        port = 8083;
    else
        port = atoi(args[1]);
	
    if(!s.ServerInit(port))
        return -1;
    printf("ServerInit Success\n");	

	CanComm can1;
	can1.can_channel="can0";
	if(!can1.CanInit())
		return -1;
	printf("Can1Init Success\n");
	CanComm can2;
	can2.can_channel="can1";
	if(!can2.CanInit())
		return -1;
	printf("Can2Init Success\n");

	CanEthPara can_eth_para1;
	can_eth_para1._can=&can1;
	can_eth_para1._another_can=&can2;
	can_eth_para1._server=&s;
	CanEthPara can_eth_para2;
	can_eth_para1._can=&can2;
	can_eth_para1._another_can=&can1;
	can_eth_para1._server=&s;
	
	pthread_attr_t attr[TASK_NUM];
	int polic=SCHED_RR ; 
	struct sched_param sched_param[TASK_NUM];
	int prior[TASK_NUM]={98,97,99,88,87,89};

	for(int i=0;i<TASK_NUM;i++)
	{
		pthread_attr_init(&attr[i]);	
		pthread_attr_setschedpolicy( &attr[i], polic);
		printf("task%d's policy now is :%d \n",i,polic);
//		pthread_attr_getschedparam(&attr[i],&sched_param[i]);
//		printf("task%d's param past is :%d \n",i,sched_param[i].sched_priority);	
		sched_param[i].sched_priority=prior[i];
		pthread_attr_setschedparam(&attr[i],&sched_param[i]);
		printf("task%d's param now is :%d \n",i,sched_param[i].sched_priority); 		
	}

	int ret1 = pthread_create(&th[0],&attr[0],Task::CanReceive,(void*)&can_eth_para1);//等待客户端数据
	int ret2 = pthread_create(&th[1],&attr[1],Task::CanReceive,(void*)&can_eth_para2);//等待客户端数据
	int ret3 = pthread_create(&th[2],&attr[2],Task::ServerReceive,(void*)&can_eth_para1);//等待客户端数据
	int ret4 = pthread_create(&th[3],&attr[3],Task::CanSend,(void*)&can1);//等待客户端数据
	int ret5 = pthread_create(&th[4],&attr[4],Task::CanSend,(void*)&can2);//等待客户端数据
	int ret6 = pthread_create(&th[5],&attr[5],Task::ServerSend,(void*)&s);//等待客户端数据

   	for(int i=0;i<TASK_NUM;i++)
		pthread_attr_destroy(&attr[i]); 		

    if (ret1 || ret2 || ret3 || ret4 || ret5 || ret6)
    {
        printf("ERROR:return code frome pthread_create is %d %d %d %d",ret1,ret2,ret3,ret4,ret5,ret6);
        exit(-1);
    }
    else
        printf("PthreadCreate Success\n");
	
   	for(int i=0;i<TASK_NUM;i++)
    	pthread_join(th[i],0);		

    return 0;	
//	pthread_attr_t attr[TASK_NUM];
//	int polic=SCHED_RR ; 
//	struct sched_param sched_param[TASK_NUM];
//	int prior[TASK_NUM]={99,85,80,79};
//
//	for(int i=0;i<TASK_NUM;i++)
//		{
//		pthread_attr_init(&attr[i]);	
//		pthread_attr_setschedpolicy( &attr[i], polic);
//		printf("task%d's policy now is :%d \n",i,polic);
////		pthread_attr_getschedparam(&attr[i],&sched_param[i]);
////		printf("task%d's param past is :%d \n",i,sched_param[i].sched_priority);	
//		sched_param[i].sched_priority=prior[i];
//		pthread_attr_setschedparam(&attr[i],&sched_param[i]);
//		printf("task%d's param now is :%d \n",i,sched_param[i].sched_priority);			
//		}
//    int ret1 = pthread_create(&th[0],&attr[0],Task::WaitingForClients,(void*)&serMsg);//等待客户端数据
//
//    int ret2 = pthread_create(&th[1],&attr[1],Task::WaitingForCan,(void*)&canMsg);//接收can报文
//
//    int ret3 = pthread_create(&th[2],&attr[2],Task::MonitorRecvBuffer,(void*)&serMsg);//监视接收can的buf
//
//    int ret4 = pthread_create(&th[3],&attr[3],Task::MonitorSendBuffer,(void*)&serMsg);//监视接收eth的buf
//   // int ret3 = pthread_create(&th[2],NULL,setTimer,(void*)&timMsg);//定时向can发送线程
//    
//   // int ret4=  pthread_create(&th[3],NULL,Test_uart,NULL);//串口发送数据线程
//   	for(int i=0;i<TASK_NUM;i++)
//		pthread_attr_destroy(&attr[i]); 		
//
//    if (ret1 || ret2 || ret3 || ret4)
//    {
//        printf("ERROR:return code frome pthread_create is %d %d %d %d",ret1,ret2,ret3,ret4);
//        exit(-1);
//    }
//    else
//        printf("PthreadCreate Success\n");
//	
//   	for(int i=0;i<TASK_NUM;i++)
//    	pthread_join(th[i],0);		
//
//    return 0;
}
