#pragma once
#include <WinSock2.h>
#include <string>
typedef struct _client_addr
{
	SOCKET cli_fd;
	struct sockaddr_in cli_addr;
	char name[24];
	std::string acount;
}client_addr;


client_addr fill_arg(SOCKET fd, struct sockaddr_in addr, char *name);