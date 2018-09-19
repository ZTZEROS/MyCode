// CPUUsage_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "CPUUsage\CPUUsage.h"

#include <process.h>

using namespace Library_Jingyu;

UINT	WINAPI	TestThread(LPVOID lParam);

int _tmain()
{
	CCpuUsage_Process ProcessUsage;
	CCpuUsage_Processor ProcessorUsage;

	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, TestThread, 0, 0, 0);

	while (1)
	{
		ProcessUsage.UpdateCpuTime();
		ProcessorUsage.UpdateCpuTime();
		_tprintf_s(_T(	"CPU usage [T:%.1f%% U:%.1f%% K:%.1f%%] [Now:%.1f%% U:%.1f%% K:%.1f%%]\n"),
						ProcessorUsage.ProcessorTotal(), ProcessorUsage.ProcessorUser(), ProcessorUsage.ProcessorKernel(),
						ProcessUsage.ProcessTotal(), ProcessUsage.ProcessUser(), ProcessUsage.ProcessKernel());

		Sleep(1000);
	}	

}

UINT	WINAPI	TestThread(LPVOID lParam)
{
	int a = 0;
	while (1)
	{
		a++;
		a--;
	}

	return 0;
}

