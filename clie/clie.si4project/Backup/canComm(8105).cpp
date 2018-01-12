#include "common.h"
#include "canComm.h"


bool CanComm::canInit()
{
    bool isSetFilter = true;
    srand(time(NULL));

    if(!socketInit(m_s1,_ifr1))
    {
        printf("socket1 init failed.\n");
        return false;
    }

   /* if(!socketInit(m_s2,_ifr2))
    {
        printf("socket2 init failed.\n");
        return false;
    }
    */

    canFilter(m_s1,isSetFilter);
    //canFilter(m_s2,isSetFilter);

    return true;
}

bool CanComm::socketInit(int& s,struct ifreq& ifr)
{
    int domain = PF_CAN;
    int type = SOCK_RAW;
    int protocol = CAN_RAW;
    //默认情况下，发送CAN报文的套接字不想接受自己发送的报文，回还功能关闭
	int ro=0;//0表示关闭（默认），1表示开启
    s = socket(domain,type,protocol);//创建socketcan套接字
    if (s < 0)
    {
        perror("socket PF_CAN failed");
        return false;
    }

    strcpy(ifr.ifr_name,"can0");//指定can0设备
    struct sockaddr_can addr;
    int ret = 0;
    ret = ioctl(s, SIOCGIFINDEX, &ifr);
    if (ret < 0)
    {
        perror("ioctl failed");
        return false;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    ret = bind(s, (struct sockaddr *)&addr, sizeof(addr));//将套接字与can0绑定
    if (ret < 0)
    {
        perror("bind failed");
        return false;
    }
    setsockopt(s,SOL_CAN_RAW,CAN_RAW_RECV_OWN_MSGS,&ro,sizeof(ro));
    return true;
}

void CanComm::canFilter(int& s,bool isSet = false)
{
    if(!isSet)
        return;
    /* 第1个规则是可以接收ID为0x001 的 数据帧 */
    m_filter[0].can_id = 0x051 ;
    m_filter[0].can_mask = CAN_SFF_MASK;
    /* 第2个规则是可以接收ID为0x023 的数据帧 */
    //m_filter[1].can_id = 0x023 ;
    //m_filter[1].can_mask = CAN_SFF_MASK;
    /* 第3个规则是可以接收ID为0x1F5 的数据帧 */
    //m_filter[2].can_id = 0x1F5 ;
    //m_filter[2].can_mask = CAN_SFF_MASK;

    /* 启用过滤规则，只要CAN0接收到的数据帧满足上面2个规则中的任何一个也被接受*/
    int ret = setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &m_filter, sizeof(m_filter));
    if (ret < 0)
        perror("setsockopt filter failed");
}

