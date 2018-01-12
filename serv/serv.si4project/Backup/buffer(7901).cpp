#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>


Buffer::Buffer()
{
    int i=0;
	for(i=0;i<BUF_MAX_SIZE;i++)
	{
     recv_q.q_buf[i]=new char[FRAME_MAX_SIZE];
	 memset(recv_q.q_buf[i],0,FRAME_MAX_SIZE);
	 
     send_q.q_buf[i]=new char[FRAME_MAX_SIZE];
	 memset(send_q.q_buf[i],0,FRAME_MAX_SIZE);
	}
	recv_q.head=0;
	recv_q.tail=0;
	send_q.head=0;
	send_q.tail=0;
}

Buffer::~Buffer()
{
	int i=0;
	for(i=0;i<BUF_MAX_SIZE;i++)
	{
		if(recv_q.q_buf[i] != NULL)delete[] recv_q.q_buf[i];
		if(send_q.q_buf[i] != NULL)delete[] send_q.q_buf[i];
	}
	recv_q.head=0;
	recv_q.tail=0;
	send_q.head=0;
	send_q.tail=0;
}

void Buffer::BufferInit(void)
{	
	recv_q.head=0;
	recv_q.tail=0;
	send_q.head=0;
	send_q.tail=0;	


}

int Buffer::GetFrame(struct can_queue* queue,char* c,int num)// 把queue往c里面读取
{
	char* str=c;
	struct can_queue* q = queue ;
//	int i;
//	for(i=0;i<strlen(queue.q_buf[(queue).tail]);i++)
//	{
//		(*c)=*((queue).q_buf[(queue).tail]);
//		c++;	
//		(queue).q_buf[(queue).tail]++;
//	}
	memcpy(str, q->q_buf[q->tail], strlen(q->q_buf[q->tail]));
	
	INC(q->tail);
    return strlen(q->q_buf[q->tail-1]);
}
int Buffer::PutFrame(char* c,struct can_queue* queue,int num)// 把c往queue里面写入
{
	char* str=c;
	struct can_queue* q = queue ;
	memset(q->q_buf[q->head],0,sizeof(q->q_buf[q->head]));
	memcpy(q->q_buf[q->head], str, num);
//	for(int i=0;i<num;i++)
//	   printf("q->q_buf[q->head][%d]=%d\r\n",i,q->q_buf[q->head][i]);
	INC(q->head);
//	printf("head=%d\n",q->head);
    return num;
}


