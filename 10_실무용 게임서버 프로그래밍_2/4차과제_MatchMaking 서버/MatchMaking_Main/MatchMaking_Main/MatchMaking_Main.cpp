#include "pch.h"
#include "MatchmakingServer/MatchmakingServer.h"

using namespace Library_Jingyu;

int _tmain()
{
	system("mode con: cols=80 lines=33");

	Matchmaking_Net_Server MatchServer;

	// 매칭서버 시작
	if (MatchServer.ServerStart() == false)
		return 0;


	// 1초에 1회 화면에 출력
	while (1)
	{
		Sleep(1000);

		// 화면 출력 함수 호출
		MatchServer.ShowPrintf();
	}

	return 0;
}

