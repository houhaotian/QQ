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
#include <algorithm>
#include <vector>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8000

using namespace std;

vector<client_addr> server::cli_info;


class compare_is_fd
{
	SOCKET fd;
public:
	compare_is_fd(SOCKET infd) : fd(infd) {}
	bool operator()(client_addr cli_info) const { return cli_info.cli_fd == fd; }
};


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
	cout << "bind success" << endl;
	ret = listen(listen_fd, 20);
	if (ret < 0)
	{
		perror("listen error.\n");
		closesocket(listen_fd);
		return SER_LISTEN_FAILED;
	}
	cout << "listen success" << endl;

	int addr_len;
	struct sockaddr_in cliaddr;
	SOCKET cli_fd;
	client_addr clientinfo;
	addr_len = sizeof(cliaddr);

	fd_set rset, allset;
	int max_fd = (int)listen_fd;
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
				cout << "accept client success" << endl;
				FD_SET(cli_fd, &allset);
				if ((int)cli_fd > max_fd)
					max_fd = (int)cli_fd;

				clientinfo = fill_arg(cli_fd, cliaddr, "NULL");
				cli_info.push_back(clientinfo);
			}
			for (int i = 0; i < (int)cli_info.size(); ++i)	//在这里做客户端读写控制
			{
				if (FD_ISSET(cli_info.at(i).cli_fd, &rset))
				{
					listenCli(&cli_info.at(i), &rset, &allset);
				}
			}
		}
	}
	return RUN_SUCCESS;
}



/*监听客户端线程*/
DWORD WINAPI server::listenCli(LPVOID pM, fd_set * rset, fd_set * allset)
{
	client_addr clientinfo = *(client_addr *)pM;
	int ret;
	int recv_len;
	Pack_head pack_head;

	ret = recv(clientinfo.cli_fd, (char *)&pack_head, sizeof(pack_head), 0);
	if (ret == SOCKET_ERROR) {
		cout << "RECV pack_head error!" << endl;
		closesocket(clientinfo.cli_fd);
		FD_CLR(clientinfo.cli_fd, allset);
		printf("bye!");
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
			ret = send_onlinelist(clientinfo, sizeof("-look"));
			if (ret != RUN_SUCCESS)
				return ret;
			break;
		}
		case TALK:/*如果是TALK,直接转发到相对应fd客户端处即可*/
		{
			strcpy_s(clientinfo.name, pack_head.name);
			ret = trans_word(clientinfo, dfd, recv_len);
			if (ret != RUN_SUCCESS)
				return ret;
			break;
		}
		case ZHUCE:
		{
			ret = zhuce_program(clientinfo,recv_len);
		}
		default:
			break;
	}
	return 0;
}



int server::recv_arp(client_addr clientinfo, int datalen)
{
	int ret;
	char recv_data[40 * 1024];
	memset(recv_data, 0, 40 * 1024);
	client_addr tempinfo;
	tempinfo = fill_arg(clientinfo.cli_fd, clientinfo.cli_addr, clientinfo.name);
	/*根据clientinfo.cli_fd查找cli_info对应的vector,把name替换*/
	vector<client_addr>::iterator it;
	it = find_if(cli_info.begin(), cli_info.end(), compare_is_fd(clientinfo.cli_fd));
	*it = tempinfo;

	ret = recv(tempinfo.cli_fd, recv_data, datalen, 0);
	if (ret == SOCKET_ERROR) {
		cout << "RECV data error" << endl;
		return RECV_FAILED;
	}
	cout << recv_data << endl;
	cout << clientinfo.cli_fd << endl;
	return RUN_SUCCESS;
}

