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

    bool ClientConnect();

    int ClientSend(int&, const char*);
    int ClientRecv(int&);

public:
    int ClieSock;
    struct sockaddr_in ServAddr;
    char* c_txBuf;//发送的数据
    char* c_rxBuf;//服务器接收的数据放堆空间

};

#endif

