// TCP_Dummy.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#include <locale>

#include "Network_Func.h"

// 입력받은 IP
TCHAR g_tIP[30];

// Dummy 카운트
int g_iDummyCount;

// 로그용
int		g_LogLevel;			// 로그 출력 레벨.
TCHAR	g_szLogBuff[1024];	//  로그 출력용 임시 버퍼

// 화면 출력용 전역변수
DWORD	g_WhileCount;			// 1초동안 처리한 로직 카운트
int g_iConnectTry;				// 커넥트 시도 횟수
int g_iConnectFail;				// 커넥트 실패 횟수
int g_iConnectSuccess;			// 커넥트 성공 횟수
DWORD g_dwJoinUserCount;		// Accept된 유저 수
DWORD g_dwSyncCount;			// 싱크받은 횟수 카운트

// RTT 체크용
// 1초동안 RTT를 보낸 후 받은 횟수
int g_RTTSendCount;

// RTT Max
ULONGLONG g_RTTMax;


// Recv, Send바이트
int g_iRecvByte;
int g_iSendByte;

// 에러 발생 횟수 체크하는 카운트. 증가하면 절대 초기화되지 않는다.
UINT64 g_iErrorCount;

// 업데이트 부분
void Update();

int _tmain()
{
	_tsetlocale(LC_ALL, _T("Korean"));

	timeBeginPeriod(1);

	// 1. IP입력받기
	fputs("IP 입력 : ", stdout);
	_tscanf_s(_T("%s"), g_tIP, (UINT)sizeof(g_tIP));

	// 2. Dummy 카운트 입력받기
	fputs("Dummy 카운트 입력 : ", stdout);
	scanf_s("%d", &g_iDummyCount);

	// 3. 네트워크 초기화
	if (Network_Init(&g_iConnectTry, &g_iConnectFail, &g_iConnectSuccess) == false)
		return 0;

	while (1)
	{
		Network_Process();		

		Update();

		g_WhileCount++;
		
		// 다른 프로세스에 CPU 양보
		
	}
	

	timeEndPeriod(1);

    return 0;
}

// 업데이트 부분
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
	static DWORD dwFPS = 0;

	// 현재 시간 구해오기.
	ULONGLONG uiNowTime = GetTickCount64();

	if ((uiNowTime - uiStartTime) >= (g_dw1Frame - dwSave))
	{
		// ---------------------------------
		// 1초 단위 처리
		// ---------------------------------	
		if (uiNowTime - uiShowTimeCheck >= 1000)
		{			
			_tprintf(L"--------------------\nConnect Try : %d, Success : %d, Fail : %d, NowAccpet : [%d]\nFrame : %d, LogicCount : %d, RecvPacket : %d Byte,  SendPacket : %d Byte\nRTT Avr : %lldms, Max %lldms, CheckCount : %d, [SyncCount : %d]\n\n",
				g_iConnectTry, g_iConnectSuccess, g_iConnectFail, g_dwJoinUserCount,
				dwFPS, g_WhileCount, g_iRecvByte, g_iSendByte,
				RTTAvr(), g_RTTMax, g_RTTSendCount, g_dwSyncCount);

			// 다음 1초 체크를 위해 변수들 갱신
			uiShowTimeCheck = uiNowTime;

			dwFPS = 0;
			g_WhileCount = 0;
			g_iRecvByte = 0;
			g_iSendByte = 0;
			g_RTTMax = 0;
			g_RTTSendCount = 0;

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
			DummyAI();

			// Update로직 처리 수 증가	
			dwFPS++;			

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

