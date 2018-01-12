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
    char* r_buf=new char[20]; 

	
    while(1) 
    {
		ret=client->ClientRecv(client->ClieSock);
		if(ret>0)
		{   
		    memcpy(r_buf, client->c_rxBuf, strlen(client->c_rxBuf));
//		    for(i=0;i<strlen(client->c_rxBuf);i++)
//            {
//               r_buf[i]=*(client->c_rxBuf);
//			   client->c_rxBuf++;
//			}

//			canfr.can_id=0x052;
            canfr.can_id=((r_buf[0]&0xffff)<<8|r_buf[1])+60;
			printf("can_id=%d~\r\n",canfr.can_id);

			canfr.can_dlc=strlen(client->c_rxBuf)-2;	
			printf("datalength=%d~\r\n",canfr.can_dlc);
			
//			memcpy((void*)canfr.data, &r_buf[2], canfr.can_dlc);
//			
//			ret=write(canctp->fd1, &canfr, sizeof(canfr));
			
			if(ret==sizeof(canfr))
			 {
				 printf("both send to can~\r\n");
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
				ret=_client->ClientSend(_client->ClieSock, (const char *)fr.data);
				if(ret=_client->c_tx_cryptLen)
				{
				 printf("both send out by client~\r\n");
				}	 

			}	   	    
            //usleep(100);
        }
    }
    return NULL;
}
