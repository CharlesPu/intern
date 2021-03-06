/*******************************
@@Author     : Charles
@@Date       : 2017-10-18
@@Mail       : pu17rui@sina.com
@@Description: definition of all tasks             
*******************************/
#include <pthread.h>
#include "client.h"
#include "common.h"
#include "cancomm.h"
#include "des.h"
#include "buffer.h"
#include "task.h"

void* Task::CanReceive(void* can_eth_para)
 {
	CanEthPara* can_eth_para_temp = (CanEthPara*)can_eth_para;
	int fd = can_eth_para_temp->_can->can_sock;
	int fd_another = can_eth_para_temp->_another_can->can_sock;

	Buffer* _can_buf = &(can_eth_para_temp->_can->can_buf);
	Buffer* another_can_buf = &(can_eth_para_temp->_another_can->can_buf);
	
	Buffer* _clie_buf = &(can_eth_para_temp->_client->clie_buf);

	unsigned int ret;
	struct can_frame fr;
	char* temp= new char[FRAME_MAX_SIZE];

	struct can_frame fr2;
	char* temp2= new char[FRAME_MAX_SIZE];

	while(1)
	{
		ret = read(fd, &fr, sizeof(fr));//recv
		if (ret < sizeof(fr))
			perror("read failed");
		else if(fr.can_dlc>0)
		{
#ifdef PRINT_CAN        	
			CanComm::canPrintFrame(&fr);
#endif
			/**************封装成以太网帧：报文总长+id+data+异或校验和 ***************/
			memset(temp,0,FRAME_MAX_SIZE);
			temp[0]= fr.can_dlc+4;		//帧头为一帧总长
			temp[1]= (fr.can_id>>8)&0xff;
			temp[2]= fr.can_id&0xff;
			memcpy(temp+3, fr.data, fr.can_dlc);
	
			unsigned char xorsum=0;
			for(int i=0;i<3+fr.can_dlc;i++)
				xorsum = xorsum ^ temp[i];
			xorsum = xorsum & 0xff;
			temp[fr.can_dlc+3]=xorsum;

			if(IsBelongTo(fr.can_id, (void*)can_eth_para_temp->_another_can))//如果要发给另一个can
			{		
				pthread_mutex_lock(&(can_eth_para_temp->_another_can->send_lock));				
				int r=another_can_buf->PutFrame(temp, &(another_can_buf->send_q),temp[0]);
#ifdef PRINT_PROCESS
				printf("another can's frame:");//将从客户端读到的数据，在屏幕上输出
				for (int i = 0; i < r; i++)
					printf("0x%02x ",temp[i]);
				printf("\n");
#endif
				pthread_mutex_unlock(&(can_eth_para_temp->_another_can->send_lock));
				pthread_cond_signal(&(can_eth_para_temp->_another_can->send_signal));
			}else						//如果要发给以太网
			{			
				pthread_mutex_lock(&(can_eth_para_temp->_client->send_lock));
				int r=_clie_buf->PutFrame(temp, &(_clie_buf->send_q),temp[0]);
#ifdef PRINT_PROCESS
				printf("eth's frame:");//将从客户端读到的数据，在屏幕上输出
				for (int i = 0; i < r; i++)
					printf("0x%02x ",temp[i]);
				printf("\n");
#endif
				pthread_mutex_unlock(&(can_eth_para_temp->_client->send_lock));
				pthread_cond_signal(&(can_eth_para_temp->_client->send_signal));
			}
		}
/**********************************************************************************************************************************/
/**********************************************************************************************************************************/

		ret = read(fd_another, &fr2, sizeof(fr2));//recv
		if (ret < sizeof(fr2))
			perror("read2 failed");
		else if(fr2.can_dlc>0)
		{
#ifdef PRINT_CAN        	
//			CanComm::canPrintFrame(&fr2);
#endif
			/**************封装成以太网帧：报文总长+id+data+异或校验和 ***************/
			memset(temp2,0,FRAME_MAX_SIZE);
			temp2[0]= fr2.can_dlc+4;		//帧头为一帧总长
			temp2[1]= (fr2.can_id>>8)&0xff;
			temp2[2]= fr2.can_id&0xff;
			memcpy(temp2+3, fr2.data, fr2.can_dlc);
	
			unsigned char xorsum2=0;
			for(int i=0;i<3+fr2.can_dlc;i++)
				xorsum2 = xorsum2 ^ temp2[i];
			xorsum2 = xorsum2 & 0xff;
			temp2[fr2.can_dlc+3]=xorsum2;

			if(IsBelongTo(fr2.can_id, (void*)can_eth_para_temp->_can))//如果要发给另一个can
			{		
				pthread_mutex_lock(&(can_eth_para_temp->_can->send_lock));				
				int r=_can_buf->PutFrame(temp2, &(_can_buf->send_q),temp2[0]);
#ifdef PRINT_PROCESS
				printf("another2 can's frame:");//将从客户端读到的数据，在屏幕上输出
				for (int i = 0; i < r; i++)
					printf("0x%02x ",temp2[i]);
				printf("\n");
#endif
				pthread_mutex_unlock(&(can_eth_para_temp->_can->send_lock));
//				pthread_cond_signal(&(can_eth_para_temp->_can->send_signal));
			}else						//如果要发给以太网
			{			
				pthread_mutex_lock(&(can_eth_para_temp->_client->send_lock));
				int r=_clie_buf->PutFrame(temp2, &(_clie_buf->send_q),temp2[0]);
#ifdef PRINT_PROCESS
				printf("eth's frame:");//将从客户端读到的数据，在屏幕上输出
				for (int i = 0; i < r; i++)
					printf("0x%02x ",temp2[i]);
				printf("\n");
#endif
				pthread_mutex_unlock(&(can_eth_para_temp->_client->send_lock));
//				pthread_cond_signal(&(can_eth_para_temp->_client->send_signal));
			}
		}

	}
	delete[] temp;
	delete[] temp2;
	return NULL;	
 }

 void* Task::ClientReceive(void* can_eth_para)
 {
    CanEthPara* can_eth_para_temp = (CanEthPara*)can_eth_para;
	Client* clie_temp=  can_eth_para_temp->_client;
	
	Buffer* _can_buf = &(can_eth_para_temp->_can->can_buf);
	Buffer* another_can_buf = &(can_eth_para_temp->_another_can->can_buf);
	 
	 int ret;
	 
	 while(1) 
	 {
		 ret=clie_temp->ClientRecv(clie_temp->clie_sock);
		 if(ret>0)
		 {	 			
		   /**************拆分成can报文：报文总长+id+data+校验和 ***************/
			int frame_head=0;
			int frame_len=0;
			while(clie_temp->m_rxBuf[frame_head] != 0)
			{
				unsigned char xorsum=0;
				frame_len = clie_temp->m_rxBuf[frame_head];
				for(int j=0;j<frame_len -1;j++)
					xorsum = xorsum ^ clie_temp->m_rxBuf[frame_head+j];
				xorsum = xorsum & 0xff;
				if(xorsum == clie_temp->m_rxBuf[frame_head + frame_len -1])
					{					
					unsigned short id_temp
						=((clie_temp->m_rxBuf[frame_head+1]&0xffff)<<8|clie_temp->m_rxBuf[frame_head+2]);	
					if(IsBelongTo(id_temp,(void*)can_eth_para_temp->_can))//如果属于第一个can的
						{
						pthread_mutex_lock(&(can_eth_para_temp->_can->send_lock));
						int r=_can_buf->PutFrame(&(clie_temp->m_rxBuf[frame_head]), 
							&(_can_buf->send_q), frame_len);
#ifdef PRINT_PROCESS
						printf("another can's frame:");//将从客户端读到的数据，在屏幕上输出
						for (int i = 0; i < r; i++)
							printf("0x%02x ",clie_temp->m_rxBuf[frame_head+i]);
						printf("\n");
#endif							
						pthread_mutex_unlock(&(can_eth_para_temp->_can->send_lock));
//						pthread_cond_signal(&(can_eth_para_temp->_can->send_signal));
						}
					else	//如果属于另一个can的
						{
						pthread_mutex_lock(&(can_eth_para_temp->_another_can->send_lock));
						int r=another_can_buf->PutFrame(&(clie_temp->m_rxBuf[frame_head]), 
							&(another_can_buf->send_q), frame_len);
#ifdef PRINT_PROCESS
						printf("eth's frame:");//将从客户端读到的数据，在屏幕上输出
						for (int i = 0; i < r; i++)
							printf("0x%02x ",clie_temp->m_rxBuf[frame_head+i]);
						printf("\n");
#endif	
						pthread_mutex_unlock(&(can_eth_para_temp->_another_can->send_lock));
//						pthread_cond_signal(&(can_eth_para_temp->_another_can->send_signal));									
						}
					}
				else
					printf("xor error!	data missed!\n");
				frame_head += frame_len;
			}
		 }
	 }
	 close(clie_temp->clie_sock);
	 return 0;
 }
 
