// Network.cpp: implementation of the CNetwork class.
//
//////////////////////////////////////////////////////////////////////

#include "Network.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetwork::CNetwork()
{
	WSADATA	wsaData;
	int		nRetCode;

	// ���� Ÿ���� �ʱ�ȭ �Ѵ�
	m_nServiceType = TYPE_NONE;
	
	// ������ �ʱ�ȭ �Ѵ�
	nRetCode = WSAStartup( MAKEWORD(2,2), &wsaData );

	if ( nRetCode != NO_ERROR )
	{
		cout << "���� : " << WSAGetLastError() << endl;

		return;
	}

	// ������ �ʱ�ȭ �Ѵ�
	m_hLocalSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if( m_hLocalSocket == INVALID_SOCKET )
	{
		cout << "���� : " << WSAGetLastError() << endl;

		return;
	}
	
	return;
}

CNetwork::~CNetwork()
{
	// ������ �ݴ´�
	closesocket( m_hLocalSocket );

	// ���� �ʱ�ȭ
    WSACleanup();
}

//////////////////////////////////////////////////////////////////////////
// �ּҿ� ��Ʈ �Ҵ� (������)
//////////////////////////////////////////////////////////////////////////
bool CNetwork::Bind(unsigned short usPort)
{
	SOCKADDR_IN hLocalService;
	int			nRetCode;

	if( m_nServiceType != TYPE_SERVER )
	{
		cout << "���� : ���� Ÿ�� ������ �߸��Ǿ��ֽ��ϴ�" << endl;

		return false;
	}

	hLocalService.sin_family = AF_INET;
	hLocalService.sin_addr.s_addr = INADDR_ANY;
	hLocalService.sin_port = htons( usPort );

	// �ּҿ� ��Ʈ �Ҵ�
	nRetCode = bind( m_hLocalSocket, (SOCKADDR*)&hLocalService, sizeof(hLocalService) );

	if( nRetCode == SOCKET_ERROR )
	{
		cout << "���� : " << WSAGetLastError() << endl;

		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// ���� ��û ���·� ���� (������)
//////////////////////////////////////////////////////////////////////////
bool CNetwork::Listen(int nConnection)
{
	int nRetCode;

	if( m_nServiceType != TYPE_SERVER )
	{
		cout << "���� : ���� Ÿ�� ������ �߸��Ǿ��ֽ��ϴ�" << endl;

		return false;
	}

	// ���� ��û ���·� ����
	nRetCode = listen( m_hLocalSocket, nConnection );

	if( nRetCode == SOCKET_ERROR )
	{
		cout << "���� : " << WSAGetLastError() << endl;

		return false;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Ŭ���̾�Ʈ�� ���� (������)
//////////////////////////////////////////////////////////////////////////
SOCKET CNetwork::Accept()
{
	SOCKET	hCLientSocket;
	int		cbRemote;

	if( m_nServiceType != TYPE_SERVER )
	{
		cout << "���� : ���� Ÿ�� ������ �߸��Ǿ��ֽ��ϴ�" << endl;

		return false;
	}
	
	cbRemote = sizeof( m_hRemoteAddr );

	// Ŭ���̾�Ʈ�� ���� ����
	hCLientSocket = accept( m_hLocalSocket, (SOCKADDR*)&m_hRemoteAddr, &cbRemote);

	if( hCLientSocket == INVALID_SOCKET )
	{
		cout << "���� : " << WSAGetLastError() << endl;
		
		return false;
	}

	return hCLientSocket;
}

//////////////////////////////////////////////////////////////////////////
// Ŭ���̾�Ʈ ����Ʈ ����ü (Ŭ���̾�Ʈ��)
//////////////////////////////////////////////////////////////////////////
SOCKET CNetwork::Connect(char szIpAddress[], unsigned short usPort)
{
	int nRetCode;

	if( m_nServiceType != TYPE_CLIENT )
	{
		cout << "���� : ���� Ÿ�� ������ �߸��Ǿ��ֽ��ϴ�" << endl;

		return SOCKET_ERROR;
	}

	m_hRemoteAddr.sin_family = AF_INET;
	m_hRemoteAddr.sin_addr.s_addr = inet_addr( szIpAddress );
	m_hRemoteAddr.sin_port = htons( usPort );

	// ������ ���� ��û
	nRetCode = connect( m_hLocalSocket, (SOCKADDR*)&m_hRemoteAddr, sizeof(m_hRemoteAddr) );

	if( nRetCode == SOCKET_ERROR )
	{
		cout << "���� : " << WSAGetLastError() << endl;

		return SOCKET_ERROR;
	}

	return m_hLocalSocket;
}

//////////////////////////////////////////////////////////////////////////
// ������ Ŭ���̾�Ʈ�� �ּ� ���� ����
//////////////////////////////////////////////////////////////////////////
SOCKADDR_IN CNetwork::GetLastAddress()
{
	// �����϶� : ���� �ֱٿ� ������ Ŭ���̾�Ʈ �ּ����� ����
	// Ŭ���϶� : ���� �ֱٿ� ���� ������ ������ �ּ����� ����
	return m_hRemoteAddr;
}

//////////////////////////////////////////////////////////////////////////
// �޽��� ����
//////////////////////////////////////////////////////////////////////////
bool CNetwork::Send(SOCKET hSocket, char szMessage[], long cbMessage)
{
	int nRetCode;

	// �޽��� ����
	nRetCode = send( hSocket, szMessage, cbMessage, 0 );

	if( nRetCode == SOCKET_ERROR )
	{
		cout << "���� : " << WSAGetLastError() << endl;

		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// �޽��� ����
//////////////////////////////////////////////////////////////////////////
bool CNetwork::Recv(SOCKET hSocket, char* pMessage, long cbMessage)
{
	int nRetCode;

	memset( pMessage, 0, cbMessage );

	// �޽��� ����
	nRetCode = recv( hSocket, pMessage, cbMessage, 0 );

	if( nRetCode == SOCKET_ERROR )
	{
		cout << "���� : " << WSAGetLastError() << endl;

		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// ���� Ÿ�� ���� - �������� Ŭ���̾�Ʈ���� �����ؾ� ��
//////////////////////////////////////////////////////////////////////////
void CNetwork::SetType(int nFlag)
{
	if( m_nServiceType != TYPE_NONE )
	{
		cout << "���� : ���� Ÿ���� �缳�� �Ҽ� �����ϴ�" << endl;

		return;
	}

	m_nServiceType = nFlag;
	
	return;
}