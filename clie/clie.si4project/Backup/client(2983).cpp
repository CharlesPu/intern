#include "client.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "des.h"


Client::Client()
{
    c_txBuf = new char[BUF_LENGTH];
    memset(c_txBuf,0,BUF_LENGTH);
    sprintf(c_txBuf,"hello world\n");
    c_rxBuf = new char[BUF_LENGTH];
    memset(c_rxBuf,0,BUF_LENGTH);
}

Client::~Client()
{
    if(c_txBuf !=NULL)
        delete[] c_txBuf;
    if(c_rxBuf !=NULL)
        delete[] c_rxBuf;
    if(ClieSock)
        close(ClieSock);
}

bool Client::ClientInit(int port )
{
    if ((ClieSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        puts("ERROR: Failed to obtain Socket Descriptor.");
        return false;
    }

    bzero(&ServAddr, sizeof(ServAddr));
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_port = htons(port);
    //_addrLocal.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, "192.168.2.165", &ServAddr.sin_addr.s_addr); 

    if(!Client::ClientConnect())
		return false;
    return true;
}

bool Client::ClientConnect()
{
    if (connect(ClieSock, (struct sockaddr *)&ServAddr, sizeof(ServAddr)))
    {
        printf("ERROR: Failed to connect!\n");
        return false;
    }else
        printf("OK: Client has connected to %s.\n",inet_ntoa(ServAddr.sin_addr));
	
    return true;
}
int Client::ClientRecv(int &csock)
{
    memset(c_rx_cryptBuf,0,sizeof(c_rx_cryptBuf));
    int ret = read(csock, c_rx_cryptBuf, BUF_LENGTH);
	if(ret<0)
	{
		printf("client read erro!\r\n");
		return -1;	
	}else 
	if(ret>0)
	{
	    #if 1
	    int flag=0;
	   /****************报文解密*********/
		flag= DesDec(
		       (unsigned char*)c_rx_cryptBuf,
		       ret,
		       (unsigned char*)c_rxBuf,
		       &ret);
		if(flag!=0)
		{
		    printf("DEC fail!\r\n");
		    return NULL;
		}else printf("DEC success!\r\n");
		#endif
		//printf("client_rec_data:%s\n",c_rxBuf);//将从服务器读到的数据，在屏幕上输出
		printf("client_rec_data:");//将从服务器读到的数据，在屏幕上输出
		for (int i = 0; i < ret; i++)
			printf("0x%x ",c_rxBuf[i]);
		printf("\n");
		return ret;
	}
}
int Client::ClientSend(int &csock, const char *sendbuf,int size)
{
    int flag=0;
	int ret=0;
	memset(c_tx_cryptBuf,0,sizeof(c_tx_cryptBuf));
	/************报文加密*************/
	flag= DesEnc(
		(unsigned char*)sendbuf,
		size,
		(unsigned char*)c_tx_cryptBuf,
		&c_tx_cryptLen);
	if(flag!=0)
	{
		printf("ENC fail!\r\n");
		return NULL;
	}else printf("ENC success!\r\n");
	ret=write(csock, c_tx_cryptBuf, c_tx_cryptLen); 
	if(ret==c_tx_cryptLen)
		printf("send out by client SUCCESS!\r\n");
	else
		printf("send out by client FAIL!\r\n");
	
	return ret;
}

