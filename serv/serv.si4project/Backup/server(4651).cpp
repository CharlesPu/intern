#include "server.h"
#include "common.h"
#include "des.h"
#include "buffer.h"

Server::Server()
{
    m_txBuf = new char[BUF_LENGTH];
    memset(m_txBuf,0,BUF_LENGTH);
    m_rxBuf = new char[BUF_LENGTH];
    memset(m_rxBuf,0,BUF_LENGTH);
	
    _clientNum = 0;
	memset(m_rx_cryptBuf,0,BUF_LENGTH);
    m_rx_cryptLen=0;
	memset(m_tx_cryptBuf,0,BUF_LENGTH);
    m_tx_cryptLen=0;	
}

Server::~Server()
{
    if(m_txBuf !=NULL)
        delete[] m_txBuf;
    if(m_rxBuf !=NULL)
        delete[] m_rxBuf;
//    if(m_tx_cryptBuf !=NULL)
//        delete[] m_tx_cryptBuf;
//    if(m_rx_cryptBuf !=NULL)
//        delete[] m_rx_cryptBuf;
	memset(m_rx_cryptBuf,0,BUF_LENGTH);
	m_rx_cryptLen=0;
	memset(m_tx_cryptBuf,0,BUF_LENGTH);
	m_tx_cryptLen=0;


    if(serv_sock)
        close(serv_sock);
    if(clie_sock)
        close(clie_sock);
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

bool Server::ServerInit(int port )
{
    _addrLocal.sin_family = AF_INET;
    _addrLocal.sin_port = htons(port);
    //_addrLocal.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, SERV_IP, &_addrLocal.sin_addr.s_addr); 

    bzero(&(_addrLocal.sin_zero),8);

    _epfd = epoll_create(100);//max client 100

    if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        puts("ERROR: Failed to obtain Socket Descriptor.");
        return false;
    }

    int opt=1;
    setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	
	int recvbuf=2000;        //send的内核缓冲buf为4000字节
	int len = sizeof(recvbuf); 
//	setsockopt(_ServerSock, SOL_SOCKET, SO_SNDBUF, &recvbuf, sizeof(recvbuf)); 
    getsockopt(serv_sock, SOL_SOCKET, SO_SNDBUF, &recvbuf, (socklen_t*)&len);  
    printf("the send buffer size after setting is %d\n", recvbuf);

    if (bind(serv_sock, (struct sockaddr*) &_addrLocal,sizeof(struct sockaddr)) == -1)
    {
        close(serv_sock);
        printf("ERROR: Failed to bind Port %d.\n", port);
        return false;
    }

    if (listen(serv_sock, BACKLOG) == -1)
    {
        close(serv_sock);
        printf("ERROR: Failed to listen Port %d.\n", PORT);
        return false;
    }

    Server::SetNonblocking(serv_sock);
    _ev.data.fd = serv_sock;
    _ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
    epoll_ctl(_epfd,EPOLL_CTL_ADD,serv_sock,&_ev);
	
	serv_buf.BufferInit();
    return true;
}

bool Server::ServerAccept(void)
{
    int sin_size;
    if ((clie_sock = accept(serv_sock,(struct sockaddr *)&_addrClient,(socklen_t *)&sin_size)) < 0)
    {
        puts("ERROR: Failed to obtain transmission Socket Descriptor.");
        return false;
    }
    else
        printf("OK: Server has got connection from %s.\n",inet_ntoa(_addrClient.sin_addr));
    return true;
}

int Server::ServerRecv(int& sock)
{
#ifdef DES
	/****************报文解密*********/
	memset(m_rx_cryptBuf,0,sizeof(m_rx_cryptBuf));
	memset(m_rxBuf,0,sizeof(m_rxBuf)); 
	int ret = recv(sock, m_rx_cryptBuf, BUF_LENGTH, 0);
	if(ret<0)
	{
		printf("server read erro!\r\n");
		return -1;	
	}else 
	{
		int tem=0;
	    int flag=0;

		flag= DesDec(
		       (unsigned char*)m_rx_cryptBuf,
		       ret,
		       (unsigned char*)m_rxBuf,
		       &tem);
	   if(flag!=0)
	   {
		   printf("DEC fail!\r\n");
		   return NULL;
	   }//else printf("DEC success!\r\n");
#ifdef PRINT_ULTI_SND_REC
	 /**************最初接收到的帧，调试用 ***************/ 
		printf("server_rec_data:");//将从客户端读到的数据，在屏幕上输出
		for (int i = 0; i < tem; i++)
			printf("0x%02x ",m_rxBuf[i]);
		printf("\n");
#endif
		return tem;
	 }
#else
	memset(m_rxBuf,0,sizeof(m_rxBuf)); 
	int ret = recv(sock, m_rxBuf, BUF_LENGTH, 0);
	if(ret<0)
	{
		printf("server read erro!\r\n");
		return -1;	
	}else 
	 {
#ifdef PRINT_ULTI_SND_REC
	/**************最初接收到的帧，调试用 ***************/ 
		printf("server_rec_data:");//将从客户端读到的数据，在屏幕上输出
		for (int i = 0; i < ret; i++)
			printf("0x%02x ",m_rxBuf[i]);
		printf("\n");
#endif
		return ret;
	 }
#endif	
}

int Server::ServerSend(int& sock,char* sendBuf,int size)
{
	int ret=0;
#ifdef DES
	/************报文加密*************/
	memset(m_tx_cryptBuf,0,sizeof(m_tx_cryptBuf));
	int flag=0;
	flag= DesEnc(
			(unsigned char*)sendBuf,
			m_tx_cryptLen,
			(unsigned char*)m_tx_cryptBuf,
			&m_tx_cryptLen);
	if(flag!=0)
	{
		printf("ENC fail!\r\n");
		return NULL;
	}//else printf("ENC success!\r\n");
    ret=send(sock, m_tx_cryptBuf, m_tx_cryptLen, 0);
#ifdef PRINT_ULTI_SND_REC
	if(ret==m_tx_cryptLen)
		{
		/**************最终发出去的帧，调试用 ***************/	
			printf("server_send_data:");
			for(int i=0; i<  ret ;i++)
			   printf("0x%02x ",m_tx_cryptBuf[i]);
			printf("\n");
			printf("send out by server SUCCESS!\r\n");	
		}
	else
		printf("send out by server FAIL!\r\n");
#endif

#else
    ret=send(sock, sendBuf, size, 0);
#ifdef PRINT_ULTI_SND_REC
	if(ret==size)
		{
		/**************最终发出去的帧，调试用 ***************/	
			printf("server_send_data:");
			for(int i=0; i<  ret ;i++)
			   printf("0x%02x ",sendBuf[i]);
			printf("\n");
			printf("send out by server SUCCESS!\r\n");
		}
	else
		printf("send out by server FAIL!\r\n");
#endif
#endif
	return ret;
}

