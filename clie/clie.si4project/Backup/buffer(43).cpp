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

int Buffer::GetFrame(struct can_queue* queue,char* c)// 把queue往c里面读取
{
	char* str=c;
	struct can_queue* q = queue ;

	memset(str,0,sizeof(str));
	memcpy(str, q->q_buf[q->tail], q->q_buf[q->tail][0]);
	
	INC(q->tail);
	
	int posit=(((q->tail) - 1) & (BUF_MAX_SIZE - 1));//保证读写指针在0~max之间循环
    return q->q_buf[posit][0];
}
int Buffer::PutFrame(char* c,struct can_queue* queue,int num)// 把c往queue里面写入
{
	char* str=c;
	struct can_queue* q = queue ;
	memset(q->q_buf[q->head],0,sizeof(q->q_buf[q->head]));
	memcpy(q->q_buf[q->head], str, num);
//	q->q_buf_cell[q->head].q_length = num;

	INC(q->head);
//	printf("head=%d\n",q->head);
    return num;
}


