#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <sys/epoll.h>

class Server
{
public:
    Server();
    ~Server();

    bool ServerInit(int port);

    static void SetNonblocking(int& st);//设置非阻塞读，加快读取速度

    bool SocketAccept();     //与客户端的连接

    int SocketSend(int&, char*,int );//服务器端的发送
    int SocketRecv(int&);//服务器端的接收

public:
    int _ServerSock;
    int _ClientSock;
    struct sockaddr_in _addrLocal;
    struct sockaddr_in _addrClient;
    char* m_txBuf;//服务器发送的数据堆放空间
    char* m_rxBuf;//服务器接收的数据堆放空间
    
    char m_rx_cryptBuf[4096];//加密的接收报文
    int m_rx_cryptLen;//加密的接收报文的长度

	char m_tx_cryptBuf[4096];//加密的发送报文
	int m_tx_cryptLen;//加密的发送报文长度
    int _epfd;

    struct epoll_event _ev;
    struct epoll_event _events[10];

    int _clientNum;
};

#endif // SERVER_H
