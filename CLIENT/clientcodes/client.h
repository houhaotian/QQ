#pragma once
#include <string>
#include "..\\..\\public\\packhead.h"
#include <winsock2.h>
class client
{
public:
	client();
	~client();
	int client_login();
	int client_talkToServer();
	int send_message(std::string tempdata);
	void change_dfd();
	int send_look_request();
	int send_arp();
	int get_package(char *message, std::string temp,Pack_head myhead);
	void construct_packet_head(SOCKET source_fd, SOCKET des_fd, int messege_len, std::string myname,CMDLIST cmd);
	static DWORD WINAPI listenSer(LPVOID pM);
private:
	SOCKET fd;//自己的文件描述符；
	Pack_head mypackhead;
};