int server::send_onlinelist(client_addr clientinfo, int src_len)
{
	cout << "client income online request:" << endl;
	Pack_head p_head;
	char message[40 * 1024];
	char list_data[40 * 1024];
	memset(message, 0, 40 * 1024);
	memset(list_data, 0, 40 * 1024);
	int mes_len, len = 0;
	int ret;

	ret = recv(clientinfo.cli_fd, message, src_len, 0);
	if (ret == SOCKET_ERROR) {
		cout << "RECV data error" << endl;
		return RECV_FAILED;
	}

	vector<client_addr>::iterator it;
	for (it = cli_info.begin(); it < cli_info.end(); ++it)
	{
		memcpy(list_data + len, &it->cli_fd, sizeof(size_t));
		cout << it->cli_fd << endl;
		memcpy(list_data + sizeof(size_t) + len, it->name, 24);
		cout << it->name << endl;
		len += sizeof(size_t) + 24;//数据长度
	}
	p_head = construct_packethead(clientinfo.cli_fd, 0, len, "hello world", LOOK);//组帧头
	mes_len = get_package(message, list_data, p_head);
	ret = send(clientinfo.cli_fd, message, mes_len, 0);
	if (ret == SOCKET_ERROR) {
		cout << "SEND data error" << endl;
		return SEND_FAILED;
	}
	return RUN_SUCCESS;
}

int server::trans_word(client_addr clientinfo, SOCKET dfd, int data_len)
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
		cout << "RECV data error" << endl;
		return RECV_FAILED;
	}
	cout << recv_data << endl;
	p_head = construct_packethead(clientinfo.cli_fd, dfd, data_len, clientinfo.name, TALK);//组帧头
	mes_len = get_package(message, recv_data, p_head);
	ret = send(dfd, message, mes_len, 0);
	if (ret == SOCKET_ERROR) {
		cout << "SEND data error" << endl;
		return SEND_FAILED;
	}
	return RUN_SUCCESS;
}

int server::zhuce_program(client_addr clientinfo, int data_len)
{
	int ret;
	
	char recv_data[40 * 1024];
	memset(recv_data, 0, 40 * 1024);
	ret = recv(clientinfo.cli_fd, recv_data, data_len, 0);
	if (ret == SOCKET_ERROR) {
		cout << "RECV data error" << endl;
		return RECV_FAILED;
	}
	cout << recv_data << endl;

	string temp_len;
	string temp_acount;
	string temp_passwd;
	int len1;
	int tempflag1 = 0;
	int tempflag2 = 0;
	if ((48 <= recv_data[1]) && (recv_data[1] <= 57))//是数字
	{
		temp_len.append(&recv_data[0], 2);
		len1 = stoi(temp_len);
		tempflag1 = 1;
	}
	else
	{
		temp_len.append(&recv_data[0], 1);
		len1 = stoi(temp_len);
		tempflag1 = 0;
	}
	temp_acount.append(&recv_data[tempflag1+1], len1);

	if ((48 <= recv_data[tempflag1 + 2 + len1]) && (recv_data[tempflag1 + 2 + len1] <= 57))//是数字
	{/*不行，密码数字开头没法判断*/
		temp_len.clear();
		temp_len.append(&recv_data[tempflag1 + 1 + len1], 2);
		len1 = stoi(temp_len);
		tempflag2 = 1;
	}
	else
	{
		temp_len.clear();
		temp_len.append(&recv_data[tempflag1 + 1 + len1], 1);
		len1 = stoi(temp_len);
		tempflag2 = 0;
	}
	cout << temp_len << "  " << len1 << endl;

	temp_passwd.append(&recv_data[tempflag1 + tempflag2 + 2 + len1], len1);

	cout << "acount"<< temp_acount << endl;
	cout << "passwd"<< temp_passwd << endl;

	/*查看sql是否有该用户名，没有则注册成功存到sql，失败则不回复下面的1。*/

	/*回客户端1代表注册成功*/
	Pack_head p_head;
	p_head = construct_packethead(clientinfo.cli_fd, 0, 1, "NULL", ZHUCE);//组帧头
	char zhuceflag = ZHUCE_SUCCESS_FLAG;
	int mes_len = 0;
	char message[40 * 1024];
	memset(message, 0, 40 * 1024);
	mes_len = get_package(message, &zhuceflag, p_head);
	ret = send(clientinfo.cli_fd, message, mes_len, 0);
	if (ret == SOCKET_ERROR) {
		cout << "SEND data error" << endl;
		return SEND_FAILED;
	}
	return RUN_SUCCESS;
}