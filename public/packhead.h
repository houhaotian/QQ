#pragma once
#include <winsock2.h>
#include <string>
#ifndef __PACKHEAD_H__
#define __PACKHEAD_H__

#define ZHUCE_SUCCESS_FLAG 1
#define ZHUCE_FAILED_FLAG 0
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
	SOCKET sfd;//客户端就是与服务端连接的fd，服务端NULL.
	SOCKET dfd;//客户端就是想要发送的目标客户端的fd，服务端也是目标fd。
	int msg_len;//payload长度
	char name[24];//客户端名字
	int name_length;
	CMDLIST cmdtype;
}Pack_head;


typedef struct _SEND_ZHUCE_MSG
{
	int account_len : 8; //[7:0]
	int account : 24;	 //[31:8]
	int passwd_len : 8;	 //[39:32]
	int passwd : 24;	 //[63:40]
}SEND_ZHUCE_MSG;

Pack_head construct_packethead(SOCKET source_fd, SOCKET des_fd, int messege_len, std::string myname, CMDLIST cmd);

int get_package(char *message, const char* data, Pack_head mypackhead);
#endif/*__PACK_HEAD_H__*/
