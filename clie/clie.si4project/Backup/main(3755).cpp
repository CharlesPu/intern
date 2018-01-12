#include <pthread.h>
//#include "Server.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "canComm.h"
#include "messages.h"

//void* WaitingForClients(void* sock);
void* WaitingForServer(void* csock);
void* WaitingForCan(void* ctp);
void* setTimer(void* tim);
void* Test_uart(void* uart);


int main(int argc,char* args[])
{
    pthread_t th[4];
    //Server s;
	Client c;
    int port;
    if(argc == 1)
        port = 8083;
    else
        port = atoi(args[1]);

	if(!c.ClientInit(port))
		{
		 cout << "client init failed";
         return -1;
		}
	printf("ClientInit Succes\n");
    
    CanComm can;
    if(!can.canInit())
       {
        cout << "can init failed";
        return -1;
       }    
    printf("CanInit Success\n");
    CTP canMsg;
    canMsg.fd1 = can.m_s1;//绑定的socketcan套接字标识符
   //canMsg.fd2 = can.m_s2;
    canMsg.frameId = can._frameId;
    canMsg.master = can._master;
    canMsg._client = &c;

    CLTP cliMsg;
	cliMsg._pctp=&canMsg;
	cliMsg._pclient=&c;
	
    //STP serMsg;
    //serMsg._pCtp = &canMsg;
    //serMsg._pServer = &s;

    TTP timMsg;
    timMsg.seconds=1;//1s
    timMsg.mseconds=0;//0ms

    int ret1 = pthread_create(&th[0],NULL,WaitingForServer,(void*)&cliMsg);//等待服务器连接线程

    int ret2 = pthread_create(&th[1],NULL,WaitingForCan,(void*)&canMsg);//can测试线程

    int ret3 = pthread_create(&th[2],NULL,setTimer,(void*)&timMsg);//定时向can发送线程
    
   // int ret4=  pthread_create(&th[3],NULL,Test_uart,NULL);//串口发送数据线程

    if (ret1 || ret2 || ret3 )//||ret4)
    {
        printf("ERROR:return code frome pthread_create is %d %d %d",ret1,ret2,ret3);
        exit(-1);
    }
    else
    {
        printf("PthreadCreate Success\n");
    }
    pthread_join(th[0],0);
    pthread_join(th[1],0);
    pthread_join(th[2],0);
//    pthread_join(th[3],0);

    return 0;
}
