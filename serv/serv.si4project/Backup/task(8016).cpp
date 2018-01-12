#include <pthread.h>
#include "Server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "canComm.h"
#include "messages.h"
#include "des.h"
#include "serial.h"
#include "buffer.h"
#include "task.h"

STP* timStp;
struct can_frame timFr_tcp;

pthread_cond_t has_recv_can = PTHREAD_COND_INITIALIZER;
pthread_cond_t has_recv_eth = PTHREAD_COND_INITIALIZER;

pthread_mutex_t recv_can_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t recv_eth_lock = PTHREAD_MUTEX_INITIALIZER;

void* Task::setTimer(void* tim)
{
    TTP* ttm=(TTP*)tim;
    struct timeval temp;
	__u8 	buf[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    while(1)
    {
        temp.tv_sec=ttm->seconds;
        temp.tv_usec=ttm->mseconds;
        select(0,NULL,NULL,NULL,&temp);
        //sleep(3);
		timFr_tcp.can_id=  0x020;
		timFr_tcp.can_dlc=  8 ;
		//memcpy((void*)timFr_tcp.data, buf, timFr_tcp.can_dlc);
        //write(timStp->_pCtp->fd1,&timFr_tcp,sizeof(timFr_tcp));
        //printf("\ntime success:%d\n",i++);

    }
    return NULL;
}

void* Task::Test_uart(void* uart)
{
        static int n_com_port = ttyO5;
        sleep(1);
        cout<<"Test_uart success"<<endl;
        int ret = -1;
        int len_tty = -1;
        int fd_rs485;
        unsigned char buf_tty[256];
        /*if (argc > 1)
        {
            n_com_port = strtol( argv[1], NULL, 10 );
        }*/
        ret = OpenComPort(n_com_port, 115200, 8, "1", 'N');
        if (ret < 0)
        {
            fprintf(stderr, "Error: Opening Com Port %d\n", n_com_port);
            return NULL;
        }
        else
        {
            printf("Open Com Port %d Success, Now going to read port\n", n_com_port);
        }
         while(1)
         {
                bzero(buf_tty, sizeof(buf_tty));
        #if 1
                len_tty = ReadComPort(buf_tty, 255);

                if (len_tty < 0)
                {
                    printf("Error: Read Com Port\n");
                    break;
                }
        #endif
        #if 0
                if (len_tty == 0)
                {
                    write(STDOUT_FILENO, ".", sizeof("."));//$)Ad82e?f2!f?h/;e??0f?o<??e1??500ms???d8?d8?."
                    continue;
                }
        #endif
                printf("Recv: %d bytes, [%s]\n", len_tty, buf_tty);

                len_tty = WriteComPort(buf_tty, len_tty);
                //WriteComPort(" recved:", sizeof(" recved:"));
                if (len_tty < 0)
                {
                    printf("Error: WriteComPort Error\n");
                }
                // delay 500 ms to let data transfer complete
                // here will cause bugs
                usleep(1000 * 1000);
            }

            CloseComPort();

            printf("Program Terminated\n");

    return NULL;
}

void* Task::WaitingForClients(void* sock)
{
    STP* stp = (STP*)sock;
    timStp=stp;
    Server* server = stp->_pServer;
    CTP* canctp= stp->_pCtp;
	Buffer* sbuffer = stp->_pCtp->_sbuffer;

	struct can_frame canfr;
	int ret;

    while(1)
    {
        int nfds = epoll_wait(server->_epfd,server->_events,10,-1);
        if(nfds == -1)
        {
            printf("epoll_wait failed %s\n",strerror(errno));
            return false;
        }

        for(int i = 0; i < nfds; i++)
        {
            if(server->_events[i].data.fd < 0)
                continue;

            if(server->_events[i].data.fd == server->_ServerSock)
            {
                if(server->SocketAccept())
                {
                    server->_clientNum++;
                    Server::SetNonblocking(server->_ClientSock);
                    server->_ev.data.fd = server->_ClientSock;
                    server->_ev.events = EPOLLIN | EPOLLERR |EPOLLHUP ;
                    epoll_ctl(server->_epfd,EPOLL_CTL_ADD,server->_ClientSock,&server->_ev);
                    continue;
                }
            }

            if (server->_events[i].events & EPOLLIN)
            {
                int st = server->_events[i].data.fd;
                int ret = server->SocketRecv(st);//client send data
                if(ret <= 0)
                {
                    close(st);
                    server->_events[i].data.fd = -1;
                    continue;
                }else
                {
		            pthread_mutex_lock(&recv_eth_lock);
				  /**************拆分成can报文：报文总长+can报文+校验和 ***************/
					int frame_head=0;
					int frame_len=0;
					while(server->m_rxBuf[frame_head] != 0)
						{
							unsigned char xorsum=0;
							frame_len = server->m_rxBuf[frame_head];
							for(int j=0;j<frame_len -1;j++)
								xorsum = xorsum ^ server->m_rxBuf[frame_head+j];
							xorsum = xorsum & 0xff;
							if(xorsum == server->m_rxBuf[frame_head + frame_len -1])
								{
									ret=sbuffer->PutFrame(&(server->m_rxBuf[frame_head]),
														&(sbuffer->send_q),frame_len);
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

            if(server->_events[i].events & EPOLLERR)
            {
                int st = server->_events[i].data.fd;
                close(st);
                server->_events[i].data.fd = -1;
            }

            if(server->_events[i].events & EPOLLHUP)
            {
                int st = server->_events[i].data.fd;
                close(st);
                server->_events[i].data.fd = -1;
            }
        }
//        usleep(100);
    }
    return NULL;
}

unsigned char cnt=0;
void* Task::WaitingForCan(void* ctp)
{
    CTP* ctp_tmp = (CTP*)ctp;
    int fd1 = ctp_tmp->fd1;
    //int fd2 = ctp_tmp->fd2;
    int frameId = ctp_tmp->frameId;

    Server* _server = ctp_tmp->_server;
	Buffer* _sbuffer = ctp_tmp->_sbuffer;

    int ret,i, j;
    struct can_frame fr, frdup;

    while(1)
    {
        ret = read(fd1, &fr, sizeof(fr));//recv
        if (ret < sizeof(fr))
            myerr("read failed");
        else if(fr.can_dlc>0)
        {
#ifdef PRINT_CAN        	
			CanComm::canPrintFrame(fr);
#endif
			pthread_mutex_lock(&recv_can_lock);

		/**************封装成以太网帧：报文总长+id+data+异或校验和 ***************/
			memset(_server->m_txBuf,0,BUF_LENGTH);
			_server->m_txBuf[0]= fr.can_dlc+4;		//帧头为一帧总长
			_server->m_txBuf[1]= (fr.can_id>>8)&0xff;
			_server->m_txBuf[2]= fr.can_id&0xff;
			memcpy(_server->m_txBuf+3, fr.data, fr.can_dlc);
			if(fr.can_id==0x00f9){	cnt++;	_server->m_txBuf[fr.can_dlc+2]=cnt;}
			unsigned char xorsum=0;
			for(i=0;i<3+fr.can_dlc;i++)
				xorsum = xorsum ^ _server->m_txBuf[i];
			xorsum = xorsum & 0xff;
			_server->m_txBuf[fr.can_dlc+3]=xorsum;

			ret=_sbuffer->PutFrame(_server->m_txBuf, &(_sbuffer->recv_q),
									_server->m_txBuf[0]);
			
			pthread_mutex_unlock(&recv_can_lock);
			pthread_cond_signal(&has_recv_can);
#ifdef PRINT_BUFFER
	    /**************目前can接收缓冲区的内容 ***************/
//				printf("r_tail=%d\n",_sbuffer->recv_q.tail);
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
        //usleep(100);
    }
    return NULL;
}

void* Task::MonitorRecvBuffer(void* stp)
{
	STP* stp_temp=(STP* )stp;

    Server* server = stp_temp->_pServer;
	Buffer* sbuffer = stp_temp->_pCtp->_sbuffer;

	int ret=0;
	char* s_to_c_temp;
	s_to_c_temp=new char[BUF_LENGTH];

	while(1)
		{
		pthread_mutex_lock(&recv_can_lock);
		while(EMPTY(sbuffer->recv_q))
			pthread_cond_wait(&has_recv_can,&recv_can_lock);
		memset(s_to_c_temp,0,BUF_LENGTH);
		int len=0;
		while(!EMPTY(sbuffer->recv_q))
			{
			ret=sbuffer->GetFrame(&(sbuffer->recv_q), s_to_c_temp + len);
			len=ret + len;
			}
		server->SocketSend(server->_ClientSock,s_to_c_temp,  len  );
		pthread_mutex_unlock(&recv_can_lock);
//		if(!EMPTY(sbuffer->recv_q))
//			{
//				memset(s_to_c_temp,0,BUF_LENGTH);
//				int len=0;
//				while(!EMPTY(sbuffer->recv_q))
//					{
//					ret=sbuffer->GetFrame(&(sbuffer->recv_q), s_to_c_temp + len);
//					len=ret + len;
//					}
//				server->SocketSend(server->_ClientSock,s_to_c_temp,  len  );
////				ret=sbuffer->GetFrame(&(sbuffer->recv_q), s_to_c_temp );
////				ret=server->SocketSend(server->_ClientSock,s_to_c_temp,  ret  );
//
////                printf("***************************************************\r\n");
//			}
//		if(!EMPTY(sbuffer->send_q))
//			{
//				memset(c_to_s_temp,0,FRAME_MAX_SIZE);
//				ret=sbuffer->GetFrame(&(sbuffer->send_q), c_to_s_temp);
//			/**************拆分成can报文：id+data***************/
//				canfr.can_id=((c_to_s_temp[1]&0xffff)<<8|c_to_s_temp[2])+1;
//				canfr.can_dlc= ret-4;
//				memset(canfr.data,0,8);
//				memcpy((void*)canfr.data, c_to_s_temp+3, canfr.can_dlc);
//				ret=write(canframe->fd1, &canfr, sizeof(canfr));
////				if(ret=sizeof(canfr))
////					printf("both send to can~\r\n");
////				printf("***************************************************\r\n");
//			}
		}
	delete[] s_to_c_temp;
	return NULL;
}

void* Task::MonitorSendBuffer(void* stp)
{
	STP* stp_temp=(STP* )stp;

    CTP* canframe= stp_temp->_pCtp;
	Buffer* sbuffer = stp_temp->_pCtp->_sbuffer;

	struct can_frame canfr;
	int ret=0;
	char* c_to_s_temp;
	c_to_s_temp=new char[FRAME_MAX_SIZE];

	while(1)
		{
		pthread_mutex_lock(&recv_eth_lock);
		while(EMPTY(sbuffer->send_q))
			pthread_cond_wait(&has_recv_eth,&recv_eth_lock);
		while(!EMPTY(sbuffer->send_q))
			{
				memset(c_to_s_temp,0,FRAME_MAX_SIZE);
				ret=sbuffer->GetFrame(&(sbuffer->send_q), c_to_s_temp);
			/**************拆分成can报文：id+data***************/
				canfr.can_id=((c_to_s_temp[1]&0xffff)<<8|c_to_s_temp[2])+1;
				canfr.can_dlc= ret-4;
				memset(canfr.data,0,8);
				memcpy((void*)canfr.data, c_to_s_temp+3, canfr.can_dlc);
				ret=write(canframe->fd1, &canfr, sizeof(canfr));		
			}
		pthread_mutex_unlock(&recv_eth_lock);	
		}
	
	delete[] c_to_s_temp;
	return NULL;
}

