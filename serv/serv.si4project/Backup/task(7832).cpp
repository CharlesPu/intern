/*******************************
@@Author     : Charles
@@Date       : 2017-10-18
@@Mail       : pu17rui@sina.com
@@Description: definition of all tasks             
*******************************/
#include <pthread.h>
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "cancomm.h"
#include "des.h"
#include "buffer.h"
#include "task.h"

pthread_cond_t has_recv_can = PTHREAD_COND_INITIALIZER;
pthread_cond_t has_recv_eth = PTHREAD_COND_INITIALIZER;

pthread_mutex_t recv_can_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t recv_eth_lock = PTHREAD_MUTEX_INITIALIZER;

void* Task::CanReceive(void* can_eth_para)
{
    CanEthPara* can_eth_para_temp = (CanEthPara*)can_eth_para;
    int fd = can_eth_para_temp->_can->can_sock;
    Server* _server = can_eth_para_temp->_server;
	
	Buffer* can_recv_buf = can_eth_para_temp->_can->can_buf;
	Buffer* another_can_buf = can_eth_para_temp->_another_can->can_buf;
	Buffer* serv_buf = can_eth_para_temp->_server->serv_buf;

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
			if(Task::IsBelongTo(fr.can_id,(void*)can_eth_para_temp->_another_can))//如果要发给另一个can
			{
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
			}else              			//如果要发给以太网
				{
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

				ret=serv_buf->PutFrame(temp, &(serv_buf->send_q),temp[0]);				
			}
//			pthread_mutex_lock(&recv_can_lock);

		/**************封装成以太网帧：报文总长+id+data+异或校验和 ***************/
//			char* temp= new char[BUF_LENGTH];
//			memset(temp,0,BUF_LENGTH);
//			temp[0]= fr.can_dlc+4;		//帧头为一帧总长
//			temp[1]= (fr.can_id>>8)&0xff;
//			temp[2]= fr.can_id&0xff;
//			memcpy(temp+3, fr.data, fr.can_dlc);
//			
//			unsigned char xorsum=0;
//			for(int i=0;i<3+fr.can_dlc;i++)
//				xorsum = xorsum ^ temp[i];
//			xorsum = xorsum & 0xff;
//			temp[fr.can_dlc+3]=xorsum;
//
//			ret=can_buf->PutFrame(temp, &(can_buf->recv_q),temp[0]);
					
//			pthread_mutex_unlock(&recv_can_lock);
//			pthread_cond_signal(&has_recv_can);
#ifdef PRINT_BUFFER
	    /**************目前can接收缓冲区的内容 ***************/
				printf("r_tail=%d\n",_sbuffer->recv_q.tail);
			printf("r_size=%d\n",CHARS(_sbuffer->recv_q));
			for(i=0;i<CHARS(_sbuffer->recv_q);i++)
			{
			   for(int j=0;j<_sbuffer->recv_q.q_buf[(_sbuffer->recv_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
			   		printf("recvbuf[%d]=0x%02x ",j,
			   		 _sbuffer->recv_q.q_buf[(_sbuffer->recv_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
			   printf("length=%d\n",
				 _sbuffer->recv_q.q_buf[(_sbuffer->recv_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
			}
#endif
		}
    }
	delete[] temp;
    return NULL;	
}
void* Task::ServerReceive(void* can_eth_para)
{
    CanEthPara* can_eth_para_temp = (CanEthPara*)can_eth_para;
	Server* serv_temp=  can_eth_para_temp->_server;
	
	Buffer* can_buf = can_eth_para_temp->_can->can_buf;
	Buffer* another_can_buf = can_eth_para_temp->_another_can->can_buf;
	Buffer* serv_buf =	serv_temp->serv_buf;

    while(1)
    {
        int nfds = epoll_wait(serv_temp->_epfd,serv_temp->_events,10,-1);
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
		            pthread_mutex_lock(&recv_eth_lock);
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
								char id_temp
								=((serv_temp->m_rxBuf[frame_head+1]&0xffff)<<8|serv_temp->m_rxBuf[frame_head+2]);
								if(Task::IsBelongTo(id_temp,can_eth_para_temp->_can))
									{
									can_buf->PutFrame(&(serv_temp->m_rxBuf[frame_head], 
										&(can_buf->send_q), frame_len)
									}
								else
									{
									another_can_buf->PutFrame(&(serv_temp->m_rxBuf[frame_head], 
										&(another_can_buf->send_q), frame_len)
									}
#ifdef PRINT_BUFFER
							/**************目前can发送缓冲区的内容 ***************/
									printf("s_tail=%d\n",sbuffer->send_q.tail);
									printf("s_size=%d\n",CHARS(sbuffer->send_q));
									for(int k=0;k<CHARS(sbuffer->send_q);k++)
									{
									   for(int j=0;j<sbuffer->send_q.q_buf[(sbuffer->send_q.tail+k)& (BUF_MAX_SIZE - 1)][0];j++)
											printf("sendbuf[%d]=0x%02x ",j,
											sbuffer->send_q.q_buf[(sbuffer->send_q.tail+k)& (BUF_MAX_SIZE - 1)][j]);
									   printf("length=%d\n",
										sbuffer->send_q.q_buf[(sbuffer->send_q.tail+k)& (BUF_MAX_SIZE - 1)][0]);
									}
#endif
								}
			//				else
			//					printf("xor error!	data missed!\n");
							frame_head += frame_len;
						}
					
					pthread_mutex_unlock(&recv_eth_lock);
					pthread_cond_signal(&has_recv_eth);
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
	
	Buffer* can_buf=can_temp->can_buf;
	
	struct can_frame canfr;
	int ret=0;
	char* temp;
	temp=new char[FRAME_MAX_SIZE];

	while(1)
		{
		pthread_mutex_lock(&recv_eth_lock);
		while(EMPTY(can_buf->send_q))
			pthread_cond_wait(&has_recv_eth,&recv_eth_lock);
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
		pthread_mutex_unlock(&recv_eth_lock);	
		}
	
	delete[] temp;
	return NULL;
}
void* Task::ServerSend(void* s)
{
	Server* serv_temp = (Server*)s;
	Buffer* serv_buf = serv_temp->serv_buf;

	int ret=0;
	char* temp;
	temp=new char[BUF_LENGTH];

	while(1)
		{
			pthread_mutex_lock(&recv_can_lock);
			while(EMPTY(serv_buf->send_q))
				pthread_cond_wait(&has_recv_can,&recv_can_lock);
			memset(temp,0,BUF_LENGTH);
			int len=0;
			while(!EMPTY(serv_buf->send_q))
				{
				ret=serv_buf->GetFrame(&(serv_buf->send_q), temp + len);
				len=ret + len;
				}
			serv_temp->ServerSend(serv_temp->serv_sock ,temp,  len  );
			pthread_mutex_unlock(&recv_can_lock);
		}
	delete[] temp;
	return NULL;	
}

bool Task::IsBelongTo(char canfr_id, void* owner)
{
	struct can_frame* fr_temp = canfr;
	
}

