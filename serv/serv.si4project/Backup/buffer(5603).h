/*******************************
@@Author     : Charles
@@Date       : 2017-10-18
@@Mail       : pu17rui@sina.com
@@Description: basic structure and macro             
*******************************/
#ifndef __COM_H__
#define __COM_H__

#include <system.h>

//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------
#define COM_NUM	UART_NUM     // Com ports. can be different from UART_NUM.
#define COM_BUF_SIZE 128      // must be power of 2. Maximen 256.

struct com_queue {
	unsigned char head;		             // 写指针
	unsigned char tail;		             // 读指针
	unsigned char buf[COM_BUF_SIZE];   //真实的缓冲区
};

struct com_struct {
	unsigned char uart;                     // 串口号
	void (*write)(struct com_struct *com);  // 函数指针和结构体指针，其中函数的入口参数为结构体指针
	struct com_queue read_q;                // 接收缓冲区
	struct com_queue write_q;               // 发送缓冲区
};

extern xdata struct com_struct com_table[COM_NUM];

#define INC(a) ((a) = ((a) + 1) & (COM_BUF_SIZE - 1))                      //a自增但是不能超过最大范围，若超过255，则变为0，因为是8位
#define DEC(a) ((a) = ((a) - 1) & (COM_BUF_SIZE - 1))   
#define EMPTY(a) ((a).head == (a).tail)                                    //首末指针相等即空
#define LEFT(a) (((a).tail - (a).head - 1)&(COM_BUF_SIZE - 1))    
#define LAST(a) ((a).buf[(COM_BUF_SIZE - 1)&((a).head - 1)])      
#define FULL(a) (!LEFT(a))                                                 // 首末指针相差1则认为满
#define CHARS(a) (((a).head - (a).tail)&(COM_BUF_SIZE-1))      
#define GETCH(queue,c) {c=(queue).buf[(queue).tail];INC((queue).tail);}    // 把queue往c里面读取
#define PUTCH(c,queue) {(queue).buf[(queue).head]=(c);INC((queue).head);}  // 把c往queue里面写入

void com_init(void);
unsigned char com_read(unsigned char channel, char *buf, unsigned char n);
unsigned char com_write(unsigned char channel, char *buf, unsigned char n);

#endif //__COM_H__

