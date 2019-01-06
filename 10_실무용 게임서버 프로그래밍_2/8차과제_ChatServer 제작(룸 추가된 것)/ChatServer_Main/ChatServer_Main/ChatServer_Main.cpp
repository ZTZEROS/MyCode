// ChatServer_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "ChatServer_Room.h"

#include <conio.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace Library_Jingyu;

int _tmain()
{
	timeBeginPeriod(1);

	system("mode con: cols=80 lines=36");

	// 채팅서버 생성
	CChatServer_Room ChatServer;

	if (ChatServer.ServerStart() == false)
	{
		printf("Server OpenFail...\n");
		return 0;
	}

	// 출력
	while (1)
	{
		Sleep(1000);

		//if (_kbhit())
		//{
		//	int Key = _getch();

		//	// q를 누르면 채팅서버 종료
		//	if (Key == 'Q' || Key == 'q')
		//	{
		//		ChatServer.ServerStop();
		//		break;
		//	}

		//}

		ChatServer.ShowPrintf();
	}

	timeEndPeriod(1);
	return 0;
}
