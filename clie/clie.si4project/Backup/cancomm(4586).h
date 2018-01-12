//#include "Common.h"

//class Server;

class CanComm
{
public:
    CanComm(int master = 1,int frameId = 0)
    {
        _master = master;
        _frameId = frameId;
    }

    ~CanComm()
    {
        close(m_s1);
        close(m_s2);
    }

public:
    bool canInit();

    bool socketInit(int&,struct ifreq&);

    void canFilter(int& s,bool isSet);
	
	static void canPrintFrame(struct can_frame *);

public:
	int m_s1, m_s2;
    struct ifreq _ifr1, _ifr2;
    int _master, _frameId;

};
#define CAN_SFF_MASK 0x000007FFU /* 标准帧格式(SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* 扩展帧格式(EFF) */

