// MPool_TLS.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "ObjectPool\Object_Pool_LockFreeVersion.h"

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

#define CREATE_THREAD_COUNT 4
#define ALLOC_COUNT 230


ULONGLONG ThreadCount[CREATE_THREAD_COUNT] = { 0, };

CMemoryPoolTLS<CTest>* g_MPool = new CMemoryPoolTLS<CTest>(200, false);
CCrashDump* g_CDump = CCrashDump::GetInstance();

UINT	WINAPI	TestThread(LPVOID lParam);

int _tmain()
{
	timeBeginPeriod(1);
	HANDLE ThreadHandle[CREATE_THREAD_COUNT];

	// 스레드 4개 만들기
	for (int i = 0; i < CREATE_THREAD_COUNT; ++i)
	{
		ThreadHandle[i] = (HANDLE)_beginthreadex(NULL, 0, TestThread, (void*)i, 0, NULL);
	}
	   
	while (1)
	{
		Sleep(1000);
		
		printf(	"---------------------------------------------------------------"
				"AllocChunkCount : %d	OutChunkCount : %d\n"
				"ThreadLoop : [ %lld ], [ %lld ], [ %lld ], [ %lld ]\n"
				"---------------------------------------------------------------"
				, g_MPool->GetAllocChunkCount(), g_MPool->GetOutChunkCount()
				, ThreadCount[0], ThreadCount[1], ThreadCount[2], ThreadCount[3]);
	}

	timeEndPeriod(1);

	return 0;
}


UINT	WINAPI	TestThread(LPVOID lParam)
{

	CTest* cTestArray[ALLOC_COUNT];
	ULONGLONG TempAddr[ALLOC_COUNT];
	LONG TempCount[ALLOC_COUNT];

	int ID = (int)lParam;

	while (1)
	{	
		// 1. Alloc
		for (int i = 0; i < ALLOC_COUNT; ++i)
		{
			cTestArray[i] = g_MPool->Alloc();
		}

		// 2. Addr 비교 (0x0000000055555555가 맞는지)
		for (int i = 0; i < ALLOC_COUNT; ++i)
		{
			if (cTestArray[i]->Addr != 0x0000000055555555)
				g_CDump->Crash();

			// 3. 인터락으로 Addr, Count ++
			TempAddr[i] = InterlockedIncrement(&cTestArray[i]->Addr);
			TempCount[i] = InterlockedIncrement(&cTestArray[i]->Count);
		}

		// 3. 잠시 대기 (Sleep)
		//Sleep(0);

		// 4. Addr과 Count 다시 비교
		for (int i = 0; i < ALLOC_COUNT; ++i)
		{
			if (cTestArray[i]->Addr != TempAddr[i])
				g_CDump->Crash();

			else if (cTestArray[i]->Count != TempCount[i])
				g_CDump->Crash();

			// 6. 인터락으로 Addr, Count --
			TempAddr[i] = InterlockedDecrement(&cTestArray[i]->Addr);
			TempCount[i] = InterlockedDecrement(&cTestArray[i]->Count);
		}


		// 5. 잠시 대기 (Sleep)
		//Sleep(0);

		// 6. Addr과 Count가 0x0000000055555555, 0이 맞는지 확인
		for (int i = 0; i < ALLOC_COUNT; ++i)
		{
			if (cTestArray[i]->Addr != TempAddr[i])
				g_CDump->Crash();

			else if (cTestArray[i]->Count != TempCount[i])
				g_CDump->Crash();
		}

		// 7. Free
		for (int i = 0; i < ALLOC_COUNT; ++i)
		{
			if(g_MPool->Free(cTestArray[i]) == false)
				g_CDump->Crash();
			
		}

		ThreadCount[ID]++;
	}
	return 0;
	

}
