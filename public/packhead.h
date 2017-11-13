#pragma once
#include <winsock2.h>
#include <string>
#ifndef __PACKHEAD_H__
#define __PACKHEAD_H__

typedef enum 
{
	ARP,	//ARP包，告诉服务器记录客户端的IP
	LOOK,	//查看在线人员
	TALK,	//数据包
	ZHUCE,	//注册请求包
	DENGLU, //登录请求包
}CMDLIST;
typedef struct _pack_head
{
	SOCKET sfd;
	SOCKET dfd;
	int msg_len;
	char name[24];
	int name_length;
	char passwd[24];
	CMDLIST cmdtype;
}Pack_head;

Pack_head construct_packethead(SOCKET source_fd, SOCKET des_fd, int messege_len, std::string myname, CMDLIST cmd);

int get_package(char *message, const char* data, Pack_head mypackhead);
#endif/*__PACK_HEAD_H__*/
