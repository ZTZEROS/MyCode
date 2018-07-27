// TCP_Fighter_Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")


#include <locale>
#include <conio.h>
#include "Network_Func.h"


DWORD	g_WhileCount;		// 1초동안 처리한 로직 카운트
int		g_LogLevel;			// 로그 출력 레벨.
TCHAR	g_szLogBuff[1024];	//  로그 출력용 임시 버퍼

int g_MaxFrameDelta = 0;	// 1프레임 사이 간격. Max 저장.

int g_FrameDeltaSave = 0;	// 1초동안의 1프레임 간격 사이 저장.

// 업데이트
void Update();

// 락 체크 및 종료 체크
bool LockCheck();

int _tmain()
{
	_tsetlocale(LC_ALL, _T("Korean"));

	// 로그 레벨 입력받기
	fputs("LogLevel (0:Debuf / 1:Warning / 2:Error) : ", stdout);
	scanf_s("%d", &g_LogLevel);

	FREQUENCY_SET(); // 주파수 구하기	

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

	// 여기까지 오면 서버 정상오픈
	SYSTEMTIME lst;
	GetLocalTime(&lst);

	_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] Server Open... Port : %d\n",
		lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, SERVERPORT);
	
	while (1)
	{
		Network_Process(&listen_sock);

		Update();
		g_WhileCount++;
		/*if (LockCheck() == false)
		{
			PROFILING_FILE_SAVE();
			fputs("Profilong_Save!\n", stdout);
			break;
		}*/

	}

	// 리슨 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();

	fputs("Exit!\n", stdout);

	timeEndPeriod(1);

    return 0;
}

// 업데이트
void Update()
{
	// 1프레임은 40m/s
	static const DWORD g_dw1Frame = 40;

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
			// FPS가 0~23이거나 27~이면, 현재 상태 로그로 저장.
			// 1프레임 정도는 허용해준다. 2프레임 차이부터 저장한다.
			if (dwLogicCount <= 23 || dwLogicCount >= 27)
			{			
				SYSTEMTIME lst;
				GetLocalTime(&lst);

				// [년/월/일 시/분/초] FPS카운트, Loop카운트 순서대로 표시
				_LOG(dfLOG_LEVEL_ERROR, L"[%02d/%02d/%02d %02d:%02d:%02d] [ERROR] FPS : %d  Loop : %d, FrameDelta Avr : %d,  FrameDelta Max : [%d]\n",
					lst.wYear - 2000, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond,
					dwLogicCount, g_WhileCount, g_FrameDeltaSave / dwLogicCount, g_MaxFrameDelta);
			}		

			// 그게 아니라면 그냥 출력만한다.
			else
			{
				_tprintf(L"FPS : %d  Loop : %d, FrameDelta Avr : %d,  FrameDelta Max : [%d] \n",
					dwLogicCount, g_WhileCount, g_FrameDeltaSave/ dwLogicCount, g_MaxFrameDelta);
			}

			// 출력과 상관없이 다음 1초 체크를 위해 변수들 갱신
			uiShowTimeCheck = uiNowTime;
			dwLogicCount = 0;
			g_WhileCount = 0;
			g_MaxFrameDelta = 0;
			g_FrameDeltaSave = 0;
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

			if (g_MaxFrameDelta < (uiNowTime - uiStartTime))
				g_MaxFrameDelta = uiNowTime - uiStartTime;

			g_FrameDeltaSave += uiNowTime - uiStartTime;

			// ---------------------------------
			// 다음 로직 처리를 위한 체크
			// ---------------------------------
			if (bFirstCheck == true)
			{
				dwSave += (uiNowTime - uiStartTime) - g_dw1Frame;
				uiStartTime = uiNowTime;
			}

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
				_tprintf(_T("락 해제! q를 눌러 프로파일링 저장 및 종료 가능. 다시 u를 누르면 락 상태로 전환.\n"));
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
