// IOCP_Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"

#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>

#include <Windows.h>
#include <process.h>

#define SERVER_PORT	9000
#define BUFSIZE		512

struct SOCKETINFO
{
	OVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvBytes;
	int sendBytes;
	WSABUF wsabuf;
};

// 작업자 스레드 함수
UINT	WINAPI	WorkerThread(LPVOID lparam);


int _tmain()
{
	int retval;

	// 윈속초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 입출력 완료포트 생성
	HANDLE	hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL)
		return 1;

	// CPU 개수 확인
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// cpu 개수 * 2만큼 작업자 스레드 생성
	HANDLE hThread;

	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; ++i)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, hcp, 0, NULL);
		if (hThread == NULL)
			return 1;

		CloseHandle(hThread);
	}
	
	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
		return 1;

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == INVALID_SOCKET)
		return 1;

	// 리슨
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		fputs("listen() error\n", stdout);
		return 1;
	}

	// 데이터 통신에 사용되는 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	DWORD recvBytes, flag;

	while (1)
	{
		// accpet()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			fputs("accept error\n", stdout);
			break;
		}

		// 접속한 클라 출력
		char buf[30];
		inet_ntop(AF_INET, &clientaddr.sin_addr, (char*)&buf, sizeof(buf));
		printf("[TCP 서버] 클라 접속 : IP = %s, 포트 번호 = %d\n", buf, ntohs(clientaddr.sin_port));

		// 소켓과 입출력 완료포트 연결
		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);

		// 소켓 정보 구조체 할당
		SOCKETINFO	*ptr = new SOCKETINFO;
		if (ptr == NULL)
			break;

		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvBytes = ptr->sendBytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		// 비동기 입출력 시작
		flag = 0;
		retval = WSARecv(client_sock, &ptr->wsabuf, 1, &recvBytes, &flag, &ptr->overlapped, NULL);

		if (retval == SOCKET_ERROR)
		{
			if(WSAGetLastError() != ERROR_IO_PENDING)
				fputs("비동기 입출력 에러!\n", stdout);

		}

	}

	// 윈속 종료
	WSACleanup();
    return 0;
}

// 작업자 스레드 함수
UINT	WINAPI	WorkerThread(LPVOID lparam)
{
	int retval;

	HANDLE hcp = (HANDLE)lparam;

	while (1)
	{
		// 비동기 입출력 완료 대기
		DWORD cbTransferred;
		SOCKET clinet_sock;
		SOCKETINFO *ptr;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, &clinet_sock, (LPOVERLAPPED*)&ptr, INFINITE);
		

		// 클라 정보 얻기
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);

		getpeername(clinet_sock, (SOCKADDR*)&clientaddr, &addrlen);

		// 비동기 입출력 결과 확인
		if (retval == 0 || cbTransferred == 0)
		{
			if (retval == 0)
			{
				DWORD temp1, temp2;
				WSAGetOverlappedResult(ptr->sock, &ptr->overlapped, &temp1, FALSE, &temp2);

			}
			closesocket(clinet_sock);
			char buf[30];
			inet_ntop(AF_INET, &clientaddr.sin_addr, (char*)&buf, sizeof(buf));

			printf("TCp 서버] 클라이언트 종료 : IP = %s, 포트 = %d\n", buf, ntohs(clientaddr.sin_port));

			delete ptr;
			continue;
		}

		// 데이터 전송량 갱신
		if (ptr->recvBytes == 0)
		{
			ptr->recvBytes = cbTransferred;
			ptr->sendBytes = 0;

			// 받은 데이터 출력
			ptr->buf[ptr->recvBytes] = '\0';
			char buf[30];
			inet_ntop(AF_INET, &clientaddr.sin_addr, (char*)&buf, sizeof(buf));

			printf("[TCP %s:%d] %s\n", buf, ntohs(clientaddr.sin_port), ptr->buf);
		}

		else
			ptr->sendBytes += cbTransferred;

		// 데이터 보내기
		if (ptr->recvBytes > ptr->sendBytes)
		{
			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf + ptr->sendBytes;
			ptr->wsabuf.len = ptr->recvBytes - ptr->sendBytes;

			DWORD sendbytes;
			retval = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendbytes, 0, &ptr->overlapped, NULL);
			if (retval == SOCKET_ERROR)
			{
				if(WSAGetLastError() != WSA_IO_PENDING)
					fputs("데이터 보내기 실패!\n", stdout);

				continue;
			}
			
		}

		else
		{
			ptr->recvBytes = 0;

			// 데이터 받기
			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf;
			ptr->wsabuf.len = BUFSIZE;

			DWORD recvBytes;
			DWORD flags = 0;
			retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvBytes, &flags, &ptr->overlapped, NULL);
			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
					fputs("데이터 받기 실패!\n", stdout);

				continue;
			}		

		}

	}

	return 0;
}

