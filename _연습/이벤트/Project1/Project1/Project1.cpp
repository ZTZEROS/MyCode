// Project1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <windows.h>
#include <process.h>

TCHAR	string[100];
HANDLE	hEvent;

UINT	WINAPI	OutPutThreadFunc1(LPVOID lpParam); 
UINT	WINAPI	OutPutThreadFunc2(LPVOID lpParam);


int _tmain()
{
	HANDLE	hThread[2];
	DWORD	dwThreadID;

	// 책 : 2번 인자가 TRUE면 수동, FALSE면 자동
	// TREU면 모든 스레드가 다 깨어난다. Non-Signaled가 자동으로 안된다. 
	// FALSE면 하나만 깨운다. 깨어난 후 자동으로 Event는 Non-Signaled가 된다.
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	hThread[0] = (HANDLE)_beginthreadex(NULL, 0, OutPutThreadFunc1, NULL, 0, (unsigned*)&dwThreadID);
	hThread[1] = (HANDLE)_beginthreadex(NULL, 0, OutPutThreadFunc2, NULL, 0, (unsigned*)&dwThreadID);
		
	_fputts(L"OK\n", stdout);

	SetEvent(hEvent);

	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	CloseHandle(hEvent);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

    return 0;
}

UINT	WINAPI	OutPutThreadFunc1(LPVOID lpParam)
{
	WaitForSingleObject(hEvent, INFINITE);
	_fputts(L"OutPutThreadFunc1\n", stdout);

	return 0;
}

UINT	WINAPI	OutPutThreadFunc2(LPVOID lpParam)
{
	WaitForSingleObject(hEvent, INFINITE);
	_fputts(L"OutPutThreadFunc2\n", stdout);

	return 0;

}


