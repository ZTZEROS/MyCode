// ConsoleApplication1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

#pragma comment(lib,"ws2_32")
#include <WS2tcpip.h>

#include <process.h>

#include "ProtocolBuff\ProtocolBuff(Net)_ObjectPool.h"

using namespace Library_Jingyu;
using namespace std;

UINT WINAPI TestThread(LPVOID lParam);

//#define THREAD_COUNT	10

LONG g_lDelay;
LONG g_lSendByte;
LONG g_lRecvByte;
LONG g_lCheckSumError;

BYTE bCode = 0x11;
BYTE bXORCode_1 = 0xc9;
BYTE bXORCode_2 = 0x88;

int _tmain()
{
	printf("Code : %d, XORCode1 : %d, XORCode2 : %d\n", bCode, bXORCode_1, bXORCode_2);
	cout << "Delay(Sleep) : ";
	cin >> g_lDelay;

	int ThradCount = 0;
	cout << "Thread : ";
	cin >> ThradCount;

	system("mode con: cols=120 lines=7");

	HANDLE *hThread = new HANDLE[ThradCount];

	// 소켓 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Error1\n");
		return 0;
	}

	// 스레드 만들기
	for (int i = 0; i < ThradCount; ++i)
	{
		hThread[i] = (HANDLE)_beginthreadex(0, 0, TestThread, (void*)i, 0, 0);
	}

	while (1)
	{
		Sleep(1000);

		printf("======================================================================\n"
			"Delay : %d, Thread : %d\n"
			"SendByte(sec) : %d\n"
			"RecvByte(sec) : %d\n"
			"CheckSumError : %d\n"
			"======================================================================\n",
			g_lDelay, ThradCount, g_lSendByte, g_lRecvByte, g_lCheckSumError);

		g_lSendByte = g_lRecvByte = 0;

	}

	WSACleanup();
	return 0;

}


UINT WINAPI TestThread(LPVOID lParam)
{
	int CountIndex = (int)lParam;

	// 소켓 생성
	SOCKET clinet_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clinet_sock == INVALID_SOCKET)
	{
		printf("Error2\n");
		return 0;
	}

	// connect
	SOCKADDR_IN clientaddr;
	ZeroMemory(&clientaddr, sizeof(clientaddr));

	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(6000);
	InetPton(AF_INET, L"127.0.0.1", &clientaddr.sin_addr.s_addr);

	if (connect(clinet_sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr)) == SOCKET_ERROR)
	{
		DWORD Error = WSAGetLastError();
		printf("Error3 (%d)\n", Error);
		return 0;
	}

	while (1)
	{
		// 보내기
		char PayloadTest[] = "abcdefghiijklnm";

		// Send
		CProtocolBuff_Net Buff;
		Buff.PutData((const char*)PayloadTest, sizeof(PayloadTest));
		Buff.Encode(bCode, bXORCode_1, bXORCode_2);

		int SendByte = send(clinet_sock, Buff.GetBufferPtr(), Buff.GetUseSize(), 0);
		InterlockedAdd(&g_lSendByte, SendByte);

		// Recv
		CProtocolBuff_Net Buff2;
		int HopeByte = sizeof(PayloadTest) + 5;
		int AddByte = HopeByte;
		int Index = 0;
		while (1)
		{
			int RecvByte = recv(clinet_sock, &Buff2.GetBufferPtr()[Index], HopeByte, 0);

			HopeByte -= RecvByte;

			if (HopeByte == 0)
				break;

			Index += RecvByte;
		}

		InterlockedAdd(&g_lRecvByte, AddByte);

		Buff2.MoveWritePos(sizeof(PayloadTest));

		// 한번에 풀기

		if (Buff2.Decode((BYTE*)Buff2.GetBufferPtr(), bXORCode_1, bXORCode_2) == false)
			InterlockedIncrement(&g_lCheckSumError);

		Sleep(g_lDelay);
	}

	closesocket(clinet_sock);

	return 0;

}