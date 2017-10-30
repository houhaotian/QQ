#include "packhead.h"
#include <string>

Pack_head construct_packethead(SOCKET source_fd, SOCKET des_fd, int messege_len, std::string myname, CMDLIST cmd)
{
	int name_len;
	Pack_head mypackhead;
	mypackhead.sfd = source_fd;
	mypackhead.dfd = des_fd;
	mypackhead.msg_len = messege_len;
	mypackhead.cmdtype = cmd;
	name_len = (int)myname.length();
	if (name_len > 24)//名字最大24字符
		name_len = 24;
	mypackhead.name_length = name_len;
	strncpy_s(mypackhead.name, myname.c_str(), name_len);
	return mypackhead;
}

/*message前面放pack_head,后面放纯数据,返回总数据长度*/
int get_package(char *message,const char* data, Pack_head mypackhead)
{
	memcpy(message, &mypackhead, sizeof(mypackhead));
	memcpy(message + sizeof(mypackhead), data, mypackhead.msg_len);
	message[sizeof(mypackhead) + mypackhead.msg_len] = '\0';
	return (mypackhead.msg_len + sizeof(mypackhead));
}
