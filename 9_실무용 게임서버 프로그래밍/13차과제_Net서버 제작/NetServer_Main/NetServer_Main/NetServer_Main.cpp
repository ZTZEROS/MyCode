// NetServer_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "ChatServer.h"

#include <time.h>
#include <process.h>
#include <conio.h>

using namespace Library_Jingyu;

// ------------------------------------------------
// 각종 define
// ------------------------------------------------
#define SERVER_IP	L"0.0.0.0"
#define SERVER_PORT	12001

#define CREATE_WORKER_THREAD	4
#define ACTIVE_WORKER_THREAD	2
#define CREATE_ACCEPT_THREAD	1


// 출력용 코드
LONG g_lAllocNodeCount;
LONG g_lFreeNodeCount;

ULONGLONG g_ullUpdateStructCount;
ULONGLONG g_ullUpdateStruct_PlayerCount;

ULONGLONG g_ullAcceptTotal;
LONG	  g_lAcceptTPS;
LONG	  g_lUpdateTPS;


int m_SectorPosError = 0;
int m_SectorByteError = 0;
int m_SectorNoError = 0;
int m_HeadCodeError = 0;


int _tmain()
{
	//system("mode con: cols=120 lines=18");

	srand((UINT)time(NULL));

	// 채팅서버
	CChatServer ChatS;

	BYTE HeadCode = 0x77;
	BYTE XORCode_1 = 0x32;
	BYTE XORCode_2 = 0x84;

	// 채팅서버 시작
	if (ChatS.ServerStart(SERVER_IP, SERVER_PORT, CREATE_WORKER_THREAD, ACTIVE_WORKER_THREAD, CREATE_ACCEPT_THREAD, false, 6000, HeadCode, XORCode_1, XORCode_2) == false)
		printf("Server OpenFail...\n");

	while (1)
	{
		Sleep(1000);

		if (_kbhit())
		{
			int Key = _getch();

			// q를 누르면 채팅서버 종료
			if (Key == 'Q' || Key == 'q')
			{
				ChatS.ServerStop();
				break;
			}

		}
		
		// 화면 출력할 것 셋팅
		/*
		SessionNum : 	- NetServer 의 세션수
		PacketPool : 	- Packet 풀 할당량

		UpdateMessage_Pool :	- UpdateThread 용 구조체 할당량 (일감)
		UpdateMessage_Queue : 	- UpdateThread 큐 남은 개수

		PlayerData_Pool :	- Player 구조체 할당량
		Player Count : 		- Contents 파트 Player 개수

		LoginSessionKey : 	- 아직 미사용
		Accept Total :		- Accept 전체 카운트 (accept 리턴시 +1)
		Accept TPS :		- Accept 처리 횟수
		Update TPS :		- Update 처리 초당 횟수

		SessionMiss : 		- 미사용
		SessionNotFound : 	- 미사용
		*/
		
		//printf("%d , %d\n", g_lAllocNodeCount, g_lFreeNodeCount);

		printf(	"========================================================\n"
				"SessionNum : %lld\n"
				"PacketPool : %d\n\n"

				"UpdateMessage_Pool : %lld\n"
				"UpdateMessage_Queue : %d\n\n"

				"PlayerData_Pool : %lld\n"
				"Player Count : %d\n\n"

				"LoginSessionKey : Non\n"
				"Accept Total : %lld\n"
				"Accept TPS : %d\n"
				"Update TPS : %d\n"
				
				"SecotrPosError : %d\n"
				"SectorByteError : %d\n"
				"SectorAccountError : %d\n"
				"HeadCodeError : %d\n"
				"========================================================\n\n",
				ChatS.GetClientCount(), g_lAllocNodeCount, g_ullUpdateStructCount, ChatS.GetQueueInNode(),
				g_ullUpdateStruct_PlayerCount, ChatS.JoinPlayerCount(),
				g_ullAcceptTotal, g_lAcceptTPS, g_lUpdateTPS,
				m_SectorPosError, m_SectorByteError, m_SectorNoError, m_HeadCodeError);


		InterlockedCompareExchange(&g_lUpdateTPS, 0, g_lUpdateTPS);
		InterlockedCompareExchange(&g_lAcceptTPS, 0, g_lAcceptTPS);			
	}	
		

	return 0; 
}

