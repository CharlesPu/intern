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
#include <string>
#include <cstdio>
#include <vector>
#include "buffer.h"
using namespace std;

//#define DES
#define PRINT_ULTI_SND_REC

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

class Client;
class Buffer;
typedef struct _CANthreadPara
{
    int fd1;
    int master;
    int frameId;
    int fd2;
    Client* _client;
	Buffer* _cbuffer;
}CTP;

typedef struct _ClientThreadPara
{
    Client* _pclient;
	CTP* _pctp;
}CLTP;

typedef struct _TimerPara
{
    int seconds;
    int mseconds;
}TTP;

#endif // COMMON_H
