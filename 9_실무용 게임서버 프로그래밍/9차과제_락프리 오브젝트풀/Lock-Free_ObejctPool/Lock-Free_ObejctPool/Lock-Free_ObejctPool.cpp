// Lock-Free_ObejctPool.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <stdio.h>
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "CrashDump\CrashDump.h"
#include "profiling\Profiling_Class.h"
#include <Windows.h>
#include <process.h>
#include <conio.h>

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

using namespace Library_Jingyu;

#define ALLOC_COUNT	100000

class CTest
{
public:
	ULONGLONG Addr = 0x0000000055555555;
	LONG Count = 0;
};

CMemoryPool<CTest> g_MPool(0, false);
CCrashDump* g_CDump = CCrashDump::GetInstance();




///////  메모리풀 테스트 툴  //////////////////////////////////////////////////////////////////////////////////////////

// 여러개의 스레드에서 일정수량의 Alloc 과 Free 를 반복적으로 함
// 모든 데이터는 0x0000000055555555 으로 초기화 되어 있음.

// 0. Alloc (스레드당 10000 개 x 10 개 스레드 총 10만개)
// 1. 0x0000000055555555 이 맞는지 확인.
// 2. Interlocked + 1 (Data + 1 / Count + 1)
// 3. 약간대기
// 4. 여전히 0x0000000055555556 이 맞는지 (Count == 1) 확인.
// 5. Interlocked - 1 (Data - 1 / Count - 1)
// 6. 약간대기
// 7. 0x0000000055555555 이 맞는지 (Count == 0) 확인.
// 8. Free
// 반복.

// 테스트 목적
//
// - 할당된 메모리를 또 할당 하는가 ?
// - 잘못된 메모리를 할당 하는가 ?

// TestThread
UINT	WINAPI	TestThread(LPVOID lParam);

HANDLE Event;

int _tmain(void)
{
	timeBeginPeriod(1);	

	FREQUENCY_SET(); // 주파수 구하기	

	Event = CreateEvent(NULL, FALSE, FALSE, NULL);

	// 1. 스레드 생성
	// 10개의 스레드 x 스레드당 10000개의 블럭 사용
	HANDLE hThread[10];
	for (int i = 0; i < 1; ++i)
	{
		hThread[i] = (HANDLE)_beginthreadex(NULL, 0, TestThread, 0, 0, NULL);
	}

	
	// 2. 메인스레드는 1초에 1회, 정보 출력
	// [유저가 사용중인 블록 수 / 총 블록 수] 
	while (1)
	{
		Sleep(1000);

		if (_kbhit())
		{
			int Key = _getch();
			if (Key == 'q' || Key == 'Q')
			{
				SetEvent(Event);
				WaitForSingleObject(hThread[0], INFINITE);

				printf("Save..!\n");
				PROFILING_FILE_SAVE();
				printf("Save End!!\n");

				break;
			}
		}


		printf("User UseCount : %d		AllocCount : %d\n", g_MPool.GetUseCount(), g_MPool.GetAllocCount());
	}

	for (int i = 0; i < 1; ++i)
		CloseHandle(hThread[i]);

	CloseHandle(Event);

	timeEndPeriod(1);
	return 0;
}


// TestThread
UINT	WINAPI	TestThread(LPVOID lParam)
{
	while (1)
	{
		DWORD Check = WaitForSingleObject(Event, 0);
		if (Check == WAIT_OBJECT_0)
			break;

		CTest* cTestArray[10000];

		// 1. Alloc (1만개)
		/*for (int i = 0; i < 10000; ++i)
		{
			BEGIN("Alloc");
			cTestArray[i] = g_MPool.Alloc();
			END("Alloc");

			if(cTestArray[i] == nullptr)
				g_CDump->Crash();
		}	*/
		for (int i = 0; i < 10000; ++i)
		{
			BEGIN("Alloc");
			cTestArray[i] = new CTest;
			END("Alloc");

			if (cTestArray[i] == nullptr)
				g_CDump->Crash();
		}


		// 2. Free (1만개)
		/*for (int i = 0; i < 10000; ++i)
		{		
			BEGIN("Free");
			bool Check = g_MPool.Free(cTestArray[i]);
			END("Free");

			if (Check == false)
				g_CDump->Crash();
		}*/
		for (int i = 0; i < 10000; ++i)
		{
			BEGIN("Free");
			delete cTestArray[i];
			END("Free");

			if (Check == false)
				g_CDump->Crash();
		}
	}
	return 0;
}