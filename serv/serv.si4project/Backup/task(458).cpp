/*******************************
@@Author     : Charles
@@Date       : 2017-10-18
@@Mail       : pu17rui@sina.com
@@Description: definition of all tasks             
*******************************/
#include <pthread.h>
#include "server.h"
#include "common.h"
#include "cancomm.h"
#include "des.h"
#include "buffer.h"
#include "task.h"

void* Task::CanReceive(void* can_eth_para)
{
    CanEthPara* can_eth_para_temp = (CanEthPara*)can_eth_para;
    int fd = can_eth_para_temp->_can->can_sock;

	Buffer* another_can_buf = &(can_eth_para_temp->_another_can->can_buf);
	Buffer* _serv_buf = &(can_eth_para_temp->_server->serv_buf);

    unsigned int ret;
    struct can_frame fr;
	char* temp= new char[FRAME_MAX_SIZE];

    while(1)
    {
        ret = read(fd, &fr, sizeof(fr));//recv
        if (ret < sizeof(fr))
            myerr("read failed");
        else if(fr.can_dlc>0)
        {
#ifdef PRINT_CAN        	
			CanComm::canPrintFrame(&fr);
#endif
			if(IsBelongTo(fr.can_id, (void*)can_eth_para_temp->_another_can))//如果要发给另一个can
			{
				pthread_mutex_lock(&(can_eth_para_temp->_another_can->send_lock));
		/**************封装成以太网帧：报文总长+id+data+异或校验和 ***************/
				memset(temp,0,FRAME_MAX_SIZE);
				temp[0]= fr.can_dlc+4;		//帧头为一帧总长
				temp[1]= (fr.can_id>>8)&0xff;
				temp[2]= fr.can_id&0xff;
				memcpy(temp+3, fr.data, fr.can_dlc);

				unsigned char xorsum=0;
				for(int i=0;i<3+fr.can_dlc;i++)
					xorsum = xorsum ^ temp[i];
				xorsum = xorsum & 0xff;
				temp[fr.can_dlc+3]=xorsum;
				
				ret=another_can_buf->PutFrame(temp, &(another_can_buf->send_q),temp[0]);
				pthread_mutex_unlock(&(can_eth_para_temp->_another_can->send_lock));
				pthread_cond_signal(&(can_eth_para_temp->_another_can->send_signal));
#ifdef PRINT_BUFFER
			/**************目前can发送缓冲区的内容 ***************/
	//			printf("s_tail=%d\n",another_can_buf->send_q.tail);
	//			printf("s_size=%d\n",CHARS(another_can_buf->send_q));
				for(int i=0;i<CHARS(another_can_buf->send_q);i++)
				{
				   for(int j=0;j<another_can_buf->send_q.q_buf[(another_can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
						printf("send_buf[%d]=0x%02x ",j,
						 another_can_buf->send_q.q_buf[(another_can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
				   printf("length=%d\n",
					 another_can_buf->send_q.q_buf[(another_can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
				}
#endif
			}else              			//如果要发给以太网
			{
				pthread_mutex_lock(&(can_eth_para_temp->_server->send_lock));
		/**************封装成以太网帧：报文总长+id+data+异或校验和 ***************/
				memset(temp,0,FRAME_MAX_SIZE);
				temp[0]= fr.can_dlc+4;		//帧头为一帧总长
				temp[1]= (fr.can_id>>8)&0xff;
				temp[2]= fr.can_id&0xff;
				memcpy(temp+3, fr.data, fr.can_dlc);
				
				unsigned char xorsum=0;
				for(int i=0;i<3+fr.can_dlc;i++)
					xorsum = xorsum ^ temp[i];
				xorsum = xorsum & 0xff;
				temp[fr.can_dlc+3]=xorsum;

				ret=_serv_buf->PutFrame(temp, &(_serv_buf->send_q),temp[0]);
				pthread_mutex_unlock(&(can_eth_para_temp->_server->send_lock));
				pthread_cond_signal(&(can_eth_para_temp->_server->send_signal));
#ifdef PRINT_BUFFER
			/**************目前eth发送缓冲区的内容 ***************/
	//			printf("s_tail=%d\n",another_can_buf->send_q.tail);
	//			printf("s_size=%d\n",CHARS(another_can_buf->send_q));
				for(int i=0;i<CHARS(_serv_buf->send_q);i++)
				{
				   for(int j=0;j<_serv_buf->send_q.q_buf[(_serv_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
						printf("send_buf[%d]=0x%02x ",j,
						 _serv_buf->send_q.q_buf[(_serv_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
				   printf("length=%d\n",
					 _serv_buf->send_q.q_buf[(_serv_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
				}
#endif

			}
		}
    }
	delete[] temp;
    return NULL;	
}
void* Task::ServerReceive(void* can_eth_para)
{
    CanEthPara* can_eth_para_temp = (CanEthPara*)can_eth_para;
	Server* serv_temp=  can_eth_para_temp->_server;
	
	Buffer* _can_buf = &(can_eth_para_temp->_can->can_buf);
	Buffer* another_can_buf = &(can_eth_para_temp->_another_can->can_buf);

    while(1)
    {
        int nfds = epoll_wait(serv_temp->_epfd,serv_temp->_events,10,-1);//-1为阻塞等待
        if(nfds == -1)
        {
            printf("epoll_wait failed %s\n",strerror(errno));
            return false;
        }

        for(int i = 0; i < nfds; i++)
        {
            if(serv_temp->_events[i].data.fd < 0)
                continue;

            if(serv_temp->_events[i].data.fd == serv_temp->serv_sock)
            {
                if(serv_temp->ServerAccept())
                {
                    serv_temp->_clientNum++;
                    Server::SetNonblocking(serv_temp->clie_sock);
                    serv_temp->_ev.data.fd = serv_temp->clie_sock;
                    serv_temp->_ev.events = EPOLLIN | EPOLLERR |EPOLLHUP ;
                    epoll_ctl(serv_temp->_epfd,EPOLL_CTL_ADD,serv_temp->clie_sock,&serv_temp->_ev);
                    continue;
                }
            }

            if (serv_temp->_events[i].events & EPOLLIN)
            {
                int st = serv_temp->_events[i].data.fd;
                int ret = serv_temp->ServerRecv(st);//client send data
                if(ret <= 0)
                {
                    close(st);
                    serv_temp->_events[i].data.fd = -1;
                    continue;
                }else
                {            
				  /**************拆分成以太网帧：报文总长+can报文+校验和 ***************/
					int frame_head=0;
					int frame_len=0;
					while(serv_temp->m_rxBuf[frame_head] != 0)
						{
							unsigned char xorsum=0;
							frame_len = serv_temp->m_rxBuf[frame_head];
							for(int j=0;j<frame_len -1;j++)
								xorsum = xorsum ^ serv_temp->m_rxBuf[frame_head+j];
							xorsum = xorsum & 0xff;
							if(xorsum == serv_temp->m_rxBuf[frame_head + frame_len -1])
								{
								unsigned short id_temp
								=((serv_temp->m_rxBuf[frame_head+1]&0xffff)<<8|serv_temp->m_rxBuf[frame_head+2]);
								if(IsBelongTo(id_temp,(void*)can_eth_para_temp->_can))//如果属于第一个can的
									{
									pthread_mutex_lock(&(can_eth_para_temp->_can->send_lock));
									_can_buf->PutFrame(&(serv_temp->m_rxBuf[frame_head]), 
										&(_can_buf->send_q), frame_len);
									pthread_mutex_unlock(&(can_eth_para_temp->_can->send_lock));
									pthread_cond_signal(&(can_eth_para_temp->_can->send_signal));
#ifdef PRINT_BUFFER
								/**************目前can发送缓冲区的内容 ***************/
						//			printf("s_tail=%d\n",another_can_buf->send_q.tail);
						//			printf("s_size=%d\n",CHARS(another_can_buf->send_q));
									for(int i=0;i<CHARS(_can_buf->send_q);i++)
									{
									   for(int j=0;j<_can_buf->send_q.q_buf[(_can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
											printf("send_buf[%d]=0x%02x ",j,
											 _can_buf->send_q.q_buf[(_can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
									   printf("length=%d\n",
										 _can_buf->send_q.q_buf[(_can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
									}
#endif

									}
								else	//如果属于另一个can的
									{
									pthread_mutex_lock(&(can_eth_para_temp->_another_can->send_lock));
										another_can_buf->PutFrame(&(serv_temp->m_rxBuf[frame_head]), 
										&(another_can_buf->send_q), frame_len);
									pthread_mutex_unlock(&(can_eth_para_temp->_another_can->send_lock));
									pthread_cond_signal(&(can_eth_para_temp->_another_can->send_signal));
#ifdef PRINT_BUFFER
								/**************目前can发送缓冲区的内容 ***************/
						//			printf("s_tail=%d\n",another_can_buf->send_q.tail);
						//			printf("s_size=%d\n",CHARS(another_can_buf->send_q));
									for(int i=0;i<CHARS(another_can_buf->send_q);i++)
									{
									   for(int j=0;j<another_can_buf->send_q.q_buf[(another_can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
											printf("send_buf[%d]=0x%02x ",j,
											 another_can_buf->send_q.q_buf[(another_can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
									   printf("length=%d\n",
										 another_can_buf->send_q.q_buf[(another_can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
									}
#endif									
									}
								}
			//				else
			//					printf("xor error!	data missed!\n");
							frame_head += frame_len;
						}
                }
            }

            if(serv_temp->_events[i].events & EPOLLERR)
            {
                int st = serv_temp->_events[i].data.fd;
                close(st);
                serv_temp->_events[i].data.fd = -1;
            }

            if(serv_temp->_events[i].events & EPOLLHUP)
            {
                int st = serv_temp->_events[i].data.fd;
                close(st);
                serv_temp->_events[i].data.fd = -1;
            }
        }
//        usleep(100);
    }
    return NULL;	
}
void* Task::CanSend(void* can)
{
	CanComm* can_temp=(CanComm*)can;
	int fd = can_temp->can_sock;
	
	Buffer* can_buf=&(can_temp->can_buf);
	
	struct can_frame canfr;
	int ret=0;
	char* temp;
	temp=new char[FRAME_MAX_SIZE];

	while(1)
		{
#ifdef PRINT_BUFFER
	/**************目前can发送缓冲区的内容 ***************/
		printf("s2_tail=%d\n",can_buf->send_q.tail);
		printf("s2_size=%d\n",CHARS(can_buf->send_q));
		for(int i=0;i<CHARS(can_buf->send_q);i++)
		{
		   for(int j=0;j<can_buf->send_q.q_buf[(can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
		   		printf("2send_buf[%d]=0x%02x ",j,
		   		 can_buf->send_q.q_buf[(can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
		   printf("2length=%d\n",
			 can_buf->send_q.q_buf[(can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
		}
#endif
		pthread_mutex_lock(&(can_temp->send_lock));
		while(EMPTY(can_buf->send_q))
			pthread_cond_wait(&(can_temp->send_signal),&(can_temp->send_lock));
		while(!EMPTY(can_buf->send_q))
			{
				memset(temp,0,FRAME_MAX_SIZE);
				ret=can_buf->GetFrame(&(can_buf->send_q), temp);
			/**************拆分成can报文：id+data***************/
				canfr.can_id=((temp[1]&0xffff)<<8|temp[2])+1;
				canfr.can_dlc= ret-4;
				memset(canfr.data,0,8);
				memcpy((void*)canfr.data, temp+3, canfr.can_dlc);
				ret=write(fd, &canfr, sizeof(canfr));	
			}
		pthread_mutex_unlock(&(can_temp->send_lock));	
		}
	
	delete[] temp;
	return NULL;
}
void* Task::ServerSend(void* s)
{
	Server* serv_temp = (Server*)s;
	Buffer* _serv_buf = &(serv_temp->serv_buf);

	int ret=0;
	char* temp;
	temp=new char[BUF_LENGTH];

	while(1)
		{	
			pthread_mutex_lock(&(serv_temp->send_lock));
			while(EMPTY(_serv_buf->send_q))
				pthread_cond_wait(&(serv_temp->send_signal),&(serv_temp->send_lock));
#ifdef PRINT_BUFFER
		/**************目前eth发送缓冲区的内容 ***************/
			printf("s2_tail=%d\n",_serv_buf->send_q.tail);
			printf("s2_size=%d\n",CHARS(_serv_buf->send_q));
			for(int i=0;i<CHARS(_serv_buf->send_q);i++)
			{
			   for(int j=0;j<_serv_buf->send_q.q_buf[(_serv_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
					printf("2send_buf[%d]=0x%02x ",j,
					 _serv_buf->send_q.q_buf[(_serv_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
			   printf("2length=%d\n",
				 _serv_buf->send_q.q_buf[(_serv_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
			}
#endif
			memset(temp,0,BUF_LENGTH);
			int len=0;
			while(!EMPTY(_serv_buf->send_q))
				{
				ret=_serv_buf->GetFrame(&(_serv_buf->send_q), temp + len);
				len=ret + len;
				}
			serv_temp->ServerSend(serv_temp->clie_sock ,temp,  len  );//注意这里是clie_sock！！！
			pthread_mutex_unlock(&(serv_temp->send_lock));
		}
	delete[] temp;
	return NULL;	
}

extern Node node[];
extern int nodeid_on_bus[4][node_num_per_bus];
extern unsigned short recvid_on_bus[4][filter_size_per_bus];
extern int node_num;
int recvid_on_bus_size[4];//最终二维数组每个维度的有效数据长度

//使用差值查找算法进行查找，可参考 https://www.cnblogs.com/leezx/p/5719012.html
bool IsBelongTo(unsigned short canfr_id, void* owner)
{
	CanComm* can_temp=(CanComm*)owner;
	int channel_temp=can_temp->can_channel;

	unsigned short low=0,mid,high=recvid_on_bus_size[channel_temp] - 1;
	while (low<=high)
		{	if(low == high)
				mid = low;
			else
				mid = low + (canfr_id - recvid_on_bus[channel_temp][low]) / 
							(recvid_on_bus[channel_temp][high] - recvid_on_bus[channel_temp][low]) * (high - low);
			if(canfr_id < recvid_on_bus[channel_temp][mid])
				high = mid -1;
			else if( canfr_id > recvid_on_bus[channel_temp][mid])
				low  = mid +1;
			else 
				return 1;//如果查到了就返回1
		}
	return 0;
}
void CreatAllRecvFilter(void)
{
	//先生成最终数组
	for(int i=0 ;i<4 ;i++)
	{
		int m=0;
		for(int j=0;j< node_num_per_bus; j++)
		{
			for(int k=0; k < node_num ; k++)
			{
				if(node[k].id == nodeid_on_bus[i][j])
				{//复制进recvid_on_bus数组
					for (int n = 0; n < filter_size_per_node; n++)
					{
						if(node[k].recv_id[n] != 0)
							recvid_on_bus[i][m++] = node[k].recv_id[n];
						else 
							break;
					}	
					break;
				}				
			}
		}
		recvid_on_bus_size[i] = m;
		for (int j = 0;j < m; j++)
			printf("0x%03x\t",recvid_on_bus[i][j]);
		printf("\n");
	}
	printf("nodenum: %d\n",node_num);
//	进行冒泡排序的预处理！
	printf("array after bubble:\n");
	for(int i=0; i<4 ; i++)
		{BubbleSort(recvid_on_bus[i],recvid_on_bus_size[i]);printf("\n");}
}

void BubbleSort(unsigned short arr[],int size)
{
    int low = 0, high = size-1 ; 
    unsigned short tmp=0;    
    while (low < high) {    
        for (int j= low; j< high; ++j) //正向冒泡,找到最大者    
            if (arr[j]> arr[j+1]) {    
                tmp = arr[j]; arr[j]=arr[j+1]; arr[j+1]=tmp;    
            }     
        --high;                 //修改high值, 前移一位    
        for (int j=high; j>low; --j) //反向冒泡,找到最小者    
            if (arr[j]<arr[j-1]) {    
                tmp = arr[j]; arr[j]=arr[j-1]; arr[j-1]=tmp;    
            }    
        ++low;                  //修改low值,后移一位    
    } 	
	for(int j=0; j<size ;j++)
		printf("[%d]:0x%03x  ",j,arr[j]);
}

