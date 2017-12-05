#include <winsock2.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <corecrt_io.h>
#include <string>
#include <iostream>
#include <corecrt_malloc.h>
#include <string.h>
#include "client.h"
#include "..\\..\\public\\errcode.h"
#include "..\\..\\public\\packhead.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define CLIENT_PORT 8000


using namespace std;

int client::zhuCeFlag = ZHUCE_FAILED;

client::client()
{
}

client::~client()
{
}

int client::client_login()
{
	struct sockaddr_in seraddr;
	int ret;

	//��ʼ��WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0){
		return 0;
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0){
		perror("socket create error!\n");
		return SOCK_CREATE_FAILED;
	}
	//������˵�ַ
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(CLIENT_PORT);
	inet_pton(AF_INET, SERVER_IP, &(seraddr.sin_addr.s_addr));

	//�����������֣���������
	ret = connect(fd, (struct sockaddr*)&seraddr, sizeof(seraddr));
	if (ret < 0){
		perror("connect");
		closesocket(fd);
		return CLI_CONNECT_FAILED;
	}
	
	/*���������̣߳����շ�����������������*/
	HANDLE listen_to_server_handle = CreateThread(NULL, 64 * 1024, listenSer, (LPVOID)&fd, 0, NULL);

	//ע��
	/*ѭ����ֱ��ע��ɹ���ARP��ֻ�Ƿ������֣�Ҳ���Ա���������server��Ӧ�ý��˺����������Լ�fd�����һ��*/
	/*server�����ȿ�ע����˺���sql����û�У�û����ͨ��ע�ᣬȻ���˺�����д��sql�С�Ȼ����sql�б�ǵ�¼״̬��*/
	/*��¼ʱһ���˺�������ȷ����Ҫ���Ƿ��Ѿ���¼�������õ�¼������������Ա������*/
	/*sql��Ӧ���˺�����͸����ǳư󶨺ò�Ȼ�������ϱ߹��ܡ�ͬʱ����vectorֻҪά��fd������,�˺ž͹���*/
	ret = zhuce();
	if (ret != RUN_SUCCESS)
	{
		closesocket(fd);
		return ZHUCE_FAILED;
	}

	//����ARP��
	ret = send_arp();
	if (ret != RUN_SUCCESS)
		return ret;

	return RUN_SUCCESS;
}

int client::client_talkToServer()
{
	int ret;
	std::string temp_data;
	while (1)
	{
		std::cout << "����������,����'-change'���ĶԷ�fd,����'-look'�鿴������Ա" << std::endl;
		std::cin >> temp_data;//��stdin��ȡ����
		if (temp_data.compare("-change") == 0)
		{
			change_dfd();
		}
		else if (temp_data.compare("-look") == 0)
		{
			ret = send_look_request();
			if (ret != RUN_SUCCESS)
				return ret;
		}
		else
		{
			ret = send_message(temp_data);
			if (ret != RUN_SUCCESS)
				return ret;
		}
	}
	return 0;
}

int client::send_message(std::string temp_data)
{
	char message[40 * 1024];
	int mes_len;
	int ret;
	construct_packet_head(fd, mypackhead.dfd, (int)temp_data.length(), mypackhead.name, TALK);//��֡ͷ
	mes_len = get_package(message, temp_data, mypackhead);//��֡����������buffer����
	ret = send(fd, message, mes_len, 0);
	if (ret == SOCKET_ERROR) {
		printf("send error!");
		return SEND_FAILED;
	}
	return RUN_SUCCESS;
}

void client::change_dfd()
{
	std::cout << "����Է�fd" << std::endl;
	std::cin >> mypackhead.dfd;//��stdin��ȡdfd
}

int client::send_look_request()
{
	std::string temp_data = "-look";
	char message[1024];
	int mes_len;
	int ret;
	construct_packet_head(fd, mypackhead.dfd, (int)temp_data.length(), mypackhead.name, LOOK);//��֡ͷ
	mes_len = get_package(message, temp_data, mypackhead);//��֡����������buffer����
	ret = send(fd, message, mes_len, 0);
	if (ret == SOCKET_ERROR) {
		printf("send error!");
		return SEND_FAILED;
	}
	return RUN_SUCCESS;
}


/* ARP��֡��ʽ��
	" pack_head + payload(hello world) "
	"ARP֡�а����Լ���fd������"
*/
int client::send_arp()
{
	//����ARP��
	std::string temp_data;
	temp_data = "hello world";
	int mes_len;
	int ret;
	char arp_message[1024];
	std::cout << "�����������" << std::endl;
	std::cin >> mypackhead.name;
	construct_packet_head(fd, 0, temp_data.length(), mypackhead.name, ARP);//��֡ͷ
	mes_len = get_package(arp_message, temp_data, mypackhead);//��֡����������buffer����
	ret = send(fd, arp_message, mes_len, 0);
	if (ret == SOCKET_ERROR) {
		printf("send error!");
		return SEND_FAILED;
	}
	return RUN_SUCCESS;
}

