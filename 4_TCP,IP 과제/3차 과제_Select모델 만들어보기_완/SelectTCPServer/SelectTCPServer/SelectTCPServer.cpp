// SelectTCPServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//
#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define SERVERPORT 9000
#define BUFFER_SIZE 512

struct SOCKETINFO
{
	SOCKET sock;
	char buf[BUFFER_SIZE + 1];
	int recvbyte;
	int sendbyte;
};

int nTotalSOcket = 0;
SOCKETINFO *SocketInfoArray[FD_SETSIZE];
TIMEVAL timeval;

BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int iIndex);

int main()
{
	timeval.tv_sec = 0;
	timeval.tv_usec = 0;
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
	{
		fputs("listen_sock socket() fail\n", stdout);
		return 1;
	}

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		fputs("listen_sock bind() fail\n", stdout);
		return 1;
	}

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR)
	{
		fputs("listen_sock listen() fail\n", stdout);
		return 1;
	}

	// 논 블로킹 소켓으로 변환
	ULONG on = 1;
	retval = ioctlsocket(listen_sock, FIONBIO, &on);
	if (retval == SOCKET_ERROR)
	{
		fputs("listen_sock ioctlsocket() fail\n", stdout);
		return 1;
	}

	// 데이터 통신에 사용할 변수
	FD_SET rset, wset;
	SOCKET clinet_sock;
	SOCKADDR_IN clinetaddr;
	int addrlen;

	while (1)
	{
		// 소켓 셋 초기화
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(listen_sock, &rset);
		for (int i = 0; i < nTotalSOcket; ++i)
		{
			// 받은 데이터(recvbyte)가 보낸 데이터(sendbyte)보다 크다면, 받을것을 다 받았다는 뜻이니, 데이터를 다시 샌드해야 한다.
			if (SocketInfoArray[i]->recvbyte > SocketInfoArray[i]->sendbyte)
				FD_SET(SocketInfoArray[i]->sock, &wset);

			// 보낸 데이터가 더 크거나 같다면, 보낼것을 다 보낸것이니, 데이터를 받아야 한다.
			else
				FD_SET(SocketInfoArray[i]->sock, &rset);
		}

		// Select()
		retval = select(0, &rset, &wset, NULL, &timeval);
		if(retval == SOCKET_ERROR)
		{
			fputs("select() fail\n", stdout);
			return 1;
		}

		// 소켓 셋 검사(1): 클라이언트 접속 수용
		// accept 가능여부 체크
		if (FD_ISSET(listen_sock, &rset))
		{
			addrlen = sizeof(clinetaddr);
			clinet_sock = accept(listen_sock, (SOCKADDR*)&clinetaddr, &addrlen);
			if (clinet_sock == INVALID_SOCKET)
				fputs("accept() fail\n", stdout);	
			else
			{
				TCHAR TextBuff[30];
				InetNtop(AF_INET, &clinetaddr.sin_addr, TextBuff, sizeof(TextBuff));
				_tprintf(_T("\naccept clinet!\nIP : %s, Port : %d\n\n"), 
					TextBuff, ntohs(clinetaddr.sin_port));

				// 소켓 정보 추가
				AddSocketInfo(clinet_sock);
			}
		}

		// 소켓 셋 검사(2): 데이터 통신
		for (int i = 0; i < nTotalSOcket; i++)
		{
			SOCKETINFO* ptr = SocketInfoArray[i];

			// 리시브 가능여부 체크
			if (FD_ISSET(ptr->sock, &rset))
			{
				// 가능하다면 리시브 진행(데이터 받기)
				retval = recv(ptr->sock, ptr->buf, BUFFER_SIZE, 0);
				if(retval == 0)
				{
					RemoveSocketInfo(i);
					continue;
				}
				else if (retval == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					printf("recv() fail : %d\n", error);
					RemoveSocketInfo(i);
					continue;
				}

				else if (retval == WSAEWOULDBLOCK)
					continue;

				ptr->recvbyte = retval;

				// 받은 데이터 출력
				addrlen = sizeof(clinetaddr);
				getpeername(ptr->sock, (SOCKADDR*)&clinetaddr, &addrlen);	// 1번 인자 소켓의 정보를 2번 인자에 넣어준다.
				ptr->buf[retval] = '\0';

				TCHAR TextBuff[30];
				InetNtop(AF_INET, &clinetaddr.sin_addr, TextBuff, sizeof(TextBuff));
				_tprintf(_T("[%s / %d] "), TextBuff, ntohs(clinetaddr.sin_port));
				printf("%s\n", ptr->buf);

			}

			// 샌드 가능여부 체크
			if (FD_ISSET(ptr->sock, &wset))
			{
				// 가능하다면 샌드 진행 (데이터 보내기)
				// 보내는 데이터의 버퍼 위치는 ptr->buf + ptr->sendbyte. sendbyte는 이미 보낸 데이터의 크기이니, 보낸 데이터만큼 위치 이동
				// 보내는 데이터의 크기는 ptr->recvbyte - ptr->sendbyte. recvbyte는 받은 데이터의 크기. 즉, 해당 데이터의 max 크기이다.
				// 여기서 보낸 데이터만큼(sendbyte)을 빼면 보내야하는 남은 데이터 크기가 나온다.
				retval = send(ptr->sock, ptr->buf + ptr->sendbyte, ptr->recvbyte - ptr->sendbyte, 0);
				if (retval == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					printf("send() fail : %d\n", error);
					RemoveSocketInfo(i);
					continue;
				}
				else if (retval == WSAEWOULDBLOCK)
					continue;

				ptr->sendbyte += retval;
				if (ptr->recvbyte == ptr->sendbyte)
				{
					ptr->recvbyte = ptr->sendbyte = 0;
				}

			}
			
		}

	}

	// 윈속 종료
	WSACleanup();

    return 0;
}

// 소켓 정보 추가
BOOL AddSocketInfo(SOCKET sock)
{
	if (nTotalSOcket >= FD_SETSIZE)
	{
		fputs("[error] nTotalSOcket over fail\n", stdout);
		return FALSE;
	}

	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL)
	{
		fputs("[error] AddSocketInfo fail. Memory lack\n", stdout);
		return FALSE;
	}

	ptr->sock = sock;
	ptr->recvbyte = 0;
	ptr->sendbyte = 0;
	SocketInfoArray[nTotalSOcket] = ptr;
	nTotalSOcket++;

	return TRUE;
}

// 소켓 정보 삭제
void RemoveSocketInfo(int iIndex)
{
	SOCKETINFO *ptr = SocketInfoArray[iIndex];

	// 클라 정보얻기
	SOCKADDR_IN RemClinetAddr;
	int addrlen = sizeof(RemClinetAddr);
	getpeername(ptr->sock, (SOCKADDR*)&RemClinetAddr, &addrlen);

	TCHAR TextBuff[30];
	InetNtop(AF_INET, &RemClinetAddr.sin_addr, TextBuff, sizeof(TextBuff));
	_tprintf(_T("\nclose clinetIP : %s, Port : %d\n\n"), TextBuff, ntohs(RemClinetAddr.sin_port));

	closesocket(ptr->sock);
	delete ptr;

	// 만약, iIndex가 마지막 소켓이 아니라면
	// 마지막 소켓을 사라진 현재 인덱스에 넣는다. 즉, 번호 채우기.
	if (iIndex != (nTotalSOcket - 1))
		SocketInfoArray[iIndex] = SocketInfoArray[nTotalSOcket - 1];

	nTotalSOcket--;

}
