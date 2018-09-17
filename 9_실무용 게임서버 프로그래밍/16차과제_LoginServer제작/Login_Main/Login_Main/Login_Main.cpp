// Login_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <Windows.h>
#include "LoginServer\LoginServer.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")


// 출력용 코드
LONG g_lAllocNodeCount;
LONG g_lStruct_PlayerCount;

extern ULONGLONG g_ullAcceptTotal;
extern ULONGLONG g_ullDisconnectTotal;
extern LONG	  g_lAcceptTPS;
extern LONG	g_lSendPostTPS;

extern ULONGLONG	g_ullTrensferdCount;


LONG g_lAllocNodeCount_Lan;




using namespace Library_Jingyu;

int _tmain()
{
	timeBeginPeriod(1);

	system("mode con: cols=80 lines=29");

	srand((UINT)time(NULL));

	CLogin_NetServer LoginS;	

	if (LoginS.ServerStart() == false)
		return 0;

	while (1)
	{
		Sleep(1000);

		// 화면 출력할 것 셋팅
		/*
		SessionNum : 	- NetServer 의 세션수
		PacketPool_Net : 	- 외부에서 사용 중인 Net 직렬화 버퍼의 수

		PlayerData_Pool :	- Player 구조체 할당량
		Player Count :		- Contents 파트 Player의 수 (밖에서 사용중인 노드 수 포함)

		LoginSessionKey : 	- 아직 미사용
		Accept Total :		- Accept 전체 카운트 (accept 리턴시 +1)
		Disconnect Total:	- OnSend를 받아서, Disconnect를 보낸 총 횟수

		Accept TPS :		- Accept 처리 횟수		
		Send TPS			- 초당 Send완료 횟수. (완료통지에서 증가)

		Net_BuffChunkAlloc_Count : - Net 직렬화 버퍼 총 Alloc한 청크 수 (밖에서 사용중인 청크 수)
		Player_ChunkAlloc_Count : - 플레이어 총 Alloc한 청크 수 (밖에서 사용중인 청크 수)

		----------------------------------------------------
		SessionNum : 	- LanServer 의 세션수
		PacketPool_Lan : 	- 외부에서 사용 중인 Lan 직렬화 버퍼의 수

		Lan_BuffChunkAlloc_Count : - Lan 직렬화 버퍼 총 Alloc한 청크 수 (밖에서 사용중인 청크 수)
		*/

		LONG AccpetTPS = g_lAcceptTPS;
		LONG SendTPS = g_lSendPostTPS;
		InterlockedExchange(&g_lAcceptTPS, 0);
		InterlockedExchange(&g_lSendPostTPS, 0);

		printf("========================================================\n"
			"SessionNum : %lld\n"
			"PacketPool_Net : %d\n\n"

			"PlayerData_Pool : %d\n"
			"Player Count : %d\n\n"

			"LoginSessionKey : Non\n"
			"Accept Total : %lld\n"
			"Disconnect Total : %lld\n"
			"Transferred : %lld (%lld)\n\n"

			"Accept TPS : %d\n"
			"Send TPS : %d\n\n"

			"Net_BuffChunkAlloc_Count : %d (Out : %d)\n"
			"Player_ChunkAlloc_Count : %d (Out : %d)\n\n"

			"------------------------------------------------\n"
			"SessionNum : %lld\n"
			"PacketPool_Lan : %d\n\n"

			"Lan_BuffChunkAlloc_Count : %d (Out : %d)\n\n"

			"========================================================\n\n",
			// ----------- 로그인 넷 서버용
			LoginS.GetClientCount(), g_lAllocNodeCount,
			g_lStruct_PlayerCount, LoginS.JoinPlayerCount(),
			g_ullAcceptTotal, g_ullDisconnectTotal, g_ullTrensferdCount, g_ullDisconnectTotal+ g_ullTrensferdCount,
			AccpetTPS, SendTPS,
			CProtocolBuff_Net::GetChunkCount(), CProtocolBuff_Net::GetOutChunkCount(), 
			LoginS.GetPlayerChunkCount(), LoginS.GetPlayerOutChunkCount(),
			
			// ----------- 로그인 랜 서버용
			LoginS.GetLanClientCount(), g_lAllocNodeCount_Lan,
			CProtocolBuff_Lan::GetChunkCount(), CProtocolBuff_Lan::GetOutChunkCount());
	}

	timeEndPeriod(1);
	

	return 0;
}
