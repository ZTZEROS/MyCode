// Acho_Client.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
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

// 화면 출력용 전역변수
int g_iConnectTry;				// 커넥트 시도 횟수
int g_iConnectFail;				// 커넥트 실패 횟수
int g_iConnectSuccess;			// 커넥트 성공 횟수

// 에러 발생 횟수 체크하는 카운트. 증가하면 절대 초기화되지 않는다.
UINT64 g_iErrorCount;			


int _tmain()
{
	_tsetlocale(LC_ALL, _T("Korean"));

	srand(time(NULL));

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

	UINT64 TimeCheck = timeGetTime();
	DWORD junkTIme;
	const DWORD  OneFrame = 1000;

	while (1)
	{	
		// 리시브 대기중이 아닌 더미는, 문자열 생성 후 SendBuff에 넣는다.
		DummyWork_CreateString();

		// 네트워크 프로세스 호출
		Network_Process();			

		// 1초에 1회 디버깅 하기
		if (timeGetTime() - TimeCheck >= OneFrame)
		{
			printf("/////////////////////////////////////////////\n1ConnectTry : %d\nConnectFail : %d\nConnectSuccess : %d\nLaytency Avg : %d ms,  TPS : %d,  Error Count : %lld\n\n\n",
				g_iConnectTry, g_iConnectFail, g_iConnectSuccess, GetAvgLaytency(), GetTPS(), g_iErrorCount);

			TimeCheck = timeGetTime();
		}			
	}

	timeEndPeriod(1);

	return 0;
}

