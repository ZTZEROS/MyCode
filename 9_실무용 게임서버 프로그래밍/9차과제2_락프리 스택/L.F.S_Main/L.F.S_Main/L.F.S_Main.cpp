// L.F.S_Main.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "LockFree_Stack.h"

#include <process.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace Library_Jingyu;


int _tmain()
{
	CLF_Stack<int> LFS;
	
	for (int i = 0; i < 100; ++i)
	{
		LFS.Push(i);
	}

	/*for (int i = 0; i < 100; ++i)
	{
		printf("i : %d\n", LFS.Pop());
	}*/

	int abc = 10;


    return 0;
}

