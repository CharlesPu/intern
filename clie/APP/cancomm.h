#ifndef CANCOMM_H
#define CANCOMM_H

#include "buffer.h"
#include <net/if.h>
#include <pthread.h>
#include "common.h"


class CanComm
{

public:
	CanComm();
	~CanComm();
    bool CanInit(int channel);
    void CanRecvFilter(int& sock);
    static void canPrintFrame(struct can_frame*);

public:
	int can_sock;
	int can_channel;
    struct ifreq _ifr1, _ifr2;
	Buffer can_buf;	
	pthread_cond_t send_signal ;
	pthread_mutex_t send_lock ;	

};
#define CAN_SFF_MASK 0x000007FFU /* 标准帧格式(SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* 扩展帧格式(EFF) */

#endif
