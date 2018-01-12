/*******************************
@@Author     : Charles
@@Date       : 2017-10-18
@@Mail       : pu17rui@sina.com
@@Description: declaration of class client             
*******************************/
#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <sys/epoll.h>

class Client
{
public:
    Client();
    ~Client();

    bool ClientInit(int port);

    bool ClientConnect();     //与服务器端的连接

    int ClientSend(int&, const char*,int size);//客户端的发送
    int ClientRecv(int&);//客户端的接收
    int ClieSock;
    struct sockaddr_in ServAddr;
    char* c_txBuf;//客户端发送的数据堆放空间
    char* c_rxBuf;//客户端接收的数据堆放空间
    
    char c_rx_cryptBuf[4096];//加密的接收报文
    int c_rx_cryptLen;//加密的接收报文的长度

	char c_tx_cryptBuf[4096];//加密的发送报文
	int c_tx_cryptLen;//加密的发送报文长度

};

#endif

