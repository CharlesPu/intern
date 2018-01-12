#ifndef CANCOMM_H
#define CANCOMM_H

#include "buffer.h"
//#include "common.h"
//#include <linux/can.h>
//#include <linux/can/error.h>
//#include <linux/can/raw.h>
//#include <linux/socket.h>
//#include <sys/socket.h>
#include <net/if.h>


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
//	const unsigned short* filter_id;
	Buffer can_buf;

};
#define CAN_SFF_MASK 0x000007FFU /* 标准帧格式(SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* 扩展帧格式(EFF) */

#endif
