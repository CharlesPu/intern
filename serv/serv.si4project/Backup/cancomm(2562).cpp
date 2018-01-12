#include "cancomm.h"
//#include <fcntl.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <string.h>
//#include <errno.h>
//#include <time.h>
//#include <stdio.h>
//#include <sys/ioctl.h>
//#include <arpa/inet.h>
//#include <net/if.h>
#include "common.h"

using namespace std;


bool CanComm::CanInit()
{
    bool isSetFilter = true;
    srand(time(NULL));
	
    int domain = PF_CAN;
    int type = SOCK_RAW;
    int protocol = CAN_RAW;
    //默认情况下，发送CAN报文的套接字不想接受自己发送的报文，回还功能关闭
	int ro=0;//0表示关闭（默认），1表示开启
    can_sock = socket(domain,type,protocol);//创建socketcan套接字
    if (can_sock< 0)
    {
        perror("socket PF_CAN failed");
        return false;
    }

    strcpy(_ifr1.ifr_name,can_channel);//指定can0设备
    struct sockaddr_can addr;
    int ret = 0;
    ret = ioctl(can_sock, SIOCGIFINDEX, &_ifr1);
    if (ret < 0)
    {
        perror("ioctl failed");
        return false;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = _ifr1.ifr_ifindex;

    ret = bind(can_sock, (struct sockaddr *)&addr, sizeof(addr));//将套接字与can0绑定
    if (ret < 0)
    {
        perror("bind failed");
        return false;
    }
    setsockopt(can_sock,SOL_CAN_RAW,CAN_RAW_RECV_OWN_MSGS,&ro,sizeof(ro));

    printf("socket1 init failed.\n");

   /* if(!socketInit(m_s2,_ifr2))
    {
        printf("socket2 init failed.\n");
        return false;
    }
    */

    CanRecvFilter(can_sock,isSetFilter);
    //canFilter(m_s2,isSetFilter);

    return true;
}

//bool Can::socketInit(int& s,struct ifreq& ifr)
//{
//
//    return true;
//}

void CanComm::canPrintFrame(struct can_frame* fra)
{
	struct can_frame* fr=fra;
    printf("recv: can.id=0x%03x  ", (*fr).can_id & CAN_EFF_MASK);
    printf("dlc = %d  ", (*fr).can_dlc);
    printf("data = ");
    for (int i = 0; i < (*fr).can_dlc; i++)
        printf("0x%02x ", (*fr).data[i]);
    printf("\n");
}

void CanComm::CanRecvFilter(int& s,bool isSet)
{
    if(!isSet)
        return;
	struct can_filter m_filter[filter_num];
	for(int i=0;i<filter_num;i++)
		{
		    m_filter[i].can_id = filter_id[i] ;
		    m_filter[i].can_mask = CAN_SFF_MASK;		
		}

    /* 启用过滤规则*/
    int ret = setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &m_filter, sizeof(m_filter));
    if (ret < 0)
        perror("setsockopt filter failed");
}
//void CanComm::CanRecvDivision(void)
//{
//	
//}


