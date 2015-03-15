#include "../Communication/Interface.h"
#include <iostream>
#include <windows.h>
using namespace std;

#pragma comment(lib, "Communication.lib")

void* GetClientMess(void *pData = 0, int length = 0, void* pSocket = 0)
{
	if(0 != pData)
	{
		cout<<(char*)pData<<endl;
	}

	return 0;
}

int main()
{
	if(InitClient("127.0.0.1", 5181, GetClientMess))
	{
		while(1)
		{
			Sleep(1000);
			Transmit("okkkkkkkkkkkk", 20);
		}
	}

	system("pause");
	return 0;
}

