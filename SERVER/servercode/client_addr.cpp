#include "client_addr.h"
#include <string.h>

client_addr fill_arg(SOCKET fd, struct sockaddr_in addr,char *name)
{
	client_addr cli_addr;
	cli_addr.cli_addr = addr;
	cli_addr.cli_fd = fd;
	strcpy_s(cli_addr.name, name);
	return cli_addr;
}
