// StarMove_Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>
#include <locale>

#define SERVERPORT 3000
#define HEIGHT	23
#define WIDTH	80

// 패킷 타입
enum PacketType
{
	ID_GET = 0, NEW_CONNECT, DISCONNECT, MOVE
};

// 패킷
#pragma pack(push, 1)
struct Packet
{
	int m_Type;
	int m_ID;
	int m_Xpos;
	int m_Ypos;
};
#pragma pack(pop)

// PlayerStruct
struct S_Player
{
	int m_ID;
	int m_Xpos;
	int m_Ypos;
	SOCKET sock;
};

// 전역 변수
int iUserID = 0;
int iUserCount = 0;
S_Player* s_PlayerList[64];			// 플레이어 구조체 리스트
HANDLE hConsole;

// 프로세스 함수
BOOL NetworkProcess(SOCKET* listen_sock);

// 패킷 처리 함수
BOOL CreateStar(SOCKET* client_sock);
void Disconnect(int ListIndex);
void Move(int ListIndex, SOCKET* client_sock, Packet* pack);
BOOL SendUnicast(SOCKET* client_sock, Packet* pack);
BOOL SendBroadcast(SOCKET* client_sock, Packet* pack);


int main()
{
	setlocale(LC_ALL, "korean");

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		_tprintf(_T("WSAStartup 실패!\n"));
		return 1;
	}

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
	{
		_tprintf(_T("listen_sock / socket() 실패!\n"));
		return 1;
	}

	// 바인딩
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	DWORD dCheck = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (dCheck == SOCKET_ERROR)
	{
		DWORD error = WSAGetLastError();
		_tprintf(_T("bind() 에러 : %d\n"), error);
		return 1;
	}	

	// 리슨 상태
	dCheck = listen(listen_sock, SOMAXCONN);
	if (dCheck == SOCKET_ERROR)
	{
		_tprintf(_T("listen() 에러\n"));
		return 1;
	}

	// 논블로킹 소켓으로 변경
	ULONG on = 1;
	dCheck = ioctlsocket(listen_sock, FIONBIO, &on);
	if (dCheck == SOCKET_ERROR)
	{
		_tprintf(_T("논블로킹 소켓 변경 에러\n"));
		return 1;
	}

	printf("Connect Clinet : %d\n", iUserCount);
	// 클라와 통신
	while (1)
	{
		// 네트워크
		dCheck = NetworkProcess(&listen_sock);
		if (dCheck == FALSE)
			break;
	}

	WSACleanup();
	closesocket(listen_sock);

	_tprintf(_T("정상 종료"));
    return 0;
}

BOOL NetworkProcess(SOCKET* listen_sock)
{
	// 클라와 통신에 사용할 변수
	static FD_SET rset;
	SOCKET client_sock;
	SOCKADDR_IN clinetaddr;

	// select 준비	
	FD_ZERO(&rset);
	FD_SET(*listen_sock, &rset);
	for (int i = 0; i < iUserCount; ++i)
		FD_SET(s_PlayerList[i]->sock, &rset);

	TIMEVAL tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	// Select()
	DWORD dCheck = select(0, &rset, 0, 0, &tval);
	if (dCheck == SOCKET_ERROR)
	{
		_tprintf(_T("select 에러\n"));
		return FALSE;
	}
	// select의 값이 0보다 크다면 뭔가 할게 있다는 것이니 로직 진행
	else if (dCheck > 0)
	{
		// 리슨 소켓 존재하는지 검사
		if (FD_ISSET(*listen_sock, &rset))
		{
			int addrlen = sizeof(clinetaddr);
			client_sock = accept(*listen_sock, (SOCKADDR*)&clinetaddr, &addrlen);

			// 에러가 발생하면, 소켓 등록 안함
			if (client_sock == INVALID_SOCKET)
				_tprintf(_T("accept 에러\n"));

			// 최대 받을 수 있는 클라 수는 63명(남은 1개는 리슨소켓). 그 이상 접속을 시도하면 에러처리
			else if (iUserCount > FD_SETSIZE-2)
			{
				_tprintf(_T("63명 이상 접속 불가\n"));
				closesocket(client_sock);
			}
			// 에러가 발생하지 않았다면, 별 생성절차 진행 (ID발급, x/y좌표 지정, 소켓 리스트에 등록, 플레이어 리스트에 등록 등..)
			else
			{
				dCheck = CreateStar(&client_sock);
				if (dCheck == FALSE)
					return FALSE;
			}

		}

		// 리슨소켓 외 소켓 처리
		for (int i = 0; i < iUserCount; ++i)
		{
			if (FD_ISSET(s_PlayerList[i]->sock, &rset))
			{
				Packet s_Packet;
				dCheck = recv(s_PlayerList[i]->sock, (CHAR*)&s_Packet, sizeof(s_Packet), 0);
				if (dCheck == SOCKET_ERROR || dCheck == 0)
				{					
					Disconnect(i);
					_tprintf(_T("접속종료. Connect Clinet : %d\n"), iUserCount);
				}

				else if (dCheck != EWOULDBLOCK)
				{
					switch (s_Packet.m_Type)
					{
					case MOVE:
						if (s_Packet.m_Xpos < WIDTH || s_Packet.m_Xpos > 0 || s_Packet.m_Ypos < HEIGHT || s_Packet.m_Ypos > 0)
							Move(i, &s_PlayerList[i]->sock, &s_Packet);

						break;

					default:
						_tprintf(_T("이상한 패킷을 받았습니다.\n"));
						return FALSE;
					}
				}
			}
		}
	}

	return TRUE;
}

