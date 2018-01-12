#include "cancomm.h"
#include <pthread.h>


CanComm::CanComm()
{
	can_sock=0;
}

CanComm::~CanComm()
{	
	if(can_sock)
    	close(can_sock);
}


bool CanComm::CanInit(int channel)
{	
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

	switch (channel)
		{
			case 0:	strcpy(_ifr1.ifr_name,"can0");//指定can设备
					can_channel=0;
					break;
			case 1:	strcpy(_ifr1.ifr_name,"can1");//指定can设备
					can_channel=1;
					break;
			case 2:	strcpy(_ifr1.ifr_name,"can0");//指定can设备
					can_channel=2;
					break;
			case 3:	strcpy(_ifr1.ifr_name,"can1");//指定can设备
					can_channel=3;
					break;	
			default:printf("channel error!\n");
					return false;
					break;
		}
		
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

//    printf("socket1 init failed.\n");

   /* if(!socketInit(m_s2,_ifr2))
    {
        printf("socket2 init failed.\n");
        return false;
    }
    */
	can_buf.BufferInit();	
	send_signal = PTHREAD_COND_INITIALIZER;
	send_lock = PTHREAD_MUTEX_INITIALIZER;	
    CanRecvFilter(can_sock);
    //canFilter(m_s2,isSetFilter);

    return true;
}
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

extern unsigned short recvid_on_bus[4][filter_size_per_bus];
extern int recvid_on_bus_size[4];
void CanComm::CanRecvFilter(int& sock)
{
	struct can_filter m_filter[recvid_on_bus_size[0]+recvid_on_bus_size[1]+recvid_on_bus_size[2]+recvid_on_bus_size[3]
								- recvid_on_bus_size[can_channel]	];//can的接收过滤器是由另外三个can总线的recvid数组决定
	int size=0;
	for(int i=0;i<4;i++)
		{
			if(i != can_channel)
				for (int j = 0;j < recvid_on_bus_size[i]; ++ j)
					{	
						m_filter[size].can_id = recvid_on_bus[i][j] ;
						m_filter[size++].can_mask = CAN_SFF_MASK;
					}	
		}
	printf("filter_id:\n");
	for(int i=0; i<size ;i++)
		printf("0x%03x\t",m_filter[i].can_id);
    /* 启用过滤规则*/
    int ret = setsockopt(sock, SOL_CAN_RAW, CAN_RAW_FILTER, &m_filter, sizeof(m_filter));//这里一定要注意，如果sizeof的大小为0，则什么都不接收
    if (ret < 0)																		 //如果sizeof有大小，但是数组里存在0，则什么都接收
        perror("setsockopt filter failed");												 //所以一定要注意m_filter的大小！！！不能分配过大！！
}


