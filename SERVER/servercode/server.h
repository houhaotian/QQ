#pragma once
#include <vector>
#include <wsipv6ok.h>
#include "client_addr.h"
class server
{
public:
	server();
	~server();
	int server_login();

	static DWORD WINAPI listenCli(LPVOID pM, fd_set * rset, fd_set * allset);
	static int recv_arp(client_addr clientinfo,int datalen);
	static int send_onlinelist(client_addr clientinfo,int src_len);
	static int trans_word(client_addr clientinfo, SOCKET dfd, int data_len);
private:
	SOCKET listen_fd;
	static std::vector<client_addr> cli_info;
};
