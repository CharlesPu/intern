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
 
 vector<string> vecStr;
 struct can_frame timFr_tcp;
 
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
	 
 void* Task::WaitingForServer(void* csock)
 {
	 CLTP* cltp = (CLTP*)csock;
	 Client* client=cltp->_pclient;
	 CTP* canctp= cltp->_pctp;
	 Buffer* cbuffer = cltp->_pctp->_cbuffer;
	 
	 struct can_frame canfr;
	 int ret,i;
	 
	 while(1) 
	 {
		 ret=client->ClientRecv(client->ClieSock);
		 if(ret>0)
		 {	 
		   /**************拆分成can报文：id+data+校验和 ***************/
			int frame_head=0;
		    int frame_len=0;
		    while(client->c_rxBuf[frame_head] != 0)
				{
				 char xorsum=0;
				 frame_len = client->c_rxBuf[frame_head];
				 for(int j=0;j< frame_len - 1;j++)
					 xorsum = xorsum ^ client->c_rxBuf[frame_head+j];
				 if(xorsum == client->c_rxBuf[frame_head + frame_len -1])
					 {
					   ret=cbuffer->PutFrame(&(client->c_rxBuf[frame_head]), 
					   					&(cbuffer->send_q),frame_len);
#if 0
			 /**************目前can发送缓冲区的内容 ***************/
//						   printf("s_tail=%d\n",cbuffer->send_q.tail);
					   printf("s_size=%d\n",CHARS(cbuffer->send_q));
					   for(int k=0;k<CHARS(cbuffer->send_q);k++)
					   {
						  for(int j=0;j<cbuffer->send_q.q_buf[(cbuffer->send_q.tail+k)& (BUF_MAX_SIZE - 1)][0];j++)
							   printf("sendbuf[%d]=0x%02x ",j,
							   cbuffer->send_q.q_buf[(cbuffer->send_q.tail+k)& (BUF_MAX_SIZE - 1)][j]);
						  printf("length=%d",
						   cbuffer->send_q.q_buf[(cbuffer->send_q.tail+k)& (BUF_MAX_SIZE - 1)][0]);
						  printf("\n");
					   }
#endif
					 }
				 else
					 printf("xor error! data missed!\n");

				 frame_head += frame_len;
				}
		 }
	 }
	 close(client->ClieSock);
	 return 0;
 }
 
	 
 void canPrintFrame(struct can_frame& fr)
 {
	 ECM_HSC1_FrP02_DATA_t ECM_HSC1_FrP02_DATA;
	 int i;
	 if(fr.can_id==ECM_HSC1_FrP02_t)
	 {
		printf("recv: ECM_HSC1_FrP02 message!!!\n");
		for(i=0;i<fr.can_dlc;i++)
		  {
			ECM_HSC1_FrP02_DATA.d_u8[i]=fr.data[i];
		   }
		printf("singal: %x\n",CrkshftNonTrRegdCmddTHSC1());
	 }
	 printf("recv: can.id=0x%03x  ", fr.can_id & CAN_EFF_MASK);
	 printf("dlc = %d  ", fr.can_dlc);
	 printf("data = ");
	 for (i = 0; i < fr.can_dlc; i++)
		 printf("0x%02x ", fr.data[i]);
	 printf("\n");
 }
 
 void canPerFrame(struct can_frame& fr)
 {
	 char header[1024];
	 memset(header,0,sizeof(header));
   /*  snprintf(header,sizeof(header),"\nrecv: can.id=0x%03x  dlc = %d	data = ",
			  fr.can_id & CAN_EFF_MASK,
			  fr.can_dlc);	 */
	 string canMsgHeader(header);
	 vecStr.push_back(canMsgHeader);
 
	 for(int i =0 ;i<fr.can_dlc;i++)
	 {
		 char temp[8];
		 sprintf(temp,"%02x",fr.data[i]);
		 string strTmp(temp);
	 //    memcpy((void*), &fr.data[i], 8);
		 vecStr.push_back(strTmp);
	 }
 }
 
 void* Task::WaitingForCan(void* ctp)
 {
	 CTP* ctp_tmp = (CTP*) ctp;
	 int fd1 = ctp_tmp->fd1;
	 //int fd2 = ctp_tmp->fd2;
	 int frameId = ctp_tmp->frameId;
	 int master = ctp_tmp->master;
	 
	 Client* _client=ctp_tmp->_client;
	 Buffer* _cbuffer = ctp_tmp->_cbuffer;
	 
	 int ret, i, j;
	 struct can_frame fr, frdup;
 
	 string finalMsg;
 
	 while(1)
	 {
		 finalMsg = "";
		 //for(i = 0; i < master; i++)
		 {
			 ret = read(fd1, &fr, sizeof(fr));//recv
			 if (ret < sizeof(fr))
			 {
				 myerr("read failed");
			 }else
			 if(fr.can_dlc>0)
			 {
				 canPrintFrame(fr);
				 /*
				 canPerFrame(fr);
				 for(int i=0;i<vecStr.size();i++)
				 {
				  finalMsg += vecStr[i];
				 }
				 cout << finalMsg;
				 if(_server->_ClientSock > 0)
				 {
				  _server->SocketSend(_server->_ClientSock,finalMsg.data());
				 }			 
				 vecStr.clear();
				 */
			 /**************封装成以太网帧：dlc+id+data ***************/
				 memset(_client->c_txBuf,0,BUF_LENGTH);
				 _client->c_txBuf[0]= fr.can_dlc+4;			 
				 _client->c_txBuf[1]= (fr.can_id>>8)&0xff;
				 _client->c_txBuf[2]= fr.can_id&0xff;
				 memcpy(_client->c_txBuf+3, fr.data, fr.can_dlc);
			 /**************封装成以太网帧：dlc+id+data+校验和 ***************/				 
				 char xorsum=0;
				 for(i=0;i<3+fr.can_dlc;i++)
					 xorsum = xorsum ^ _client->c_txBuf[i];
				 _client->c_txBuf[fr.can_dlc+3]=xorsum;
				 
				 ret=_cbuffer->PutFrame(_client->c_txBuf, &(_cbuffer->recv_q),
										 _client->c_txBuf[0]);
			#if 0
			 /**************目前can接收缓冲区的内容 ***************/
				 printf("r_tail=%d\n",_cbuffer->recv_q.tail);
				 printf("r_size=%d\n",CHARS(_cbuffer->recv_q)); 			 
				 for(i=0;i<CHARS(_cbuffer->recv_q);i++)
				 {
					for(int j=0;j<ret;j++)
						 printf("recvbuf[%d]=0x%02x ",j,
						  _cbuffer->recv_q.q_buf_cell[(_cbuffer->recv_q.tail+i)& (BUF_MAX_SIZE - 1)].q_buf[j]);
					printf("length=%d",
					  _cbuffer->recv_q.q_buf_cell[(_cbuffer->recv_q.tail+i)& (BUF_MAX_SIZE - 1)].q_length);
					printf("\n");
				 }
			#endif	 
			 }			 
			 //usleep(100);
		 }
	 }
	 return NULL;
 }
 
 void* Task::MonitorBuffer(void* cltp)
 {
	 CLTP* cltp_temp=(CLTP* )cltp;
	 
	 Client* client = cltp_temp->_pclient;
	 CTP* canframe= cltp_temp->_pctp;
	 Buffer* sbuffer = cltp_temp->_pctp->_cbuffer;	 
 
	 struct can_frame canfr;
	 int ret=0;
	 char* s_to_c_temp;
	 s_to_c_temp=new char[FRAME_MAX_SIZE];
	 char* c_to_s_temp;
	 c_to_s_temp=new char[FRAME_MAX_SIZE];
 
	 while(1)
		 {
		 if(!EMPTY(sbuffer->recv_q))
			 {	 
				 memset(c_to_s_temp,0,FRAME_MAX_SIZE);
				 ret=sbuffer->GetFrame(&(sbuffer->recv_q), c_to_s_temp );
				 client->ClientSend(client->ClieSock,c_to_s_temp,  ret	);		 
			 /**************最终发出去的帧，调试用 ***************/	 
 // 			 for(int i=0;i<   ;i++)
 // 			 {
 // 				printf("client->c_txBuf[%d]=%d\r\n",i,c_to_s_temp[i]);
 // 			 }
				 printf("***************************************************\r\n"); 		 
			 }
		 if(!EMPTY(sbuffer->send_q))
			 {	 
				 memset(s_to_c_temp,0,FRAME_MAX_SIZE);
				 ret=sbuffer->GetFrame(&(sbuffer->send_q), s_to_c_temp);
			 /**************拆分成can报文：id+data***************/ 
				 canfr.can_id=((s_to_c_temp[1]&0xffff)<<8|s_to_c_temp[2])+1;
				 canfr.can_dlc= ret-4	 ;
				 memset(canfr.data,0,8);
				 memcpy((void*)canfr.data, s_to_c_temp+3, canfr.can_dlc);
				 ret=write(canframe->fd1, &canfr, sizeof(canfr));
				 if(ret=sizeof(canfr))
					 printf("both send to can~\r\n");
//				 printf("***************************************************\r\n"); 		 
			 }		 
		 }
	delete[] s_to_c_temp;
	delete[] c_to_s_temp;
	return NULL;
 }
 
 
