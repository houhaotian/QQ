#include "client.h"
#include <iostream>
#include "..\\..\\public\\errcode.h"

int main()
{
	int ret;
	client c1;

	ret = c1.client_login();
	if (ret != RUN_SUCCESS)
	{
		printf("something is wrong!");
		return 0;
	}
	std::cout << "connect success" << std::endl;

	ret = c1.client_talkToServer();

}