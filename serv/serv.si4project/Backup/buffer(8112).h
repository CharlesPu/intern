/*******************************
@@Author     : Charles
@@Date       : 2017-10-18
@@Mail       : pu17rui@sina.com
@@Description: basic structure and macro             
*******************************/
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <system.h>
#include <string.h>

#define BUF_MAX_SIZE 128      // must be power of 2. Maximen 256.

struct can_queue {
	unsigned char head;		             // 写指针
	unsigned char tail;		             // 读指针
	char* buf[BUF_MAX_SIZE];   //真实的缓冲区,指针数组
};

struct serv_buffer {
	//void (*write)(struct com_struct *com);  // 函数指针和结构体指针，其中函数的入口参数为结构体指针
	struct can_queue read_q;                // 接收缓冲区
	struct can_queue write_q;               // 发送缓冲区
};

#define INC(a) ((a) = ((a) + 1) & (COM_BUF_SIZE - 1))                      //a自增但是不能超过最大范围，若超过255，则变为0，因为是8位
#define DEC(a) ((a) = ((a) - 1) & (COM_BUF_SIZE - 1))   
#define EMPTY(a) ((a).head == (a).tail)                                    //首末指针相等即空
#define LEFT(a) (((a).tail - (a).head - 1)&(COM_BUF_SIZE - 1))    
#define LAST(a) ((a).buf[(COM_BUF_SIZE - 1)&((a).head - 1)])      
#define FULL(a) (!LEFT(a))                                                 // 首末指针相差1则认为满
#define CHARS(a) (((a).head - (a).tail)&(COM_BUF_SIZE-1))      
#define GETCH(queue,c) {c=(queue).buf[(queue).tail];INC((queue).tail);}    // 把queue往c里面读取
#define PUTCH(c,queue) {(queue).buf[(queue).head]=(c);INC((queue).head);}  // 把c往queue里面写入

void ServBufferInit(void);
unsigned char com_read(unsigned char channel, char *buf, unsigned char n);
//unsigned char com_write(unsigned char channel, char *buf, unsigned char n);

#endif //__COM_H__

