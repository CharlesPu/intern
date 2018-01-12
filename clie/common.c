#include <stdlib.h>
#include <stdio.h>
#include "common.h"

Node node[]=
{
	{
		1, "",
		{0x001,0x002,0x003,0x004,0x005},//recv
		{0x011,0x012,0x013,0x014,0x015} //send
	},
	{
		2, "",
		{0x011,0x012,0x013,0x014,0x015},
		{0x051,0x052,0x053,0x054,0x055}
	},
	{
		3, "",
		{0x021,0x022,0x023,0x024,0x025},//
		{0x016,0x017,0x018,0x019,0x01a}
	},
	{
		4, "",
		{0x031,0x032,0x033,0x034,0x035},
		{0x006,0x007,0x008,0x009,0x00a}
	},
	{
		5, "",
		{0x041,0x042,0x043,0x044,0x045},//
		{0x251,0x252,0x253,0x254,0x255}
	},
	{
		6, "",
		{0x051,0x052,0x053,0x054,0x055},
		{0x046,0x047,0x048,0x049,0x04a}
	},
	{
		7, "",
		{0x151,0x152,0x153,0x154,0x155},
		{0x001,0x002,0x003,0x004,0x005}
	},
	{
		8, "",
		{0x251,0x252,0x253,0x254,0x255},//
		{0x031,0x032,0x033,0x034,0x035}
	},
	{
		9, "",
		{0x006,0x007,0x008,0x009,0x00a},
		{0x056,0x057,0x058,0x059,0x05a}
	},
	{
		10, "",
		{0x016,0x017,0x018,0x019,0x01a},
		{0x041,0x042,0x043,0x044,0x045}
	},
	{
		11, "",
		{0x026,0x027,0x028,0x029,0x02a},
		{0x151,0x152,0x153,0x154,0x155}
	},
	{
		12, "",
		{0x036,0x037,0x038,0x039,0x03a},
		{0x011,0x012,0x013,0x014,0x015}
	},
	{
		13, "",
		{0x046,0x047,0x048,0x049,0x04a},
		{0x256,0x257,0x258,0x259,0x25a}
	},
	{
		14, "",
		{0x056,0x057,0x058,0x059,0x05a},
		{0x021,0x022,0x023,0x024,0x025}
	},
	{
		15, "",
		{0x156,0x157,0x158,0x159,0x15a},
		{0x026,0x027,0x028,0x029,0x02a}
	},
	{
		16, "",
		{0x256,0x257,0x258,0x259,0x25a},
		{0x156,0x157,0x158,0x159,0x15a}
	},

};
int node_num = sizeof(node)/sizeof(Node);

int nodeid_on_bus[4][node_num_per_bus]=
{
	{
		1,7,14,16
	},
	{
		2,8,13,15
	},
	{
		3,6,9,12
	},
	{
		4,5,10,11
	}

};
	
unsigned short recvid_on_bus[4][filter_size_per_bus]={};

