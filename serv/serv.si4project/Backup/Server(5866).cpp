#include "Server.h"
#include "common.h"

Server::Server()
{
    m_txBuf = new char[BUF_LENGTH];
    memset(m_txBuf,0,BUF_LENGTH);
    sprintf(m_txBuf,"hello world\n");
    m_rxBuf = new char[BUF_LENGTH];
    memset(m_rxBuf,0,BUF_LENGTH);
    _clientNum = 0;
    m_cryptBuf[4096]={0};
    m_cryptLen=0;
}

Server::~Server()
{
    if(m_txBuf !=NULL)
        delete[] m_txBuf;
    if(m_rxBuf !=NULL)
        delete[] m_rxBuf;
    if(_ServerSock)
        close(_ServerSock);
    if(_ClientSock)
        close(_ClientSock);
    _clientNum = 0;
}

void Server::SetNonblocking(int& st)
{
    //flag=fcntl(cfd,F_GETFL); /*修改cfd为非阻塞读*/
    //flag|=O_NONBLOCK;
    //fcntl(cfd,F_SERFL,flag);
    int opts = fcntl(st,F_GETFL);
    if(opts < 0)
    {
        printf("fcntl failed %s\n",strerror(errno));
        return;
    }

    opts |= O_NONBLOCK;

    if(fcntl(st,F_SETFL,opts) < 0)
    {
        printf("fcntl failed %s\n",strerror(errno));
        return;
    }
}

bool Server::ServerInit(int port = 5000)
{
    _addrLocal.sin_family = AF_INET;
    _addrLocal.sin_port = htons(port);
    //_addrLocal.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, "192.168.2.165", &_addrLocal.sin_addr.s_addr); 

    bzero(&(_addrLocal.sin_zero),8);

    _epfd = epoll_create(100);//max client 100

    if ((_ServerSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        puts("ERROR: Failed to obtain Socket Descriptor.");
        return false;
    }

    int opt=1;
    setsockopt(_ServerSock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    if (bind(_ServerSock, (struct sockaddr*) &_addrLocal,sizeof(struct sockaddr)) == -1)
    {
        close(_ServerSock);
        printf("ERROR: Failed to bind Port %d.\n", port);
        return false;
    }

    if (listen(_ServerSock, BACKLOG) == -1)
    {
        close(_ServerSock);
        printf("ERROR: Failed to listen Port %d.\n", PORT);
        return false;
    }

    Server::SetNonblocking(_ServerSock);
    _ev.data.fd = _ServerSock;
    _ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
    epoll_ctl(_epfd,EPOLL_CTL_ADD,_ServerSock,&_ev);

    return true;
}

bool Server::SocketAccept()
{
    int sin_size;
    if ((_ClientSock = accept(_ServerSock,(struct sockaddr *)&_addrClient,(socklen_t *)&sin_size)) < 0)
    {
        puts("ERROR: Failed to obtain transmission Socket Descriptor.");
        return false;
    }
    else
        printf("OK: Server has got connection from %s.\n",inet_ntoa(_addrClient.sin_addr));
    return true;
}

int Server::SocketRecv(int& sock)
{
    memset(m_rxBuf,0,strlen(m_rxBuf));
    int ret = recv(sock, m_rxBuf, BUF_LENGTH, 0);
	if(ret<0)
	{
		printf("server read erro!\r\n");
		return -1;	
	}else 
	if(ret>0)
	{
		printf("server_rec_data:%s\n",m_rxBuf);//将从客户端读到的数据，在屏幕上输出
		return ret;
	}
}

int Server::SocketSend(int& sock,const char* sendBuf)
{
    return send(sock, sendBuf, strlen(sendBuf), 0);
}