void* Task::CanSend(void* can)
{
	CanComm* can_temp=(CanComm*)can;
	int fd = can_temp->can_sock_send;

	Buffer* can_buf=&(can_temp->can_buf);

	struct can_frame canfr;
	char* temp;
	temp=new char[FRAME_MAX_SIZE];

	while(1)
	 {
	 pthread_mutex_lock(&(can_temp->send_lock));
	 while(EMPTY(can_buf->send_q))
		 pthread_cond_wait(&(can_temp->send_signal),&(can_temp->send_lock));
#ifdef PRINT_BUFFER
	/**************目前can发送缓冲区的内容 ***************/
	//		  printf("s2_tail=%d\n",can_buf->send_q.tail);
	//		  printf("s2_size=%d\n",CHARS(can_buf->send_q));
	  for(int i=0;i<CHARS(can_buf->send_q);i++)
	  {
		 for(int j=0;j<can_buf->send_q.q_buf[(can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
			  printf("2send_buf[%d]=0x%02x\t",j,
			   can_buf->send_q.q_buf[(can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
		 printf("2length=%d\n",
		   can_buf->send_q.q_buf[(can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
	  }
#endif
	 while(!EMPTY(can_buf->send_q))
		 {
			 memset(temp,0,FRAME_MAX_SIZE);
			 int ret=can_buf->GetFrame(&(can_buf->send_q), temp);
		 /**************拆分成can报文：id+data***************/
			 canfr.can_id=((temp[1]&0xffff)<<8|temp[2]);
			 canfr.can_dlc= ret-4;
			 memset(canfr.data,0,8);
			 memcpy((void*)canfr.data, temp+3, canfr.can_dlc);
			 write(fd, &canfr, sizeof(canfr));	 
	//				 printf("has send to can!\n");
		 }
	 pthread_mutex_unlock(&(can_temp->send_lock));	 
	 }

	delete[] temp;
	return NULL;
}
void* Task::CanSend_test(void* can_eth_para)
{
	CanEthPara* can_eth_para_temp = (CanEthPara*)can_eth_para;
	int fd = can_eth_para_temp->_can->can_sock;
	int fd_another = can_eth_para_temp->_another_can->can_sock;

	Buffer* _can_buf = &(can_eth_para_temp->_can->can_buf);
	Buffer* another_can_buf = &(can_eth_para_temp->_another_can->can_buf);

	struct can_frame canfr;
	char* temp;
	temp=new char[FRAME_MAX_SIZE];
	struct can_frame canfr2;
	char* temp2;
	temp2=new char[FRAME_MAX_SIZE];


	while(1)
	 {
//	 pthread_mutex_lock(&(can_temp->send_lock));
//	 while(EMPTY(can_buf->send_q))
//		 pthread_cond_wait(&(can_temp->send_signal),&(can_temp->send_lock));
#ifdef PRINT_BUFFER
	/**************目前can发送缓冲区的内容 ***************/
	//		  printf("s2_tail=%d\n",can_buf->send_q.tail);
	//		  printf("s2_size=%d\n",CHARS(can_buf->send_q));
	  for(int i=0;i<CHARS(can_buf->send_q);i++)
	  {
		 for(int j=0;j<can_buf->send_q.q_buf[(can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
			  printf("2send_buf[%d]=0x%02x\t",j,
			   can_buf->send_q.q_buf[(can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
		 printf("2length=%d\n",
		   can_buf->send_q.q_buf[(can_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
	  }
#endif
	 while(!EMPTY(_can_buf->send_q))
		 {
			 memset(temp,0,FRAME_MAX_SIZE);
			 int ret=_can_buf->GetFrame(&(_can_buf->send_q), temp);
		 /**************拆分成can报文：id+data***************/
			 canfr.can_id=((temp[1]&0xffff)<<8|temp[2]);
			 canfr.can_dlc= ret-4;
			 memset(canfr.data,0,8);
			 memcpy((void*)canfr.data, temp+3, canfr.can_dlc);
			 write(fd, &canfr, sizeof(canfr));	 
	//				 printf("has send to can!\n");
		 }
//	 pthread_mutex_unlock(&(can_temp->send_lock));	
	  while(!EMPTY(another_can_buf->send_q))
		  {
			  memset(temp2,0,FRAME_MAX_SIZE);
			  int ret=another_can_buf->GetFrame(&(another_can_buf->send_q), temp2);
		  /**************拆分成can报文：id+data***************/
			  canfr2.can_id=((temp2[1]&0xffff)<<8|temp2[2]);
			  canfr2.can_dlc= ret-4;
			  memset(canfr2.data,0,8);
			  memcpy((void*)canfr2.data, temp2+3, canfr2.can_dlc);
			  write(fd_another, &canfr2, sizeof(canfr2));   
	 // 			  printf("has send to can!\n");
		  }

	 }

	delete[] temp;
	delete[] temp2;
	return NULL;
}

void* Task::ClientSend(void* c)
{
	Client* clie_temp = (Client*)c;
	Buffer* _clie_buf = &(clie_temp->clie_buf);

	int ret=0;
	char* temp;
	temp=new char[BUF_LENGTH];

	while(1)
	 {	 
		 pthread_mutex_lock(&(clie_temp->send_lock));
		 while(EMPTY(_clie_buf->send_q))
			 pthread_cond_wait(&(clie_temp->send_signal),&(clie_temp->send_lock));
#ifdef PRINT_BUFFER
	 /**************目前eth发送缓冲区的内容 ***************/
		 printf("s2_tail=%d\n",_clie_buf->send_q.tail);
		 printf("s2_size=%d\n",CHARS(_clie_buf->send_q));
		 for(int i=0;i<CHARS(_clie_buf->send_q);i++)
		 {
			for(int j=0;j<_clie_buf->send_q.q_buf[(_clie_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0];j++)
				 printf("2send_buf[%d]=0x%02x ",j,
				  _clie_buf->send_q.q_buf[(_clie_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][j]);
			printf("2length=%d\n",
			  _clie_buf->send_q.q_buf[(_clie_buf->send_q.tail+i)& (BUF_MAX_SIZE - 1)][0]);
		 }
#endif
		 memset(temp,0,BUF_LENGTH);
		 int len=0;
		 while(!EMPTY(_clie_buf->send_q))
			 {
			 ret=_clie_buf->GetFrame(&(_clie_buf->send_q), temp + len);
			 len=ret + len;
			 }
		 clie_temp->ClientSend(clie_temp->clie_sock ,temp,	len  );//注意这里是clie_sock！！！
		 pthread_mutex_unlock(&(clie_temp->send_lock));
	 }
	delete[] temp;
	return NULL;	 

}

extern Node node[];
extern int nodeid_on_bus[4][node_num_per_bus];
extern unsigned short recvid_on_bus[4][filter_size_per_bus];
extern int node_num;
int recvid_on_bus_size[4];//最终二维数组每个维度的有效数据长度

//使用差值查找算法进行查找，可参考 https://www.cnblogs.com/leezx/p/5719012.html
bool IsBelongTo(unsigned short canfr_id, void* owner)
{
	CanComm* can_temp=(CanComm*)owner;
	int channel_temp=can_temp->can_channel;

	int low=0,mid=0,high=recvid_on_bus_size[channel_temp] - 1;
	while (low<=high)
		{	if(low == high)
				mid = low;
			else
				mid = low + (canfr_id - recvid_on_bus[channel_temp][low]) / 
							(recvid_on_bus[channel_temp][high] - recvid_on_bus[channel_temp][low]) * (high - low);
			if(mid < low ) mid = low;//保证mid大于0，因为mid是数组下标
			if(mid > high) mid = high;			
			if(canfr_id < recvid_on_bus[channel_temp][mid])
				high = mid -1;
			else if( canfr_id > recvid_on_bus[channel_temp][mid])
				low  = mid +1;
			else 
				return 1;//如果查到了就返回1
		}
	return 0;
}
void CreatAllRecvFilter(void)
{
	//先生成最终数组
	for(int i=0 ;i<4 ;i++)
	{
		int m=0;
		for(int j=0;j< node_num_per_bus; j++)
		{
			for(int k=0; k < node_num ; k++)
			{
				if(node[k].id == nodeid_on_bus[i][j])
				{//复制进recvid_on_bus数组
					for (int n = 0; n < filter_size_per_node; n++)
					{
						if(node[k].recv_id[n] != 0)
							recvid_on_bus[i][m++] = node[k].recv_id[n];
						else 
							break;
					}	
					break;
				}				
			}
		}
		recvid_on_bus_size[i] = m;
//		for (int j = 0;j < m; j++)
//			printf("0x%03x\t",recvid_on_bus[i][j]);
//		printf("\n");
	}
	printf("nodenum: %d\n",node_num);
//	进行冒泡排序的预处理！
	printf("array after bubble:\n");
	for(int i=0; i<4 ; i++)
		{BubbleSort(recvid_on_bus[i],recvid_on_bus_size[i]);printf("\n");}
}

void BubbleSort(unsigned short arr[],int size)
{
    int low = 0, high = size-1 ; 
    unsigned short tmp=0;    
    while (low < high) {    
        for (int j= low; j< high; ++j) //正向冒泡,找到最大者    
            if (arr[j]> arr[j+1]) {    
                tmp = arr[j]; arr[j]=arr[j+1]; arr[j+1]=tmp;    
            }     
        --high;                 //修改high值, 前移一位    
        for (int j=high; j>low; --j) //反向冒泡,找到最小者    
            if (arr[j]<arr[j-1]) {    
                tmp = arr[j]; arr[j]=arr[j-1]; arr[j-1]=tmp;    
            }    
        ++low;                  //修改low值,后移一位    
    } 	
	for(int j=0; j<size ;j++)
		printf("[%d]:0x%03x  ",j,arr[j]);
}
void* Task::test(void* a)
{

while (1)
	{
		printf("hahahahahahahahahahhahahahahahahahahaha");
		sleep(1);
	}
}
