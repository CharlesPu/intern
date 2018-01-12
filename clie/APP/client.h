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
#include "buffer.h"
#include <pthread.h>


class Client
{
public:
    Client();
    ~Client();

    bool ClientInit(int port);

    bool ClientConnect();     //与服务器端的连接

    int ClientSend(int&, const char*,int size);//客户端的发送
    int ClientRecv(int&);//客户端的接收
    int clie_sock;
    struct sockaddr_in ServAddr;
    char* m_txBuf;//客户端发送的数据堆放空间
    char* m_rxBuf;//客户端接收的数据堆放空间
    
    char m_rx_cryptBuf[4096];//加密的接收报文
    int m_rx_cryptLen;//加密的接收报文的长度

	char m_tx_cryptBuf[4096];//加密的发送报文
	int m_tx_cryptLen;//加密的发送报文长度

	Buffer clie_buf;
	pthread_cond_t send_signal ;
	pthread_mutex_t send_lock ;	

};

#endif

