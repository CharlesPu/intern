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
//#include "server.h"
//#include "buffer.h"
//#include "task.h"
//#include "cancomm.h"

using namespace std;

/********************   配置项      ********************/
//#define DES
//#define PRINT_ULTI_SND_REC
#define PRINT_CAN
//#define PRINT_BUFFER

#define SERV_IP "192.168.2.165"
#define TASK_NUM 6
#define filter_num 5
const unsigned short filter_id[filter_num]={
//	      /*per 10ms*/	0x00F9,	
//		  /*per 25ms*/	0x0175,0x0189,0x0197,
		  0x019D,0x01A6,0x01AF,0x01F5,
//		  /*per 50ms*/	0x02A5,
		  /*per 250ms*/	0x03F5,
//		  /*per 500ms*/	0x04A5,0x04C9
		  					};

/****************************************************/


#define PORT 4000 // The port which is communicate with server
#define BACKLOG 10// tongshi lianjie de kehuduan shuliang
#define BUF_LENGTH 4000 // Buffer length

#define ttyO0  0
#define ttyO1  1
#define ttyO2  2
#define ttyO3  3
#define ttyO4  4
#define ttyO5  5

#ifndef AF_CAN
#define AF_CAN 29
#endif
#ifndef PF_CAN
#define PF_CAN AF_CAN
#endif

#define errout(_s)  fprintf(stderr, "error class: %s\n", (_s))
#define errcode(_d) fprintf(stderr, "error code: %02x\n", (_d))
#define myerr(str)	fprintf(stderr, "%s, %s, %d: %s\n", __FILE__, __func__, __LINE__, str)

class Server;
class Buffer;
class CanComm;

typedef struct ThreadPara
{
	CanComm* _can;
	CanComm* _another_can;
    Server* _server;
}CanEthPara;


#endif // COMMON_H
