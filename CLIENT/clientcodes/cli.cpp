#include "client.h"
#include <iostream>

int main()
{
	int ret;
	client c1;

	ret = c1.client_login();
	if (ret != 0)
	{
		printf("something is wrong!");
		return 0;
	}
	std::cout << "connect success" << std::endl;

	ret = c1.client_talkToServer();

}