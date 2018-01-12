#include "buffer.h"

void Buffer::Buffer()
{
    int i=0;
	for(i=0;i<BUF_MAX_SIZE,i++)
	{
     read_q.q_buf[i]=new char[FRAME_MAX_SIZE];
	 memset(read_q.q_buf[i],0,FRAME_MAX_SIZE);
	 
     write_q.q_buf[i]=new char[FRAME_MAX_SIZE];
	 memset(write_q.q_buf[i],0,FRAME_MAX_SIZE);

	}


}
void Buffer::BufferInit(void)
{


}

int Buffer::GetFrame(struct can_queue queue,char* c)// 把queue往c里面读取
{
	int i;
	for(i=0;i<strlen(queue.q_buf[(queue).tail]);i++)
	{
		(*c)=*((queue).q_buf[(queue).tail]);
		c++;	
		(queue).q_buf[(queue).tail]++;
	}
	
	INC((queue).tail);
    return strlen(queue.q_buf[(queue).tail-1]);
}
void Buffer::PutFrame(char* c,struct can_queue queue)// 把c往queue里面写入
{
	int i;
	for(i=0;i<strlen(c);i++)
	{
		(*c)=*((queue).q_buf[(queue).tail]);
		c++;	
		(queue).q_buf[(queue).tail]++;
	}
	
	INC((queue).tail);
    return i;

	(queue).q_buf[(queue).head]=(c);INC((queue).head);



}


