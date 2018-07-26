// 일반 클라.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>
#include <clocale>
#include <conio.h>


#define SERVERIP	L"127.0.0.1"
#define SERVERPORT	9000
#define BUF_SIZE	512

void err_quit(const TCHAR* msg)
{
	LPVOID	lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const TCHAR* msg)
{
	LPVOID	lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	_tprintf(L"[%s] %s\n", msg, (TCHAR*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0)
	{
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;

		left -= received;
		ptr += received;
	}

	return (len - left);
}



int _tmain()
{
	system("mode con: cols=100 lines=20");

	int retval;

	_tsetlocale(LC_ALL, L"korean");

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
		err_quit(L"socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);
	InetPton(AF_INET, SERVERIP, &serveraddr.sin_addr.s_addr);

	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit(L"Connect()");


	char buf[BUF_SIZE + 1];
	int len;

	while (1)
	{

		fputs("[보낼 데이터] : ", stdout);
		if (fgets(buf, BUF_SIZE + 1, stdin) == NULL)
			break;		

		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';

		if (len == 0)
			break;

		retval = send(sock, buf, strlen(buf), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display(L"send()");
			break;
		}
		_tprintf(L"[TCP 클라] %d 바이트 보냄.\n", retval);

		retval = recvn(sock, buf, retval, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display(L"recv()");
			break;
		}

		else if (retval == 0)
		{
			const char* ExitText = "Thanks";
			retval = send(sock, ExitText, strlen(ExitText), 0);

			if (retval == SOCKET_ERROR)
				err_display(L"send()");

			_tprintf(L"\n[TCP 클라] shutdown받음.\n[보낸 데이터] Thanks, %d 바이트 보냄.\n", retval);

			break;
		}

		// 받은 데이터 출력
		buf[retval] = '\0';
		_tprintf(L"[TCP 클라] %d 바이트 받음.\n[받은 데이터] : ", retval);
		printf("%s\n\n", buf);
	}

	closesocket(sock);

	WSACleanup();

	printf("\nExit!!!!\n");
	_getch();
    return 0;
}

