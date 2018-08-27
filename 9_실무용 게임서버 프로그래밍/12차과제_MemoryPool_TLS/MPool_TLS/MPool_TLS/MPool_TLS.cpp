// MPool_TLS.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "ObjectPool\Object_Pool_LockFreeVersion.h"
#include "profiling\Profiling_Class.h"

#include <process.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")


using namespace Library_Jingyu;

class CTest
{
public:
	ULONGLONG Addr = 0x0000000055555555;
	LONG Count = 0;
};

CMemoryPoolTLS<CTest>* g_MPool = new CMemoryPoolTLS<CTest>(5010, false);

#define COUNT	1000000
#define THREAD_COUNT	2

CTest* Test1[COUNT];
CTest* Test2[COUNT];

UINT TestThread(LPVOID lParam);

int _tmain()
{	
	FREQUENCY_SET(); // 주파수 구하기	
	timeBeginPeriod(1);	

	// 스레드 생성
	HANDLE hThread[THREAD_COUNT];

	for (int i = 0; i < THREAD_COUNT; ++i)
		hThread[i] = (HANDLE)_beginthreadex(NULL, 0, TestThread, (void*)i, 0, 0);
	
	// 모든 스레드 종료 대기
	WaitForMultipleObjects(THREAD_COUNT, hThread, TRUE, INFINITE);
	
	// 프로파일링 파일 저장
	PROFILING_FILE_SAVE();	

	for (int i = 0; i < THREAD_COUNT; ++i)
		CloseHandle(hThread[i]);

	printf("End\n");
	timeEndPeriod(1);

	return 0;
}



UINT TestThread(LPVOID lParam)
{
	int Type = (int)lParam;

	const int loopCount = 100;

	switch (Type)
	{
	case 0:
		// 백만번씩, 100번한다.TLSAlloc
		for (int i = 0; i < loopCount; ++i)
		{
			BEGIN("TLSAlloc");
			for (int i = 0; i < COUNT; ++i)
			{				
				Test1[i] = g_MPool->Alloc();				
			}	
			END("TLSAlloc");

			//BEGIN("TLSFree");
			for (int i = 0; i < COUNT; ++i)
			{
				g_MPool->Free(Test1[i]);				
			}
			//END("TLSFree");
		}

		break;

	case 1:
		// 백만번씩, 100번한다. new
		for (int i = 0; i < loopCount; ++i)
		{
			BEGIN("new");
			for (int i = 0; i < COUNT; ++i)
			{			
				Test2[i] = new CTest;				
			}	
			END("new");

			/*BEGIN("delete");
			for (int i = 0; i < COUNT; ++i)
			{				
				delete Test2[i];				
			}
			END("delete");*/
		}
		break;
	}

	return 0;	
}