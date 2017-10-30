#include "server.h"
#include <stdio.h>

int main()
{
	int ret;
	server s1;
	ret = s1.server_login();
	if (ret != 0)
	{
		printf("something is wrong!");
		return 0;
	}
}