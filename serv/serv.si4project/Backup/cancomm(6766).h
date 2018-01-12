#ifndef _CAN_H
#define _CAN_H

//#include "buffer.h"
//#include "common.h"
#include <linux/can.h>
#include <linux/can/error.h>
#include <linux/can/raw.h>
#include <linux/socket.h>

class Buffer;
class CanComm
{
public:
    CanComm(int master = 1,int frameId = 0)
    {
//        _master = master;
//        _frameId = frameId;
    }

    ~CanComm()
    {
        close(can_sock);
//        close(m_s2);
    }

public:
    bool CanInit();
    static void CanRecvFilter(int& s,bool isSet);
    static void canPrintFrame(struct can_frame*);

public:
	int can_sock;
    struct ifreq _ifr1, _ifr2;
	char* can_channel;
	Buffer* can_buf;
//	unsigned short 

};
#define CAN_SFF_MASK 0x000007FFU /* 标准帧格式(SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* 扩展帧格式(EFF) */

#endif
