#include <stdlib.h>
#include <stdio.h>
#include "common.h"

Node node[]=
{
	{
		1, "",
		{0x007},
		{0x002}
	},
	{
		2, "",
		{0x003},
		{0x004}
	},
	{
		3, "",
		{0x007},
		{0x006}
	},
	{
		4, "",
		{0x005},
		{0x008}
	},
	{
		5, "",
		{0x015},
		{0x018}
	},
	{
		6, "",
		{0x017},
		{0x012}
	},
	{
		7, "",
		{0x012},
		{0x008}
	},
	{
		8, "",
		{0x013},
		{0x008}
	},
	{
		9, "",
		{0x014},
		{0x008}
	}
};
int node_num = sizeof(node)/sizeof(Node);

int nodeid_on_bus[4][node_num_per_bus]=
{
	{
		2,1,7
	},
	{
		3,4
	},
	{
		5,9
	},
	{
		8,6
	}
};
	
unsigned short recvid_on_bus[4][filter_size_per_bus]={};

