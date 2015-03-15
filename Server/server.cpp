#include "../Communication//Interface.h"
#include <iostream>
#include <windows.h>
using namespace std;

#pragma comment(lib, "Communication.lib")
void *p = NULL;

void* GetSerMess(void *pData = 0, int length = 0, void* pSocket = 0)
{
	if(0 != pData)
	{
		cout<<(char*)pData<<endl;
		Transmit("hellooooooo", 20, pSocket);
	}
	

	return 0;
}

int main()
{
	if(!InitServer(GetSerMess))
	{
		cout<<"error"<<endl;
	}
	

	system("pause");
	CloseServer(true);
	return 0;
}