/*******************************
@@Author     : Charles
@@Date       : 2017-10-18
@@Mail       : pu17rui@sina.com
@@Description: definition of all tasks             
*******************************/
#include <pthread.h>
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "canComm.h"
#include "messages.h"
#include "des.h"
#include "serial.h"
#include "buffer.h"
#include "task.h"
 
struct can_frame timFr_tcp;

pthread_cond_t has_recv_can = PTHREAD_COND_INITIALIZER;
pthread_cond_t has_recv_eth = PTHREAD_COND_INITIALIZER;

pthread_mutex_t recv_can_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t recv_eth_lock = PTHREAD_MUTEX_INITIALIZER;

 
 void* Task::setTimer(void* tim)
 {
	 TTP* ttm=(TTP*)tim;
	 struct timeval temp;
	 while(1)
	 {
		 temp.tv_sec=ttm->seconds;
		 temp.tv_usec=ttm->mseconds;
		// select(0,NULL,NULL,NULL,&temp);
		 //sleep(3);
		// write(timStp->_pCtp->fd1,&timFr_tcp,sizeof(timFr_tcp));
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
//		 int fd_rs485;
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
 

 void* Task::WaitingForServer(void* csock)
 {
	 CLTP* cltp = (CLTP*)csock;
	 Client* client=cltp->_pclient;
	 Buffer* cbuffer = cltp->_pctp->_cbuffer;
	 
	 int ret;
	 
	 while(1) 
	 {
		 ret=client->ClientRecv(client->ClieSock);
		 if(ret>0)
		 {	 
		 	pthread_mutex_lock(&recv_eth_lock);
			
		   /**************拆分成can报文：报文总长+id+data+校验和 ***************/
			int frame_head=0;
			int frame_len=0;
		    while(client->c_rxBuf[frame_head] != 0)
				{
				 unsigned char xorsum=0;
				 frame_len = client->c_rxBuf[frame_head];
				 for(int j=0;j< frame_len - 1;j++)
					 xorsum = xorsum ^ client->c_rxBuf[frame_head+j];
				 xorsum = xorsum & 0xff;
				 if(xorsum == client->c_rxBuf[frame_head + frame_len -1])
					 {
					   ret=cbuffer->PutFrame(&(client->c_rxBuf[frame_head]), 
					   					&(cbuffer->send_q),frame_len);
#ifdef PRINT_BUFFER
			 /**************目前can发送缓冲区的内容 ***************/
//						   printf("s_tail=%d\n",cbuffer->send_q.tail);
					   printf("s_size=%d\n",CHARS(cbuffer->send_q));
					   for(int k=0;k<CHARS(cbuffer->send_q);k++)
					   {
						  for(int j=0;j<cbuffer->send_q.q_buf[(cbuffer->send_q.tail+k)& (BUF_MAX_SIZE - 1)][0];j++)
							   printf("sendbuf[%d]=0x%02x ",j,
							   cbuffer->send_q.q_buf[(cbuffer->send_q.tail+k)& (BUF_MAX_SIZE - 1)][j]);
						  printf("length=%d\n",
						   cbuffer->send_q.q_buf[(cbuffer->send_q.tail+k)& (BUF_MAX_SIZE - 1)][0]);
					   }
#endif
					 }
//				 else
//					 printf("xor error! data missed! xorsum=0x%02x,and=0x%02x\n",
//					 		xorsum,client->c_rxBuf[frame_head + frame_len -1]);

				 frame_head += frame_len;
				}	 
			
			 pthread_mutex_unlock(&recv_eth_lock);
			 pthread_cond_signal(&has_recv_eth);
		 }
	 }
	 close(client->ClieSock);
	 return 0;
 }
 
 
 void* Task::WaitingForCan(void* ctp)
 {
	 CTP* ctp_tmp = (CTP*) ctp;
	 int fd1 = ctp_tmp->fd1;
	 //int fd2 = ctp_tmp->fd2;
//	 int frameId = ctp_tmp->frameId;
	 
	 Client* _client=ctp_tmp->_client;
	 Buffer* _cbuffer = ctp_tmp->_cbuffer;
	 
	 unsigned int ret;
	 struct can_frame fr;
 
	 while(1)
	 {
		 ret = read(fd1, &fr, sizeof(fr));//recv
		 if (ret < sizeof(fr))
			 myerr("read failed");
		 else if(fr.can_dlc>0)
		 {
#ifdef PRINT_CAN			 
		     CanComm::canPrintFrame(&fr);
#endif
			 pthread_mutex_lock(&recv_can_lock);
		/**************封装成以太网帧：报文总长+id+data+校验和 ***************/				
			 memset(_client->c_txBuf,0,BUF_LENGTH);
			 _client->c_txBuf[0]= fr.can_dlc+4;			 
			 _client->c_txBuf[1]= (fr.can_id>>8)&0xff;
			 _client->c_txBuf[2]= fr.can_id&0xff;
			 memcpy(_client->c_txBuf+3, fr.data, fr.can_dlc);
			 unsigned char xorsum=0;
			 for(int i=0;i<3+fr.can_dlc;i++)
				 xorsum = xorsum ^ _client->c_txBuf[i];
			 xorsum = xorsum & 0xff;
			 _client->c_txBuf[fr.can_dlc+3]=xorsum;
			 
			 ret=_cbuffer->PutFrame(_client->c_txBuf, &(_cbuffer->recv_q),
									 _client->c_txBuf[0]);
			 pthread_mutex_unlock(&recv_can_lock);
			 pthread_cond_signal(&has_recv_can);
#ifdef PRINT_BUFFER
		 /**************目前can接收缓冲区的内容 ***************/
			 printf("r_tail=%d\n",_cbuffer->recv_q.tail);
			 printf("r_size=%d\n",CHARS(_cbuffer->recv_q)); 			 
			 for(i=0;i<CHARS(_cbuffer->recv_q);i++)
			 {
				for(int j=0;j<_cbuffer->recv_q.q_buf[(_cbuffer->recv_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
					 printf("recvbuf[%d]=0x%02x ",j,
					  _cbuffer->recv_q.q_buf[(_cbuffer->recv_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
				printf("length=%d\n",
				  _cbuffer->recv_q.q_buf[(_cbuffer->recv_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
			 }
#endif	 
		   }			 
		 //usleep(100);
	 }
	 return NULL;
 }
 
 void* Task::MonitorSendBuffer(void* cltp)
 {
	 CLTP* cltp_temp=(CLTP* )cltp;
	 
	 CTP* canframe= cltp_temp->_pctp;
	 Buffer* cbuffer = cltp_temp->_pctp->_cbuffer;	 
 
	 struct can_frame canfr;
	 int ret=0;
	 char* s_to_c_temp;
	 s_to_c_temp=new char[FRAME_MAX_SIZE];
 
	 while(1)
		 {	
			pthread_mutex_lock(&recv_eth_lock);
			while(EMPTY(cbuffer->send_q))
			 pthread_cond_wait(&has_recv_eth,&recv_eth_lock);
			while(!EMPTY(cbuffer->send_q))
			 {
				 memset(s_to_c_temp,0,FRAME_MAX_SIZE);
				 ret=cbuffer->GetFrame(&(cbuffer->send_q), s_to_c_temp);
			 /**************拆分成can报文：id+data***************/ 
				 canfr.can_id=((s_to_c_temp[1]&0xffff)<<8|s_to_c_temp[2])+1;
				 canfr.can_dlc= ret-4	  ;
				 memset(canfr.data,0,8);
				 memcpy((void*)canfr.data, s_to_c_temp+3, canfr.can_dlc);
				 ret=write(canframe->fd1, &canfr, sizeof(canfr));
			 }
			pthread_mutex_unlock(&recv_eth_lock);
		 }
	delete[] s_to_c_temp;
	return NULL;
 }
 
void* Task::MonitorRecvBuffer(void* cltp)
{
	CLTP* cltp_temp=(CLTP* )cltp;

	Client* client = cltp_temp->_pclient;
	Buffer* cbuffer = cltp_temp->_pctp->_cbuffer;	 
	
	int ret=0;
	char* c_to_s_temp;
	c_to_s_temp=new char[BUF_LENGTH];

	while(1)
	{
		pthread_mutex_lock(&recv_can_lock);
		while(EMPTY(cbuffer->recv_q))
			pthread_cond_wait(&has_recv_can,&recv_can_lock);
		memset(c_to_s_temp,0,BUF_LENGTH);
		int len=0;
		while(!EMPTY(cbuffer->recv_q))
			{
			ret=cbuffer->GetFrame(&(cbuffer->recv_q), c_to_s_temp + len);
			len=ret + len;
			}
		client->ClientSend(client->ClieSock,c_to_s_temp,  len  );
		pthread_mutex_unlock(&recv_can_lock);
	}

	delete[] c_to_s_temp;
	return NULL;
}