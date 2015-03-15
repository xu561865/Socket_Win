#ifndef _INTERFACE_
#define _INTERFACE_

#ifdef EXPORT_INTERFACE
#define COMPII extern "C" _declspec(dllexport) 
#else
#define COMPII extern "C" _declspec(dllimport) 
#endif
//获取接收的数据
typedef void* (*GetMess)(void *pData, int length, void* pSocket);

COMPII bool InitServer(GetMess pGetMess);
COMPII bool InitClient(char* szIP, int port, GetMess pGetMess);
COMPII bool Transmit(void *pData = 0,int length = 0, void* pSocket = 0);
COMPII bool CloseClient(bool type);
COMPII bool CloseServer(bool type);
#endif