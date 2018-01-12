#ifndef CANCOMM_H
#define CANCOMM_H

#include "buffer.h"
#include <net/if.h>
#include <pthread.h>



//class Buffer;
class CanComm
{

public:
	CanComm();
	~CanComm();
    bool CanInit(const char* can_channel);
    static void CanRecvFilter(int& s,bool isSet);
    static void canPrintFrame(struct can_frame*);

public:
	int can_sock;
    struct ifreq _ifr1, _ifr2;
	Buffer can_buf;	
	const unsigned short recv_filter[NODE_MAX_NUM];
	const unsigned short send_filter[NODE_MAX_NUM];
	pthread_cond_t send_signal ;
	pthread_mutex_t send_lock ;	

};
#define CAN_SFF_MASK 0x000007FFU /* 标准帧格式(SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* 扩展帧格式(EFF) */

#endif
