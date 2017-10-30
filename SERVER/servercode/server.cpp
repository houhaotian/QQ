#include "server.h"
#include <winsock2.h>
#include <stdlib.h>
#include "client_addr.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include "..\\..\\public\\errcode.h"
#include "..\\..\\public\\packhead.h"
#include <xutility>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8000

std::vector<client_addr> server::cli_info;

server::server()
{
}

server::~server()
{
}

int server::server_login()
{
	int ret;
	struct sockaddr_in seraddr;
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	//创建socket
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
	{
		perror("socket create error.\n");
		return SOCK_CREATE_FAILED;
	}

	//填充服务端地址
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(SERVER_PORT);
	seraddr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(listen_fd, (struct sockaddr*)&seraddr, sizeof(seraddr));
	if (ret < 0)
	{
		perror("bind error.");
		closesocket(listen_fd);
		return SER_BIND_FAILED;

	}
	std::cout << "bind success" << std::endl;
	ret = listen(listen_fd, 20);
	if (ret < 0)
	{
		perror("listen error.\n");
		closesocket(listen_fd);
		return SER_LISTEN_FAILED;
	}
	std::cout << "listen success" << std::endl;

	//accpet接受链接
	int addr_len;
	struct sockaddr_in cliaddr;
	SOCKET cli_fd;
	client_addr clientinfo;
	addr_len = sizeof(cliaddr);

	fd_set rset, allset;
	int max_fd = listen_fd;
	FD_ZERO(&allset);
	FD_SET(listen_fd, &allset);
	while (1)
	{
		rset = allset;
		ret = select(max_fd + 1, &rset, NULL, NULL, NULL);
		if (ret < 0)
		{
			if (errno == EINTR)
				continue;
			else
				break;
		}
		else
		{
			if (FD_ISSET(listen_fd, &rset)) //有客户端连接请求。
			{
				cli_fd = accept(listen_fd, (struct sockaddr*)&cliaddr, &addr_len);
				std::cout << "accept client success" << std::endl;
				FD_SET(cli_fd,&allset);
				if (cli_fd > max_fd)
					max_fd = cli_fd;

				clientinfo = fill_arg(cli_fd, cliaddr, "NULL");	
			}
			else if(FD_ISSET())//在这里做客户端读写控制
			{}
		}
		
		//一直循环，每握手成功一次，便把客户端fd，addr存入容器cli_info，然后创建线程去监听消息
		HANDLE cli_handle = CreateThread(NULL, 256*1024, listenCli, (LPVOID)&clientinfo, 0, NULL);
		//WaitForSingleObject(cli_handle, INFINITE);
	}
	return RUN_SUCCESS;
}
/*监听客户端线程*/
DWORD WINAPI server::listenCli(LPVOID pM)
{
	client_addr clientinfo = *(client_addr *)pM;
	int ret;
	int recv_len;
	Pack_head pack_head;
	while (1)
	{
		ret = recv(clientinfo.cli_fd, (char *)&pack_head, sizeof(pack_head), 0);
		if (ret == SOCKET_ERROR){
			std::cout << "RECV pack_head error!" << std::endl;
			return RECV_FAILED;
		}
		recv_len = pack_head.msg_len;
		SOCKET dfd = pack_head.dfd;
		switch ((CMDLIST)pack_head.cmdtype)
		{
			case ARP:/*如果是ARP包，则记录该客户端的fd，名字*/
			{
				strcpy_s(clientinfo.name, pack_head.name);
				ret = recv_arp(clientinfo, recv_len); 
				if (ret != RUN_SUCCESS)
					return ret;
				break;
			}
			case LOOK:/*如果对方是LOOK,则发送给对方在线人员名单*/
			{
				ret= send_onlinelist(clientinfo, sizeof("-look"));
				if (ret != RUN_SUCCESS)
					return ret;
				break;
			}
			case TALK:/*如果是TALK,直接转发到相对应fd客户端处即可*/
			{
				strcpy_s(clientinfo.name, pack_head.name);
				ret = trans_word(clientinfo, dfd, recv_len);
				if (ret != RUN_SUCCESS)
			//		return ret;
				break;
			}
		}
	}
	return 0;
}

int server::recv_arp(client_addr clientinfo,int datalen)
{
	int ret;
	char recv_data[40 * 1024];
	memset(recv_data, 0, 40 * 1024);
	client_addr tempinfo;
	tempinfo = fill_arg(clientinfo.cli_fd, clientinfo.cli_addr, clientinfo.name);
	cli_info.push_back(tempinfo);
	ret = recv(tempinfo.cli_fd, recv_data, datalen, 0);
	if (ret == SOCKET_ERROR) {
		std::cout << "RECV data error" << std::endl;
		return RECV_FAILED;
	}
	std::cout << recv_data << std::endl;
	std::cout << clientinfo.cli_fd << std::endl;
	return RUN_SUCCESS;
}

int server::send_onlinelist(client_addr clientinfo,int src_len)
{
	std::cout << "client income online request:" << std::endl;
	Pack_head p_head;
	char message[40 * 1024];
	char list_data[40 * 1024];
	memset(message, 0, 40 * 1024);
	memset(list_data, 0, 40 * 1024);
	int mes_len, len = 0;
	int ret;
	
	ret = recv(clientinfo.cli_fd, message, src_len, 0);
	if (ret == SOCKET_ERROR) {
		std::cout << "RECV data error" << std::endl;
		return RECV_FAILED;
	}

	std::vector<client_addr>::iterator it;
	for (it = cli_info.begin(); it < cli_info.end(); ++it)
	{
		memcpy(list_data + len, &it->cli_fd, sizeof(size_t));
		std::cout << it->cli_fd << std::endl;
		memcpy(list_data + sizeof(size_t) + len, it->name, 24);
		std::cout << it->name << std::endl;
		len += sizeof(size_t) + 24;//数据长度
	}
	p_head = construct_packethead(clientinfo.cli_fd, 0, len, "hello world", LOOK);//组帧头
	mes_len = get_package(message, list_data, p_head);
	ret = send(clientinfo.cli_fd, message, mes_len, 0);
	if (ret == SOCKET_ERROR) {
		std::cout << "SEND data error" << std::endl;
		return SEND_FAILED;
	}
	return RUN_SUCCESS;
}

int server::trans_word(client_addr clientinfo,SOCKET dfd, int data_len)
{
	char recv_data[40 * 1024];
	char message[40 * 1024];
	memset(recv_data, 0, 40 * 1024);
	memset(message, 0, 40 * 1024);
	Pack_head p_head;
	int mes_len;
	int ret;
	ret = recv(clientinfo.cli_fd, recv_data, data_len, 0);
	if (ret == SOCKET_ERROR) {
		std::cout << "RECV data error" << std::endl;
		return RECV_FAILED;
	}
	std::cout << recv_data << std::endl;
	memset(message, 0, 40 * 1024);
	p_head = construct_packethead(clientinfo.cli_fd, dfd, data_len, clientinfo.name, TALK);//组帧头
	mes_len = get_package(message, recv_data, p_head);
	ret = send(dfd, message, mes_len, 0);
	if (ret == SOCKET_ERROR) {
		std::cout << "SEND data error" << std::endl;
			return SEND_FAILED;
	}
	return RUN_SUCCESS;
}
