#include "client.h"
#include "common.h"

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
int Client::ClientSend(int &csock, const char *sendbuf)
{
	return write(csock, sendbuf, strlen(sendbuf));  
}
int Client::ClientRecv(int &csock)
{
    memset(c_rxBuf,0,strlen(c_rxBuf));
    int ret = read(csock, c_rxBuf, BUF_LENGTH);
	if(ret<0)
	{
		printf("client read erro!\r\n");
		return -1;	
	}else 
	if(ret>0)
	{
		printf("client_rec_data:%s\n",c_rxBuf);//将从客户端读到的数据，在屏幕上输出
		return ret;
	}
}
