// TCP_Fighter_Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")


#include <locale>
#include "Network_Func.h"


DWORD	WhileCount;
int		g_LogLevel;			// 로그 출력 레벨.
TCHAR	g_szLogBuff[1024];	//  로그 출력용 임시 버퍼

// 업데이트
void Update();

int _tmain()
{
	_tsetlocale(LC_ALL, _T("Korean"));

	timeBeginPeriod(1);

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("WSAStartup() Fail...\n");
		return 0;
	}
	
	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
	{
		printf("Create Socket Fail...\n");
		return 0;
	}

	// 바인딩
	SOCKADDR_IN	serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);
	serveraddr.sin_addr.S_un.S_addr = INADDR_ANY;
	int check = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (check == SOCKET_ERROR)
	{
		printf("bind() Fail...\n");
		return 0;
	}

	// 리슨상태
	check = listen(listen_sock, SOMAXCONN);
	if (check == SOCKET_ERROR)
	{
		printf("listen() Fail...\n");
		return 0;
	}

	// 논블락 소켓으로 변경
	ULONG on = 1;
	check = ioctlsocket(listen_sock, FIONBIO, &on);
	if (check == SOCKET_ERROR)
	{
		printf("NoneBlock Change Fail...\n");
		return 0;
	}

	// 로그 레벨 입력받기
	fputs("LogLevel (0:Debuf / 1:Warning / 2:Error) : ", stdout);
	scanf_s("%d", &g_LogLevel);

	// 여기까지 오면 서버 정상오픈
	SYSTEMTIME lst;
	GetLocalTime(&lst);

	_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] Server Open... Port : %d\n",
		lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, SERVERPORT);
	
	while (1)
	{
		Network_Process(&listen_sock);

		Update();
		WhileCount++;
	}

	timeEndPeriod(1);

    return 0;
}

// 업데이트
void Update()
{
	// 1프레임은 20m/s
	static const DWORD g_dw1Frame = 20;

	// 프레임 스킵을 위한 시간 저장
	static ULONGLONG uiStartTime = GetTickCount64();
	static LONGLONG dwSave = 0;

	// 화면 출력(1초 단위) 체크를 위한 시간 저장
	static ULONGLONG uiShowTimeCheck = uiStartTime;

	// 1초동안 Update를 처리한 횟수.
	static DWORD dwLogicCount = 0;		

	// 현재 시간 구해오기.
	ULONGLONG uiNowTime = GetTickCount64();

	if ((uiNowTime - uiStartTime) >= (g_dw1Frame - dwSave))
	{
		// ---------------------------------
		// 1초 단위 처리
		// ---------------------------------	
		if (uiNowTime - uiShowTimeCheck >= 1000)
		{
			// FPS가 50이 아니라면, 시간과 FPS, Loop카운트를 찍는다
			if (dwLogicCount != 50)
			{
				SYSTEMTIME lst;
				GetLocalTime(&lst);
				// [년/월/일 시/분/초] FPS카운트, Loop카운트 순서대로 표시
				_LOG(dfLOG_LEVEL_DEBUG, L"[%02d/%02d/%02d %02d:%02d:%02d] FPS : %d  Loop : %d (%lld)\n",
					lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond,
					dwLogicCount, WhileCount, uiNowTime - uiShowTimeCheck);

			}			

			// 출력과 상관없이 다음 1초 체크를 위해 변수들 갱신
			uiShowTimeCheck = GetTickCount64();
			dwLogicCount = 0;
			WhileCount = 0;
		}

		bool bFirstCheck = true;

		// 업데이트 로직 처리
		while (1)
		{
			// ---------------------------------
			// Update 처리
			// ---------------------------------
			// 모든 플레이어의 Action처리.
			// 업데이트 처리.
			ActionProc();

			// Update로직 처리 수 증가	
			dwLogicCount++;

			// ---------------------------------
			// 다음 로직 처리를 위한 체크
			// ---------------------------------
			if (bFirstCheck == true)
			{
				dwSave += (uiNowTime - uiStartTime) - g_dw1Frame;
				uiStartTime = uiNowTime;
			}

			//printf("%d\n", dwSave);

			if (dwSave < g_dw1Frame)
				break;

			else
			{
				dwSave -= g_dw1Frame;
				bFirstCheck = true;
			}
			
		}
	}

}

