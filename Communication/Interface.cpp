#include "Interface.h"
#include <winsock2.h>
#include <process.h>
#include <iostream>
#include <vector>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT 5181
#define SIZET 1024

struct NewSocket
{
	int ID; //每一个新socket的ID
	SOCKET sock;
};
vector<NewSocket> vNewSocket;

struct	myStruct
{
	SOCKET	Socket;
	GetMess pGetMess;
};

WSADATA wsadata_Server;
SOCKADDR_IN sockaddr_Server;
SOCKET listenSock;
bool bServerDown = false;

WSADATA wsadata_Client;
SOCKADDR_IN sockaddr_Client;
SOCKET clientSock; //客户端socket
bool bClientDown = false;//控制客户端通信开关

//关闭服务端socket
bool CloseServer(bool type)
{
	bServerDown = type;

	return true;
}

//服务器线程
void ServerThread(void* pGetMess)
{
	GetMess getMess = (GetMess)pGetMess;
	SOCKET sock;
	int selectErr;
	char* pStorage = new char[SIZET];
	
	while(!bServerDown)
	{
		cout<<"主线程"<<endl;

		fd_set fdread;
		fd_set fdwrite;
		timeval tv;
		FD_ZERO(&fdread);//初始化fd_set
		FD_ZERO(&fdwrite);
		FD_SET(listenSock, &fdread);//分配套接字句柄到相应的fd_set
		FD_SET(listenSock, &fdwrite);
		//将每一个新连接的socket添加fd_set中， 包括读和写
		for(vector<NewSocket>::iterator iter = vNewSocket.begin(); iter != vNewSocket.end(); iter++)
		{
			FD_SET(iter->sock, &fdread);
			FD_SET(iter->sock, &fdwrite);
		}
		tv.tv_sec=1;
		tv.tv_usec=0;

		//selectErr = select(0, &fdread, &fdwrite, NULL, &tv);
		selectErr = select(0, &fdread, NULL, NULL, &tv);
		if(SOCKET_ERROR == selectErr) 
		{
			cout<<"error"<<endl;
		}

		if(FD_ISSET(listenSock,&fdread))
		{

			//sock = accept(listenSock, &soso, &len);
			sock = accept(listenSock, NULL, NULL);
			if(sock != INVALID_SOCKET)
			{
				//将新socket加入队列
				NewSocket newSocket;
				newSocket.ID = vNewSocket.size() + 1; //设置ID
				newSocket.sock = sock;
				vNewSocket.push_back(newSocket);
			}
		}

		for(vector<NewSocket>::iterator iter = vNewSocket.begin(); iter != vNewSocket.end(); iter++)
		{
			//可以读socket
			if(FD_ISSET(iter->sock, &fdread))
			{
				memset(pStorage, 0, SIZET);
				int recvErr = recv(iter->sock, pStorage, SIZET, 0);
				if(0 == recvErr)
				{
					//主动关闭socket
					closesocket(iter->sock);
					vNewSocket.erase(iter);
					cout<<"error"<<endl;
					break;
				}
				if(SOCKET_ERROR == recvErr)
				{
					//异常情况， 先移除此socket
					closesocket(iter->sock);
					vNewSocket.erase(iter);
					cout<<"error"<<endl;
					break;
				}
				getMess(pStorage, SIZET, &(iter->sock));
			}
		}
	}

	delete []pStorage;
	closesocket(listenSock);
	WSACleanup();
}

//初始化服务端
bool InitServer(GetMess pGetMess)
{
	sockaddr_Server.sin_family = AF_INET;
	sockaddr_Server.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr_Server.sin_port = htons(PORT);

	int er = WSAStartup(0x0202, &wsadata_Server);
	if(WSAVERNOTSUPPORTED == er){}

	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(INVALID_SOCKET == listenSock){}

	int bindErr = bind(listenSock, (PSOCKADDR)&sockaddr_Server, sizeof(sockaddr));
	if(SOCKET_ERROR == bindErr){}

	int listenErr = listen(listenSock, 5);
	if(SOCKET_ERROR == listenErr){}

	_beginthread(ServerThread, 0, pGetMess);

	return true;
}

//关闭客户端socket
bool CloseClient(bool type)
{
	bClientDown = type;

	return true;
}

//客户端线程
void ClientThread(void* pGetMess)
{
	int selectErr;
	GetMess getMess = (GetMess)pGetMess;
	char* pStorage = new char[SIZET];
	while(!bClientDown)
	{
		fd_set fdread;
		fd_set fdwrite;
		timeval tv;
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_SET(clientSock, &fdread);
		tv.tv_sec=1;
		tv.tv_usec=0;

		selectErr = select(0, &fdread, &fdwrite, NULL, &tv);
		if(SOCKET_ERROR == selectErr) 
		{
			cout<<"error"<<endl;
		}

		//获取数据
		if(FD_ISSET(clientSock, &fdread))
		{
			memset(pStorage, 0, SIZET);
			int recvErr = recv(clientSock, pStorage, SIZET, 0);
			if(0 == recvErr)
			{
				//主动关闭socket
				cout<<"error"<<endl;
				closesocket(clientSock);
				WSACleanup();
				return;
			}
			if(SOCKET_ERROR == recvErr)
			{
				cout<<"error"<<endl;
				closesocket(clientSock);
				WSACleanup();
				break;
			}
			//将获取数据传出
			getMess(pStorage, SIZET, 0);
		}
	}

	delete []pStorage;
	closesocket(clientSock);
	WSACleanup();
}

//初始化客户端
bool InitClient(char* szIP, int port, GetMess pGetMess)
{
	sockaddr_Client.sin_family = AF_INET;
	sockaddr_Client.sin_addr.s_addr = inet_addr(szIP);
	sockaddr_Client.sin_port = htons(port);

	int startErr = WSAStartup(0x0202, &wsadata_Client);
	if(WSAVERNOTSUPPORTED == startErr)
	{
		cout<<"error"<<endl;
	}

	clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(INVALID_SOCKET == clientSock)
	{
		cout<<"error"<<endl;
	}

	int conErr = connect(clientSock, (PSOCKADDR)&sockaddr_Client, sizeof(sockaddr));
	if(SOCKET_ERROR == conErr)
	{
		cout<<"error"<<endl;
	}
	
	_beginthread(ClientThread, 0, pGetMess);
	
	return true;
}

//发送消息
bool Transmit(void *pData,int length, void* pSocket)
{
	SOCKET curSocket;

	if(NULL == pSocket)
	{
		curSocket = clientSock;
	}
	else
	{
		curSocket = *(SOCKET*)pSocket;
	}

	int sendErr = send(curSocket, (char*)pData, length, 0);
	if(SOCKET_ERROR == sendErr)
	{

	}
	
	return TRUE;
}


