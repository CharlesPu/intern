#include <pthread.h>
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "canComm.h"
#include "messages.h"
#include "des.h"
#include "serial.h"
vector<string> vecStr;
//STP* timStp;
struct can_frame timFr_tcp;

void* setTimer(void* tim)
{
    int i=0;
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

void* Test_uart(void* uart)
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

void* WaitingForServer(void* csock)
{
    CLTP* cltp = (CLTP*)csock;
	Client* client=cltp->_pclient;
	CTP* canctp= cltp->_pctp;
	struct can_frame canfr;
	int ret;
    char buf[BUF_LENGTH]; 
	
    while(1) 
    {
		ret=client->ClientRecv(client->ClieSock);
		if(ret>0)
		{
			canfr.can_id=0x052;
			canfr.can_dlc=strlen(client->c_rxBuf);		
			memcpy((void*)canfr.data, client->c_rxBuf, canfr.can_dlc);
			
			ret=write(canctp->fd1, &canfr, sizeof(canfr));
			
			if(ret==sizeof(canfr))
			 {
				 printf("both send to can~\r\n");
			 }
		}
	 
    }
	close(client->ClieSock);
    return 0;
}
#if 0
void* WaitingForClients(void* sock)
{
    STP* stp = (STP*)sock;
    timStp=stp;
    Server* server = stp->_pServer;
	
    CTP* canctp= stp->_pCtp;
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
                    server->_ev.events = EPOLLIN | EPOLLERR |EPOLLHUP;
                    epoll_ctl(server->_epfd,EPOLL_CTL_ADD,server->_ClientSock,&server->_ev);
                    continue;
                }
            }

            if (server->_events[i].events & EPOLLIN)
            {
                int st = server->_events[i].data.fd;
                int ret = server->SocketRecv(st);//client send data
                int flag=0;
                if(ret <= 0)
                {
                    close(st);
                    server->_events[i].data.fd = -1;
                    continue;
                }

			canfr.can_id=0x023;
			canfr.can_dlc=strlen(server->m_rxBuf);		
			memcpy((void*)canfr.data, server->m_rxBuf, canfr.can_dlc);

			ret=write(canctp->fd1, &canfr, sizeof(canfr));

			if(ret=sizeof(canfr))
			 {
				 printf("both send to can~\r\n");
			 }	 

                //server->SocketSend(st,server->m_rxBuf);
                //cout << server->m_rxBuf<<endl;//$)Ae0??e."f?g+???0g??0f?o<??e1??d8????              
              #if 0
                /************decEN*************/
                flag= DesEnc(
                        (unsigned char*)server->m_rxBuf,
                        strlen(server->m_rxBuf),
                        (unsigned char*)server->m_cryptBuf,
                        &server->m_cryptLen);
                if(flag!=0)
                {
                    cout<<"???$)Ae$1h4%"<<endl;
                    return NULL;
                }
          

                server->SocketSend(st,server->m_cryptBuf);//$)Ae0??e."f?g+???0g??0f?o<?????e."f?g+?               
                /*struct can_frame fr_tcp;
                fr_tcp.can_id = 0x985;
                fr_tcp.can_dlc = strlen(server->m_rxBuf);
                */
                /****************decDEC*********/
                flag= DesDec(
                       (unsigned char*)server->m_cryptBuf,
                       server->m_cryptLen,
                       (unsigned char*)server->m_rxBuf,
                       &ret);
                if(flag!=0)
                {
                    cout<<"$)Ah'#e?e$1h4%"<<endl;
                    return NULL;
                }
        
                #endif
               // timFr_tcp.can_id=0x18;
                //timFr_tcp.can_dlc=strlen(server->m_rxBuf);
               // timFr_tcp.can_dlc=ret;
               // memset((void*)timFr_tcp.data,0,sizeof(timFr_tcp.data));
               // memcpy((void*)timFr_tcp.data, server->m_rxBuf, timFr_tcp.can_dlc);

                /*memset((void*)fr_tcp.data,0,sizeof(fr_tcp.data));

                memcpy((void*)fr_tcp.data, server->m_rxBuf, fr_tcp.can_dlc);

                write(stp->_pCtp->fd1,&fr_tcp,sizeof(fr_tcp));*/
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
        usleep(100);
    }
    return NULL;
}
#endif
	
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
  /*  snprintf(header,sizeof(header),"\nrecv: can.id=0x%03x  dlc = %d  data = ",
             fr.can_id & CAN_EFF_MASK,
             fr.can_dlc);   */
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

void* WaitingForCan(void* ctp)
{
    CTP* ctp_tmp = (CTP*)ctp;
    int fd1 = ctp_tmp->fd1;
    //int fd2 = ctp_tmp->fd2;
    int frameId = ctp_tmp->frameId;
    int master = ctp_tmp->master;
	
    //Server* _server = ctp_tmp->_server;
    Client* _client=ctp_tmp->_client;
	
    int ret, i, j;
    struct can_frame fr, frdup;
    struct timeval tv;
    fd_set rset;
    //EB 90 EB 90 EB 90 05 00 00 02 00 00 17 00 0C 00 2A 00 EB 90 EB 90 05 00
    char 	buff1[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#if 0
    char 	buff2[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    char 	buff3[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#endif

    unsigned char id = 0;
    unsigned char val = 0;

    string finalMsg;

    while(1)
    {
        finalMsg = "";
        for(i = 0; i < master; i++)
        {
           /* if(frameId > 0)
                frdup.can_id = frameId;
            else
                frdup.can_id = id++;
            frdup.can_dlc  = 8;

            for(j = 0; j < 8; j++)
                buff1[j] = val++;

            memcpy((void*)frdup.data, buff1, 8);*/

           /* ret = write(fd1, &frdup, sizeof(frdup));//send
            if(ret <= 0)
                {
                    printf("socket can write return error!%d,%s\n", ret, strerror(errno));
                }
            */
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
				ret=_client->ClientSend(_client->ClieSock, (const char *)fr.data);
				if(ret=sizeof((const char *)fr.data))
				{
				 printf("both send out by client~\r\n");
				}	 

			}	   	    
            //usleep(100);
        }
    }
    return NULL;
}
