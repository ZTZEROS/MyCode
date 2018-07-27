// Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <Ws2tcpip.h>

#define SERVERPORT 12347

struct st_PACKET_HEADER
{
	DWORD	dwPacketCode;		// 0x11223344	우리의 패킷확인 고정값

	WCHAR	szName[32];			// 본인이름, 유니코드 NULL 문자 끝
	WCHAR	szFileName[128];	// 파일이름, 유니코드 NULL 문자 끝
	int	iFileSize;
} sMyHeader;

int main()
{
	DWORD dCheck;

	// --------------------------------
	// 윈속 초기화
	// --------------------------------
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("WSAStartup fail\n");
		return 1;
	}

	// 소켓 생성 및 소켓 옵션 설정
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
	{
		int errorcode = WSAGetLastError();
		_tprintf(_T("socket() Error: %d\n"), errorcode);
		return 1;
	}

	int optval = 3000;
	dCheck = setsockopt(listen_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&optval, sizeof(optval));
	if (dCheck == SOCKET_ERROR)
	{
		int errorcode = WSAGetLastError();
		_tprintf(_T("setsockopt() Error: %d\n"), errorcode);
		return 1;
	}


	// binding
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));									// 메모리 초기화
	serverAddr.sin_family = AF_INET;											// ip버전 설정
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);								// 서버 ip 설정. 보통 INADDR_ANY(0.0.0.0) 사용. 해당 컴퓨터의 어떤 ip로와도 다 받겠다는 것.
	serverAddr.sin_port = htons(SERVERPORT);									// 포트 설정.
	dCheck = bind(listen_sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));		// 실제 바인딩
	if(dCheck == SOCKET_ERROR)
	{
		int errorcode = WSAGetLastError();
		_tprintf(_T("bind() Error: %d\n"), errorcode);
		return 1;
	}

	// 포트 상태 listen으로 변경
	dCheck = listen(listen_sock, SOMAXCONN);
	if (dCheck == SOCKET_ERROR)
	{
		int errorcode = WSAGetLastError();
		_tprintf(_T("listen() Error: %d\n"), errorcode);
		return 1;
	}	

	// 데이터 통신	
	SOCKADDR_IN clientAddr;
	int clinetAddrlen;
	TCHAR TextBuf[30];

	while (1)
	{
		SOCKET client_sock;		

		// accept()
		memset(&clientAddr, 0, sizeof(clientAddr));
		clinetAddrlen = sizeof(clientAddr);
		
		client_sock = accept(listen_sock, (SOCKADDR*)&clientAddr, &clinetAddrlen);
		if (client_sock == INVALID_SOCKET)
		{
			int errorcode = WSAGetLastError();
			_tprintf(_T("accept() Error: %d\n"), errorcode);
			return 1;
		}
		
		// 접속한 클라이언트 정보 출력
		_tprintf(_T("\nConnect Client IP : %s, Port Number : %d\n"), 
			InetNtop(AF_INET, &clientAddr.sin_addr, TextBuf, sizeof(TextBuf)), ntohs(clientAddr.sin_port));

		// 헤더 가져오기
		dCheck = recv(client_sock, (char*)&sMyHeader, sizeof(sMyHeader), 0);
		if (dCheck == SOCKET_ERROR)
		{
			int errorcode = WSAGetLastError();
			if (errorcode == 10060)
			{
				_tprintf(_T("TimeOut\n"));
				closesocket(client_sock);
				break;
			}

			_tprintf(_T("header recv() Error: %d\n"), errorcode);
			return 1;
		}

		else if (dCheck == 0)
			break;

		_tprintf(_T("header_Recv File: %d\n"), dCheck);

		// 헤더 정보를 기준으로 검증되지 않은 패킷 걸러내기.
		if (sMyHeader.dwPacketCode != 0x11223344)
		{
			_tprintf(_T("Hacker back!!\n"));
			break;
		}

		// 파일 크기를 기준으로 버퍼 동적할당.
		DWORD dSize = sMyHeader.iFileSize;
		BYTE* RecvBuff = new BYTE[dSize];
		BYTE* save = RecvBuff;

		while (1)
		{
			// 받아야 할 데이터가 1000 이상이라면				
			if (dSize > 1000)
			{
				dCheck = recv(client_sock, (char*)RecvBuff, 1000, 0);
				if (dCheck == SOCKET_ERROR)
				{
					int errorcode = WSAGetLastError();
					if (errorcode == 10060)
					{
						_tprintf(_T("TimeOut\n"));
						closesocket(client_sock);
						break;
					}

					_tprintf(_T("File recv() Error: %d\n"), errorcode);
					return 1;
				}

				_tprintf(_T("Recv File: %d\n"), dCheck);

				dSize -= dCheck;
				RecvBuff += dCheck;
			}

			else
			{
				dCheck = recv(client_sock, (char*)RecvBuff, dSize, 0);
				if (dCheck == SOCKET_ERROR)
				{
					int errorcode = WSAGetLastError();
					if (errorcode == 10060)
					{
						_tprintf(_T("TimeOut\n"));
						closesocket(client_sock);
						break;
					}
					_tprintf(_T("File recv() Error: %d\n"), errorcode);
					return 1;
				}

				_tprintf(_T("Recv File: %d\n"), dCheck);

				// RecvBuff위치 원래대로 이동
				RecvBuff = save;

				// 파일 생성
				FILE* fp;
				TCHAR FileName[161];
				_tcscpy_s(FileName, _countof(FileName), sMyHeader.szName);
				_tcscat_s(FileName, _countof(FileName), _T("_"));
				_tcscat_s(FileName, _countof(FileName), sMyHeader.szFileName);
				dCheck = _tfopen_s(&fp, FileName, _T("wb"));
				if (dCheck != 0)
				{
					_tprintf(_T("_tfopen_s fail\n"));
					return 1;
				}

				dCheck = fwrite(RecvBuff, 1, sMyHeader.iFileSize, fp);
				if (dCheck != sMyHeader.iFileSize)
				{
					_tprintf(_T("fwrite fail\n"));
					return 1;
				}
				fclose(fp);

				_tprintf(_T("File Copy Ok!\n"));

				break;
			}

		}

		closesocket(client_sock);
	}

	closesocket(listen_sock);
	WSACleanup();

    return 0;
}

