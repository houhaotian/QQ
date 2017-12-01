#pragma once
#include <winsock2.h>
#include <string>
#ifndef __PACKHEAD_H__
#define __PACKHEAD_H__

#define ZHUCE_SUCCESS_FLAG 1
#define ZHUCE_FAILED_FLAG 0
typedef enum 
{
	ARP,	//ARP�������߷�������¼�ͻ��˵�IP
	LOOK,	//�鿴������Ա
	TALK,	//���ݰ�
	ZHUCE,	//ע�������
	DENGLU, //��¼�����
}CMDLIST;

typedef struct _pack_head
{
	SOCKET sfd;//�ͻ��˾������������ӵ�fd�������NULL.
	SOCKET dfd;//�ͻ��˾�����Ҫ���͵�Ŀ��ͻ��˵�fd�������Ҳ��Ŀ��fd��
	int msg_len;//payload����
	char name[24];//�ͻ�������
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
