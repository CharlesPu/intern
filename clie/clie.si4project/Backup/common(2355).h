#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/socket.h>
#include <linux/can.h>
#include <linux/can/error.h>
#include <linux/can/raw.h>
#include <time.h>
#include <iostream>
#include <cstdio>
#include <vector>

using namespace std;

/********************   配置项      ********************/
//#define DES
//#define PRINT_ULTI_SND_REC
//#define PRINT_CAN
//#define PRINT_BUFFER

#define SERV_IP "192.168.2.165"
#define TASK_NUM 6
#define filter_size_per_node 30 //每个节点的发送和接收id数组最大数量
#define node_num_per_bus 20  //每个总线上挂载的设备最大数量
#define filter_size_per_bus 60  //每个总线上接收id数组的最大数量

/****************************************************/


#define PORT 4000 // The port which is communicate with server
#define BACKLOG 10// tongshi lianjie de kehuduan shuliang
#define BUF_LENGTH 4000 // Buffer length

#ifndef AF_CAN
#define AF_CAN 29
#endif
#ifndef PF_CAN
#define PF_CAN AF_CAN
#endif

#define errout(_s)  fprintf(stderr, "error class: %s\n", (_s))
#define errcode(_d) fprintf(stderr, "error code: %02x\n", (_d))
#define myerr(str)	fprintf(stderr, "%s, %s, %d: %s\n", __FILE__, __func__, __LINE__, str)

class Client;
class Buffer;
class CanComm;

typedef struct ThreadPara
{
	CanComm* _can;
	CanComm* _another_can;
    Client* _client;
}CanEthPara;

typedef struct _node
{
	int id;
	const char name[30];
	const unsigned short recv_id[filter_size_per_node];
	const unsigned short send_id[filter_size_per_node];	
}Node;




#endif // COMMON_H
