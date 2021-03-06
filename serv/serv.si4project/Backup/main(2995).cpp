#include <pthread.h>
#include "Server.h"
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "canComm.h"
#include "messages.h"
#include "buffer.h"
#include "task.h"

//void* WaitingForClients(void* sock);
//void* WaitingForCan(void* ctp);
//void* MonitorBuffer(void* stp);
//void* setTimer(void* tim);
//void* Test_uart(void* uart);


int main(int argc,char* args[])
{
    pthread_t th[4];
    Server s;
    int port;
    if(argc == 1)
        port = 8083;
    else
        port = atoi(args[1]);

    if(!s.ServerInit(port))
        return -1;
    printf("ServerInit Success\n");
    
    CanComm can;
    if(!can.canInit())
    {
        cout << "can init failed";
        return -1;
    }  
    printf("CanInit Success\n");
	Buffer b;
	b.BufferInit();
	
    CTP canMsg;
    canMsg.fd1 = can.m_s1;//绑定的socketcan套接字标识符
   //canMsg.fd2 = can.m_s2;
    canMsg.frameId = can._frameId;
    canMsg.master = can._master;
    canMsg._server = &s;
	canMsg._sbuffer= &b;

    STP serMsg;
    serMsg._pCtp = &canMsg;
    serMsg._pServer = &s;

    TTP timMsg;
    timMsg.seconds=0;//1s
    timMsg.mseconds=100;//0ms

	pthread_attr_t attr[TASK_NUM];
	int polic=SCHED_RR ; 
	struct sched_param sched_param[TASK_NUM];
	int prior[TASK_NUM]={90,85,80};

	for(int i=0;i<TASK_NUM;i++)
		{
		pthread_attr_init(&attr[i]);	
		pthread_attr_setschedpolicy( &attr[i], polic);
		printf("task%d's policy now is :%d \n",i,polic);
		pthread_attr_getschedparam(&attr[i],&sched_param[i]);
		printf("task%d's param past is :%d \n",i,sched_param[i].sched_priority);	
		sched_param[i].sched_priority=prior[i];
		pthread_attr_setschedparam(&attr[i],&sched_param[i]);
		printf("task%d's param now is :%d \n",i,sched_param[i].sched_priority);			
		}

    int ret1 = pthread_create(&th[0],&attr[0],Task::WaitingForClients,(void*)&serMsg);//等待客户端连接线程

    int ret2 = pthread_create(&th[1],&attr[1],Task::WaitingForCan,(void*)&canMsg);//can测试线程

    int ret3 = pthread_create(&th[2],&attr[2],Task::MonitorBuffer,(void*)&serMsg);//定时向can发送线程
   // int ret3 = pthread_create(&th[2],NULL,setTimer,(void*)&timMsg);//定时向can发送线程
    
   // int ret4=  pthread_create(&th[3],NULL,Test_uart,NULL);//串口发送数据线程
   	for(int i=0;i<TASK_NUM;i++)
		pthread_attr_destroy(&attr[i]); 		

    if (ret1 || ret2 || ret3 )//||ret4)
    {
        printf("ERROR:return code frome pthread_create is %d %d %d",ret1,ret2,ret3);
        exit(-1);
    }
    else
        printf("PthreadCreate Success\n");
	
   	for(int i=0;i<TASK_NUM;i++)
    	pthread_join(th[i],0);		

    return 0;
}
