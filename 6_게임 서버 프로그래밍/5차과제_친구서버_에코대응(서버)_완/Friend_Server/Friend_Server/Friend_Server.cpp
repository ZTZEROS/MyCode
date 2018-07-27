// Friend_Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.

#include "stdafx.h"
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#include <locale>
#include <conio.h>

#include "Network_Func.h"

// 디버깅용 변수 (Network_func.cpp에서 extern 변수로 사용중. 조심하자!)
UINT64 g_AcceptCount;							// Accept 한 유저 수
UINT64 g_PacketCount;							// 1초에 주고받은 패킷 수. (recv 횟수만 체크함. 어차피 send랑 1:1로 대응되니까)
UINT64 g_SendByteCount;							// 1초동안 받은 패킷 바이트 수 
UINT64 g_RecvByteCount;							// 1초동안 보낸 패킷 바이트 수


// Loop체크용 변수. (Network_func.cpp 에서 extern 변수로 사용중. 조심하자!)
UINT64 g_LoopNowCount;							// 현재 루프 횟수
UINT64 g_LoopMinCount = 99999999;				// 현재 루프횟수를 기준으로 Min값 넣기
UINT64 g_LoopAvgCount;							// 현재 루프횟수를 기준으로 Avg값 넣기
UINT64 g_LoopMaxCount;							// 현재 루프횟수를 기준으로 Max값 넣기

UINT64 g_TotalLoop;								// 현재까지 돈 총 루프 횟수(평균계산용)
UINT64 g_AddCount;								// 총 루프 횟수에 더한 횟수(평균계산용)


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
	UINT64 TimeCheck = timeGetTime();
	while (1)
	{
		// 1초에 1회 디버깅 하기
		if (timeGetTime() - TimeCheck >= 1000)
		{
			// 1. 현재 루프 횟수가, 기존의 MIn루프 횟수보다 작다면, Min 셋팅
			if (g_LoopNowCount < g_LoopMinCount)
				g_LoopMinCount = g_LoopNowCount;

			// 2. 현재 루프 횟수가, 기존의 Max루프 횟수보다 크다면, Max 셋팅
			if (g_LoopNowCount > g_LoopMaxCount)
				g_LoopMaxCount = g_LoopNowCount;

			// 3. 평균 계산
			g_TotalLoop += g_LoopNowCount;
			g_LoopAvgCount = g_TotalLoop / ++g_AddCount;

			_tprintf(_T("/////////////////////////////////////////////\nconnect : %lld\n1Frame_LoopCount : %lld,  AvgLoopCount : %lld,  MinLoopCount : %lld,  MaxLoopCount : %lld\nTPS : %lld,  SPS : %lld Byte,   RPS : %lld Byte\n\n\n"),
				g_AcceptCount, g_LoopNowCount, g_LoopAvgCount, g_LoopMinCount, g_LoopMaxCount, g_PacketCount, g_SendByteCount, g_RecvByteCount);
				

			// 5. 다음 1초 계산을 위해 시간 갱신
			TimeCheck = timeGetTime();

			// 6. 다음 루프 체크를 위해 각종 카운트 초기화
			g_LoopNowCount = 0;
			g_PacketCount = 0;
			g_SendByteCount = 0;
			g_RecvByteCount = 0;

		}			

		// 네트워크
		if (Network_Process(&listen_sock) == false)
			break;

		// 키 누름 체크 (락, 종료, 제이슨 저장 등..)
		if (LockCheck() == false)
		{
			// 제이슨 저장 함수 호출
			Json_Create();
			break;
		}

		// 현재 루프횟수 1 추가.
		g_LoopNowCount++;
	}

	// 리슨 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();

	_tprintf(_T("정상 종료!\n"));

	timeEndPeriod(1);

	return 0;
}

// 락 체크 및 종료 체크
// 종료 시, false가 리턴됨.
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