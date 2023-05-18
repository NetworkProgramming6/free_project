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

	// 서비스 타입을 초기화 한다
	m_nServiceType = TYPE_NONE;
	
	// 윈속을 초기화 한다
	nRetCode = WSAStartup( MAKEWORD(2,2), &wsaData );

	if ( nRetCode != NO_ERROR )
	{
		cout << "에러 : " << WSAGetLastError() << endl;

		return;
	}

	// 소켓을 초기화 한다
	m_hLocalSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if( m_hLocalSocket == INVALID_SOCKET )
	{
		cout << "에러 : " << WSAGetLastError() << endl;

		return;
	}
	
	return;
}

CNetwork::~CNetwork()
{
	// 소켓을 닫는다
	closesocket( m_hLocalSocket );

	// 윈속 초기화
    WSACleanup();
}

//////////////////////////////////////////////////////////////////////////
// 주소와 포트 할당 (서버용)
//////////////////////////////////////////////////////////////////////////
bool CNetwork::Bind(unsigned short usPort)
{
	SOCKADDR_IN hLocalService;
	int			nRetCode;

	if( m_nServiceType != TYPE_SERVER )
	{
		cout << "에러 : 서비스 타입 설정이 잘못되어있습니다" << endl;

		return false;
	}

	hLocalService.sin_family = AF_INET;
	hLocalService.sin_addr.s_addr = INADDR_ANY;
	hLocalService.sin_port = htons( usPort );

	// 주소와 포트 할당
	nRetCode = bind( m_hLocalSocket, (SOCKADDR*)&hLocalService, sizeof(hLocalService) );

	if( nRetCode == SOCKET_ERROR )
	{
		cout << "에러 : " << WSAGetLastError() << endl;

		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 연결 요청 상태로 변경 (서버용)
//////////////////////////////////////////////////////////////////////////
bool CNetwork::Listen(int nConnection)
{
	int nRetCode;

	if( m_nServiceType != TYPE_SERVER )
	{
		cout << "에러 : 서비스 타입 설정이 잘못되어있습니다" << endl;

		return false;
	}

	// 연결 요청 상태로 진입
	nRetCode = listen( m_hLocalSocket, nConnection );

	if( nRetCode == SOCKET_ERROR )
	{
		cout << "에러 : " << WSAGetLastError() << endl;

		return false;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 클라이언트와 연결 (서버용)
//////////////////////////////////////////////////////////////////////////
SOCKET CNetwork::Accept()
{
	SOCKET	hCLientSocket;
	int		cbRemote;

	if( m_nServiceType != TYPE_SERVER )
	{
		cout << "에러 : 서비스 타입 설정이 잘못되어있습니다" << endl;

		return false;
	}
	
	cbRemote = sizeof( m_hRemoteAddr );

	// 클라이언트의 연결 수락
	hCLientSocket = accept( m_hLocalSocket, (SOCKADDR*)&m_hRemoteAddr, &cbRemote);

	if( hCLientSocket == INVALID_SOCKET )
	{
		cout << "에러 : " << WSAGetLastError() << endl;
		
		return false;
	}

	return hCLientSocket;
}

//////////////////////////////////////////////////////////////////////////
// 클라이언트 리스트 구조체 (클라이언트용)
//////////////////////////////////////////////////////////////////////////
SOCKET CNetwork::Connect(char szIpAddress[], unsigned short usPort)
{
	int nRetCode;

	if( m_nServiceType != TYPE_CLIENT )
	{
		cout << "에러 : 서비스 타입 설정이 잘못되어있습니다" << endl;

		return SOCKET_ERROR;
	}

	m_hRemoteAddr.sin_family = AF_INET;
	m_hRemoteAddr.sin_addr.s_addr = inet_addr( szIpAddress );
	m_hRemoteAddr.sin_port = htons( usPort );

	// 서버에 연결 요청
	nRetCode = connect( m_hLocalSocket, (SOCKADDR*)&m_hRemoteAddr, sizeof(m_hRemoteAddr) );

	if( nRetCode == SOCKET_ERROR )
	{
		cout << "에러 : " << WSAGetLastError() << endl;

		return SOCKET_ERROR;
	}

	return m_hLocalSocket;
}

//////////////////////////////////////////////////////////////////////////
// 서버나 클라이언트의 주소 정보 리턴
//////////////////////////////////////////////////////////////////////////
SOCKADDR_IN CNetwork::GetLastAddress()
{
	// 서버일때 : 가장 최근에 접속한 클라이언트 주소정보 리턴
	// 클라일때 : 가장 최근에 접속 성공한 서버의 주소정보 리턴
	return m_hRemoteAddr;
}

//////////////////////////////////////////////////////////////////////////
// 메시지 전송
//////////////////////////////////////////////////////////////////////////
bool CNetwork::Send(SOCKET hSocket, char szMessage[], long cbMessage)
{
	int nRetCode;

	// 메시지 전송
	nRetCode = send( hSocket, szMessage, cbMessage, 0 );

	if( nRetCode == SOCKET_ERROR )
	{
		cout << "에러 : " << WSAGetLastError() << endl;

		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 메시지 수신
//////////////////////////////////////////////////////////////////////////
bool CNetwork::Recv(SOCKET hSocket, char* pMessage, long cbMessage)
{
	int nRetCode;

	memset( pMessage, 0, cbMessage );

	// 메시지 수신
	nRetCode = recv( hSocket, pMessage, cbMessage, 0 );

	if( nRetCode == SOCKET_ERROR )
	{
		cout << "에러 : " << WSAGetLastError() << endl;

		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 서비스 타입 설정 - 서버인지 클라이언트인지 지정해야 함
//////////////////////////////////////////////////////////////////////////
void CNetwork::SetType(int nFlag)
{
	if( m_nServiceType != TYPE_NONE )
	{
		cout << "에러 : 서비스 타입을 재설정 할수 없습니다" << endl;

		return;
	}

	m_nServiceType = nFlag;
	
	return;
}