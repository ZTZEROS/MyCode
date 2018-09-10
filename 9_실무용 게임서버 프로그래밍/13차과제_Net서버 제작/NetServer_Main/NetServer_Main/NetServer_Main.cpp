// NetServer_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "ChatServer.h"
#include "Parser\Parser_Class.h"

#include <time.h>
#include <process.h>
#include <conio.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

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

LONG g_lUpdateStructCount;
LONG g_lUpdateStruct_PlayerCount;

extern ULONGLONG g_ullAcceptTotal;
extern LONG	  g_lAcceptTPS;
extern LONG	g_lSendPostTPS;
LONG	  g_lUpdateTPS;


int m_SectorPosError = 0;

int m_SectorNoError = 0;

int m_ChatNoError = 0;

int m_TypeError = 0;

int m_HeadCodeError = 0;
int m_ChackSumError = 0;
int m_HeaderLenBig = 0;

int m_NetBuffAlloc = 0;
int m_MessageAlloc = 0;
int m_PlayerAlloc = 0;


int _tmain()
{
	timeBeginPeriod(1);

	system("mode con: cols=80 lines=30");	

	srand((UINT)time(NULL));

	// 채팅서버
	CChatServer ChatS;

	// 채팅서버 시작
	if (ChatS.ServerStart() == false)
	{
		printf("Server OpenFail...\n");
		return 0;
	}

	while (1)
	{
		Sleep(1000);

		//if (_kbhit())
		//{
		//	int Key = _getch();

		//	// q를 누르면 채팅서버 종료
		//	if (Key == 'Q' || Key == 'q')
		//	{
		//		ChatS.ServerStop();
		//		break;
		//	}

		//}
		
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
		Send TPS			- 초당 Send완료 횟수. (완료통지에서 증가)

		SecotrPosError :	- 섹터 위치 에러
		SectorAccountError : - 섹터 패킷의 AccountNo 에러
		ChatAccountError :	- 채팅 패킷의 AccountNo 에러
		TypeError :			- 페이로드의 메시지 타입 에러
		HeadCodeError :		- (네트워크) 헤더 코드 에러
		CheckSumError :		- (네트워크) 체크썸 에러	
		HeaderLenBig :		- (네트워크) 헤더의 Len사이즈가 비정상적으로 큼.

		SessionMiss : 		- 미사용
		SessionNotFound : 	- 미사용
		*/

		LONG AccpetTPS = g_lAcceptTPS;
		LONG UpdateTPS = g_lUpdateTPS;		
		LONG SendTPS = g_lSendPostTPS;
		InterlockedCompareExchange(&g_lUpdateTPS, 0, g_lUpdateTPS);
		InterlockedCompareExchange(&g_lAcceptTPS, 0, g_lAcceptTPS);
		InterlockedCompareExchange(&g_lSendPostTPS, 0, g_lSendPostTPS);

		printf("========================================================\n"
			"SessionNum : %lld\n"
			"PacketPool : %d\n\n"

			"UpdateMessage_Pool : %d\n"
			"UpdateMessage_Queue : %d\n\n"

			"PlayerData_Pool : %d\n"
			"Player Count : %d\n\n"

			"LoginSessionKey : Non\n"
			"Accept Total : %lld\n"
			"Accept TPS : %d\n"
			"Update TPS : %d\n"
			"Send TPS : %d\n\n"

			"SecotrPosError : %d\n"
			"SectorAccountError : %d\n"
			"ChatAccountError : %d\n"
			"TypeError : %d\n"
			"HeadCodeError : %d\n"
			"CheckSumError : %d\n"
			"HeaderLenBig : %d\n\n"

			"Net_BuffChunkAlloc_Count : %d (Out : %d)\n"
			"Chat_MessageChunkAlloc_Count : %d (Out : %d)\n"
			"Chat_PlayerChunkAlloc_Count : %d (Out : %d)\n"
			"========================================================\n\n",
			ChatS.GetClientCount(), g_lAllocNodeCount, g_lUpdateStructCount, ChatS.GetQueueInNode(),
			g_lUpdateStruct_PlayerCount, ChatS.JoinPlayerCount(),
			g_ullAcceptTotal, AccpetTPS, UpdateTPS, SendTPS,
			m_SectorPosError, m_SectorNoError, m_ChatNoError, m_TypeError, m_HeadCodeError, m_ChackSumError, m_HeaderLenBig,
			CProtocolBuff_Net::GetChunkCount(), CProtocolBuff_Net::GetOutChunkCount(), ChatS.GetWorkChunkCount(), ChatS.GetWorkOutChunkCount(), ChatS.GetPlayerChunkCount(), ChatS.GetPlayerOutChunkCount());

		
	}	
		
	timeEndPeriod(1);

	return 0; 
}