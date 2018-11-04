// MatchTestClient_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "Network_Func.h"
#include "Parser/Parser_Class.h"
#include <process.h>

#define THREAD_COUNT	2
#define THREAD_IN_USER	50

UINT WINAPI	DummyThread(LPVOID lParam);

struct ThreadInUser
{
	int m_iStartAccountNo;	// 시작 어카운트 넘버
	int m_iCount;	// 몇 명의 유저를 할당받았는가
};

int _tmain()
{
	// 스레드 수와 스레드 당 유저 수 파싱	
	HANDLE* hThread = new HANDLE[THREAD_COUNT];
	ThreadInUser* stUserIn = new ThreadInUser[THREAD_COUNT];

	// 스레드 수 만큼 스레드 생성
	// 생성하면서, 각 스레드가 몇 명의 유저를 담당해야하는지와 해당 유저들의 AccountNo넘긴다.
	for (int i = 0; i < THREAD_COUNT; ++i)
	{
		stUserIn[i].m_iCount = THREAD_IN_USER;
		stUserIn[i].m_iStartAccountNo = (THREAD_IN_USER*i) + 1;

		hThread[i] = (HANDLE)_beginthreadex(NULL, NULL, DummyThread, &stUserIn[i], NULL, NULL);
	}

	while (1)
	{
		Sleep(INFINITE);
		// 출력용 무언가를 한다.
	}

	return 0;	
}

UINT WINAPI	DummyThread(LPVOID lParam)
{
	// 1. 할당된 유저 수와 시작 AccountNo 알아오기
	ThreadInUser* NowThis = (ThreadInUser*)lParam;

	// 2. 더미 만든다.
	cMatchTestDummy Dummy;

	// 3. Create()
	// 미리 DB에 들어가 있어야 함.. 세션키도 동일하게.
	Dummy.CreateDummpy(NowThis->m_iCount, NowThis->m_iStartAccountNo);

	// 4. 매치메이킹 서버 알아오기
	Dummy.MatchInfoGet();

	// 5. DummyRun
	while (1)
	{
		Dummy.DummyRun();
		Sleep(1);
	}

	return 0;
}