// 별 생성 처리
BOOL CreateStar(SOCKET* client_sock)
{
	// 클라이언트 생성
	S_Player* newPlayer = new S_Player;
	newPlayer->m_ID = iUserID;
	newPlayer->m_Xpos = 40;
	newPlayer->m_Ypos = 12;
	newPlayer->sock = *client_sock;
	s_PlayerList[iUserCount] = newPlayer;

	// ID 할당 패킷 제작 후, Unicast send()
	Packet s_Packet;
	s_Packet.m_Type = ID_GET;
	s_Packet.m_ID = newPlayer->m_ID;
	s_Packet.m_Xpos = s_PlayerList[iUserCount]->m_Xpos;
	s_Packet.m_Ypos = s_PlayerList[iUserCount]->m_Ypos;
	DWORD dCheck = SendUnicast(client_sock, &s_Packet);
	if(dCheck == FALSE)
		return FALSE;

	iUserID++;
	iUserCount++;

	// 지금 막 접속한 유저의 별 생성 패킷 제작 후, 그 유저에게 Unicast send()
	s_Packet.m_Type = NEW_CONNECT;
	dCheck = SendUnicast(client_sock, &s_Packet);
	if (dCheck == FALSE)
		return FALSE;

	// 별 생성 패킷을, 이제 막 접속한 유저를 제외한 모든 접속중이던 유저에게 Broadcast send()
	// 즉, 특정 유저를 제외한 BroadCast
	dCheck = SendBroadcast(client_sock, &s_Packet);
	if (dCheck == FALSE)
		return FALSE;

	// 별 성생 패킷을, 지금 막 접속한 유저에게, 자신을 제외한 모든 유저의 별을 생성하라고 전달. Unicast send()
	// 즉, 특정 정보를 제외한 Unicast
	for (int i = 0; i < iUserCount; ++i)
	{
		if (s_PlayerList[i]->m_ID != newPlayer->m_ID)	// newPlayer->m_IDd에는 지금 막 생성된 유저의 정보가 들어있다. 즉, 
		{
			// 패킷 제작
			s_Packet.m_ID = s_PlayerList[i]->m_ID;
			s_Packet.m_Xpos = s_PlayerList[i]->m_Xpos;
			s_Packet.m_Ypos = s_PlayerList[i]->m_Ypos;

			// 패킷 send
			dCheck = SendUnicast(client_sock, &s_Packet);
			if (dCheck == FALSE)
				return FALSE;
		}
	}	

	printf("Connect Clinet : %d\n", iUserCount);
	return TRUE;
}

// 단일 send
BOOL SendUnicast(SOCKET* client_sock, Packet* pack)
{
	DWORD dCheck = send(*client_sock, (CHAR*)pack, sizeof(Packet), 0);
	if (dCheck == SOCKET_ERROR)
	{
		_tprintf(_T("SendUnicast() 에러"));
		return FALSE;
	}	

	return TRUE;
}

// 브로드캐스트 send
BOOL SendBroadcast(SOCKET* client_sock, Packet* pack)
{

	for (int i = 0; i < iUserCount; ++i)
	{
		if (s_PlayerList[i]->sock != *client_sock)
		{
			DWORD dCheck = send(s_PlayerList[i]->sock, (CHAR*)pack, sizeof(Packet), 0);
			if (dCheck == SOCKET_ERROR)
				if(WSAGetLastError() != 10054)
				_tprintf(_T("SendBroadcast() 에러. i값 : %d\n"), i);
		}
	}
	
	return TRUE;
}

// Move 패킷처리
void Move(int ListIndex, SOCKET* client_sock, Packet* pack)
{
	// 해당 ID의 유저, x/y위치 이동
	if (pack->m_Xpos >= WIDTH)
		pack->m_Xpos = WIDTH - 1;
	else if(pack->m_Xpos < 0)
		pack->m_Xpos = 0;

	if (pack->m_Ypos >= HEIGHT)
		pack->m_Ypos = HEIGHT - 1;
	else if (pack->m_Ypos < 0)
		pack->m_Ypos = 0;

	s_PlayerList[ListIndex]->m_Xpos = pack->m_Xpos;
	s_PlayerList[ListIndex]->m_Ypos = pack->m_Ypos;

	// 그리고, 해당 ID 유저를 제외한 모두에게 이동 패킷 전달(Boradcast)
	SendBroadcast(client_sock, pack);
}

// 접속 종료 처리
void Disconnect(int ListIndex)
{
	// 지정된 ID의 유저를 접속종료 시킨 후, 접속 종료한 유저 제외 모든 유저에게 DISCONNECT 패킷 발송
	// 접속 종료할 유저를 일단 뽑아낸다.
	S_Player* DisconnectPlayer = s_PlayerList[ListIndex];

	// DISCONNECT 패킷 제작
	Packet s_DisconnectPack;
	s_DisconnectPack.m_Type = DISCONNECT;
	s_DisconnectPack.m_ID = DisconnectPlayer->m_ID;
	s_DisconnectPack.m_Xpos = 0;
	s_DisconnectPack.m_Ypos = 0;	
	
	// 리스트에 있는 모든 유저에게 접속종료 패킷 발송 (접속 종료를 보낸 유저 제외)
	SendBroadcast(&DisconnectPlayer->sock, &s_DisconnectPack);

	// 리스트에서 제외	
	if (ListIndex != (iUserCount - 1))
		s_PlayerList[ListIndex] = s_PlayerList[iUserCount - 1];

	// 유저 수 감소
	iUserCount--;

	// 클로즈 소켓으로 연결 끊기.
	closesocket(DisconnectPlayer->sock);

	// 메모리 반환
	delete DisconnectPlayer;
}

