// Friend_Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.

#include "stdafx.h"
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#include <locale>
#include <conio.h>

#include "Network_Func.h"


// 락 체크 및 종료 체크
// 종료 시, 제이슨 저장 후 false가 리턴됨.
bool LockCheck();

int _tmain()
{
	timeBeginPeriod(1);

	_tsetlocale(LC_ALL, _T("Korean"));

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

	// Json에서 데이터 읽어온 후 셋팅하기
	Json_Get();

	_tprintf(_T("Friend Server Open...\n"));

	// 클라와 통신
	bool check;
	UINT64 TimeCheck = timeGetTime();
	while (1)
	{
		// 키 누름 체크 (락, 종료, 제이슨 저장 등..)
		check = LockCheck();
		if (check == false)
		{
			// 제이슨 저장 함수 호출
			Json_Create();
			break;
		}

		// 1초에 1회 디버깅 하기
		if (timeGetTime() - TimeCheck >= 1000)
		{
			_tprintf(_T("connect : %lld\n"), AcceptCount());
			TimeCheck = timeGetTime();
		}

		// 네트워크
		check = Network_Process(&listen_sock);
		if (check == false)
			break;
	}

	// 리슨 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();

	_tprintf(_T("정상 종료!\n"));

    return 0;
}

// 락 체크 및 종료 체크
// 종료 시, 제이슨 저장 후 false가 리턴됨.

bool LockCheck()
{
	static bool Lock = true;	// 락 상태 체크. 기본 걸려있는 상태

	int Key;
	if (_kbhit())
	{
		Key = _getch();

		// u를 눌렀다면
		if (Key == 'u')
		{
			// 현재 락 상태일 경우, 락 상태 해제
			if (Lock == true)
			{
				Lock = false;
				_tprintf(_T("락 해제! q를 눌러 제이슨 저장 및 종료 가능. 다시 u를 누르면 락 상태로 전환.\n"));
			}

			// 락 상태가 아니라면, 락상태로 변경
			else
			{
				Lock = true;
				_tprintf(_T("락 설정! q를 눌러도 아무 반응 없을것임. 다시 u를 누르면 락 상태 해제\n"));
			}
		}

		// q를 눌렀다면
		else if (Key == 'q')
		{
			// 락이 풀렸다면 false 리턴
			if (Lock == false)
				return false; // false 리턴으로 밖에서 제이슨 저장후 종료하도록 유도							
		}
	}

	return true;
}
