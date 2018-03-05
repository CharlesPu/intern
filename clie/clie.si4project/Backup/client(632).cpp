/*******************************
@@Author     : Charles
@@Date       : 2017-10-18
@@Mail       : pu17rui@sina.com
@@Description: class client             
*******************************/
#include "client.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "des.h"


Client::Client()
{
    m_txBuf = new char[BUF_LENGTH];
    memset(m_txBuf,0,BUF_LENGTH);

    m_rxBuf = new char[BUF_LENGTH];
    memset(m_rxBuf,0,BUF_LENGTH);
}

Client::~Client()
{
    if(m_txBuf !=NULL)
        delete[] m_txBuf;
    if(m_rxBuf !=NULL)
        delete[] m_rxBuf;
    if(clie_sock)
        close(clie_sock);
}

bool Client::ClientInit(int port )
{
    if ((clie_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Failed to obtain Socket Descriptor");
        return false;
    }

    bzero(&ServAddr, sizeof(ServAddr));
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_port = htons(port);
    //_addrLocal.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, SERV_IP, &ServAddr.sin_addr.s_addr); 
	
	int recvbuf=2000;		 //send的内核缓冲buf为4000字节
	int len = sizeof(recvbuf); 
//	setsockopt(ClieSock, SOL_SOCKET, SO_RCVBUF, &recvbuf, sizeof(recvbuf)); 
    getsockopt(clie_sock, SOL_SOCKET, SO_RCVBUF, &recvbuf, (socklen_t*)&len);  
    printf("the receive buffer size after setting is %d\n", recvbuf);

    if(!Client::ClientConnect())
		return false;

    int opts = fcntl(clie_sock,F_GETFL);
    if(opts < 0)
    {
        perror("clie fcntl failed");
    }

    opts |= O_NONBLOCK;

    if(fcntl(clie_sock,F_SETFL,opts) < 0)
    {
        perror("clie fcntl failed");
    }

	clie_buf.BufferInit();
	send_signal = PTHREAD_COND_INITIALIZER;
	send_lock = PTHREAD_MUTEX_INITIALIZER;	

    return true;
}

bool Client::ClientConnect()
{
//    if (connect(ClieSock, (struct sockaddr *)&ServAddr, sizeof(ServAddr)))
//    {
//        printf("ERROR: Failed to connect!\n");
//        return false;
//    }else
		while(connect(clie_sock, (struct sockaddr *)&ServAddr, sizeof(ServAddr)));
        printf("OK: Client has connected to %s.\n",inet_ntoa(ServAddr.sin_addr));
	
    return true;
}
int Client::ClientRecv(int &csock)
{
	memset(m_rxBuf,0,BUF_LENGTH);
#ifdef DES
	/****************报文解密*********/
	memset(m_rx_cryptBuf,0,BUF_LENGTH);
    int ret = recv(csock, m_rx_cryptBuf, BUF_LENGTH,0);
	int tem=0;
	if(ret<0)
	{
//		printf("client read erro!\r\n");
		return -1;	
	}else if(ret > 0)
	{
	    int flag=0;
		flag= DesDec(
		       (unsigned char*)m_rx_cryptBuf,
		       ret,
		       (unsigned char*)m_rxBuf,
		       &tem);
		if(flag!=0)
		{
		    printf("DEC fail!\r\n");
		    return -1;
		}//else printf("DEC success!\r\n");
#ifdef PRINT_ULTI_SND_REC
	/**************最初接收到的帧，调试用 ***************/ 
		printf("client_rec_data:");//将从服务器读到的数据，在屏幕上输出
		for (int i = 0; i < tem; i++)
			printf("0x%02x ",m_rxBuf[i]);
		printf("\n");
#endif
	}
		return tem;
	
#else

	int ret = read(csock, m_rxBuf, BUF_LENGTH);
	if(ret<0)
	{
//		printf("client read erro!\r\n");
		return -1;	
	}else if(ret >0)
	{
#ifdef PRINT_ULTI_SND_REC
	/**************最初接收到的帧，调试用 ***************/
		printf("client_rec_data:");//将从服务器读到的数据，在屏幕上输出
		for (int i = 0; i < ret; i++)
			printf("0x%02x ",m_rxBuf[i]);
		printf("\n");
#endif
	}
	return ret;
#endif
}
int Client::ClientSend(int &csock, const char *sendbuf,int size)
{
	int ret=0;
#ifdef DES
	int flag=0;
	memset(m_tx_cryptBuf,0,sizeof(m_tx_cryptBuf));
	/************报文加密*************/
	flag= DesEnc(
		(unsigned char*)sendbuf,
		size,
		(unsigned char*)m_tx_cryptBuf,
		&m_tx_cryptLen);
	if(flag!=0)
	{
		printf("ENC fail!\r\n");
		return NULL;
	}//else printf("ENC success!\r\n");
	ret=send(csock, m_tx_cryptBuf, m_tx_cryptLen,0); 
#ifdef PRINT_ULTI_SND_REC
	if(ret==m_tx_cryptLen)
		{
		/**************最终发出去的帧，调试用 ***************/	
			printf("client_send_data:");
			for(int i=0; i<  ret ;i++)
			   printf("0x%02x ",m_tx_cryptBuf[i]);
			printf("\n");
			printf("send out by client SUCCESS!\r\n");	
		}
	else
		printf("send out by client FAIL!\r\n");
#endif

#else 
	ret=send(csock, sendbuf, size,0); 
#ifdef PRINT_ULTI_SND_REC
	if(ret==size)
		{
		/**************最终发出去的帧，调试用 ***************/	
			printf("client_send_data:");
			for(int i=0; i<  ret ;i++)
			   printf("0x%02x ",sendbuf[i]);
			printf("\n");
			printf("send out by client SUCCESS!\r\n");	
		}
	else
		printf("send out by client FAIL!\r\n");	
#endif
#endif	
	return ret;
}

