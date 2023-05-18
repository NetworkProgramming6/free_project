// Network.h: interface for the CNetwork class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETWORK_H__A9EBDDB2_06F0_4293_B5C0_C405B75CC26B__INCLUDED_)
#define AFX_NETWORK_H__A9EBDDB2_06F0_4293_B5C0_C405B75CC26B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
// 각종 선언
//////////////////////////////////////////////////////////////////////////
#define MSGBUFFER 512
#define TYPE_NONE	0
#define TYPE_SERVER 1
#define TYPE_CLIENT 2

#include <winsock2.h>
#include <conio.h>
#include <iostream>

using namespace std;

class CNetwork  
{
public:
	void SetType(int nFlag);
	bool Recv(SOCKET hSocket, char* pMessage, long cbMessage);
	bool Send(SOCKET hSocket, char szMessage[], long cbMessage);
	SOCKADDR_IN GetLastAddress();
	SOCKET Connect(char szIpAddress[], unsigned short usPort);
	SOCKET Accept();
	bool Listen(int nConnection);
	bool Bind(unsigned short usPort);
	CNetwork();
	virtual ~CNetwork();

protected:
	int m_nServiceType;
	SOCKADDR_IN m_hRemoteAddr;
	SOCKET m_hLocalSocket;
};

#endif // !defined(AFX_NETWORK_H__A9EBDDB2_06F0_4293_B5C0_C405B75CC26B__INCLUDED_)