int client::zhuce()
{
	int i = 3, ret;
	int flag = 0;
	string acount, passwd;

	/*֡ͷ��fd,0,len,NULL,*/
	/*account_len 1B,account 24B ,passwd_len 1B,passwd 24B*/
	cout << "��¼����,type 1ע��, 2��¼, 3�˳�" << endl;
	cin >> flag;
	switch (flag)
	{
		case 1:
		{
			while (i--)
			{
				cout << "Ոݔ���Ñ���" << endl;
				cin >> acount;
				cout << "����������" << endl;
				cin >> passwd;
				/*�������ˣ���һ���û���������ĸ��ͷ�İ�*/
				/*if acount.data(1)������ĸ*/
#if 0
				string temp_data, temp_len;
				temp_len = to_string(acount.size());
				temp_data = temp_len;
				temp_data += acount;
				temp_len = to_string(passwd.size());
				temp_data += temp_len;
				temp_data += passwd;
				cout << temp_data << endl;
#endif
				UINT8 tempframe[64] = { 0 };
				UINT8 templength = acount.size();
				tempframe[0] = templength;
				memcpy(&tempframe[1], acount.c_str(), templength);
				UINT8 templength = passwd.size();
				tempframe[25] = templength;
				memcpy(&tempframe[26], passwd.c_str(), templength);
			//	string temp_data = tempframe;

				int mes_len;
				UINT8 arp_message[1024] = { 0 };
				construct_packet_head(fd, 0, temp_data.length(), "NULL", ZHUCE);//��֡ͷ
				mes_len = get_package(arp_message, temp_data, mypackhead);//��֡����������buffer����
				ret = send(fd, arp_message, mes_len, 0);
		//		system("cls");
				if (zhuCeFlag == 1)
					return RUN_SUCCESS;
				cout << "�û����������ظ�������������" << endl;
			}
			break;
		}
		case 2:
		{
			break;
		}
		default:
			return ZHUCE_FAILED;
	}

	return ZHUCE_FAILED;
}

/*messageǰ���pack_head,����Ŵ�����*/
int client::get_package(char *message, std::string temp, Pack_head mypackhead)
{
	int mes_len = (int)temp.size();//�����ݳ���
	memcpy(message, &mypackhead, sizeof(mypackhead));
	memcpy(message + sizeof(mypackhead), temp.c_str(), temp.size());
	message[sizeof(mypackhead) + mes_len] = '\0';
	return (mes_len + sizeof(mypackhead));
}

void client::construct_packet_head(SOCKET source_fd, SOCKET des_fd, int messege_len, std::string myname, CMDLIST cmd)
{
	int name_len;
	mypackhead.sfd = source_fd;
	mypackhead.dfd = des_fd;
	mypackhead.msg_len = messege_len;
	mypackhead.cmdtype = cmd;
	name_len = (int)myname.length();
	if (name_len > 24)//�������24�ַ�
		name_len = 24;
	mypackhead.name_length = name_len;
	strncpy_s(mypackhead.name, myname.c_str(), name_len);
}

DWORD WINAPI client::listenSer(LPVOID pM)
{
	SOCKET fd = *(SOCKET *)pM;
	Pack_head pack_head;
	/*����buffer*/
	char *listen_buffer = (char *)malloc(40 * 1024);
	memset(listen_buffer, 0, 40 * 1024);
	int ret;
	while (1)
	{
		ret = recv(fd, (char *)&pack_head, sizeof(pack_head), 0);
		if (ret == SOCKET_ERROR) {
			std::cout << "RECV data error" << std::endl;
			return RECV_FAILED;
		}

		switch ((CMDLIST)pack_head.cmdtype)
		{
			case LOOK:
			{
				ret = recv(fd, listen_buffer, pack_head.msg_len, 0);
				if (ret == SOCKET_ERROR) {
					std::cout << "RECV data error" << std::endl;
					return RECV_FAILED;
				}
				char *name = &listen_buffer[sizeof(size_t)];
				SOCKET *clifd = (SOCKET *)&listen_buffer[0];
				char *Bmovep = (char *)clifd;
				std::cout << "online list" << std::endl;
				for (int leftlen = pack_head.msg_len; leftlen > 0; leftlen -= (sizeof(size_t) + 24))
				{
					std::cout << *clifd << std::endl;
					std::cout << name << std::endl;
					name += 24 + sizeof(size_t);
					Bmovep += 24 + sizeof(size_t);
					clifd = (SOCKET *)Bmovep;
				}
				memset(listen_buffer, 0, 40 * 1024);
				break;
			}
			case TALK:
			{
				std::cout << std::endl << "recv from:" << pack_head.name << std::endl;
				ret = recv(fd, listen_buffer, pack_head.msg_len, 0);
				if (ret == SOCKET_ERROR) {
					std::cout << "RECV data error" << std::endl;
					return RECV_FAILED;
				}
				std::cout << listen_buffer << std::endl;
				memset(listen_buffer, 0, 40 * 1024);
				break; 
			}
			case ZHUCE:
			{
				ret = recv(fd, listen_buffer, pack_head.msg_len, 0);
				if (ret == SOCKET_ERROR) {
					std::cout << "RECV data error" << std::endl;
					return RECV_FAILED;
				}
				if (listen_buffer[0] == ZHUCE_FAILED_FLAG)
					zhuCeFlag = ZHUCE_FAILED_FLAG;
				else
					zhuCeFlag = ZHUCE_SUCCESS_FLAG;
				break;
			}
			default:
			{
				break;
			}
		}
	}
	free(listen_buffer);
}
