// Chat_Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <locale>

#include "Network_Func.h"

int main()
{
	setlocale(LC_ALL, "korean");

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		_tprintf(_T("WSAStartup 실패!\n"));
		return 1;
	}

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
	{
		_tprintf(_T("listen_sock / socket 실패!\n"));
		return 1;
	}

	// 바인딩
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(dfNETWORK_PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	DWORD dCheck = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (dCheck == SOCKET_ERROR)
	{
		DWORD error = WSAGetLastError();
		_tprintf(_T("bind() 에러 : %d\n"), error);
		return 1;
	}

	// 리슨 상태
	dCheck = listen(listen_sock, SOMAXCONN);
	if (dCheck == SOCKET_ERROR)
	{
		DWORD error = WSAGetLastError();
		_tprintf(_T("listen() 에러 : %d\n"), error);
		return 1;
	}

	// 논블락 소켓으로 변경
	ULONG on = 1;
	dCheck = ioctlsocket(listen_sock, FIONBIO, &on);
	if (dCheck == SOCKET_ERROR)
	{
		_tprintf(_T("논블로킹 소켓 변경 에러\n"));
		return 1;
	}


	_tprintf(_T("ChatServer Open...\n"));
	// 클라와 통신
	while (1)
	{
		// 네트워크
		bool check = Network_Process(&listen_sock);
		if (check == false)
			break;
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();

	_tprintf(_T("정상 종료!\n"));
	return 0;
}

