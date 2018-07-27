// StartMove_Clinet.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib,"ws2_32")
#include <locale.h>
#include <WS2tcpip.h>
#include "Profiling.h"

// Profiling_Class.h가 선언되어 있지 않다면, 아래 매크로들을 공백으로 만듦. 
#ifndef __PROFILING_CLASS_H__
#define BEGIN(STR) 
#define END(STR)
#define FREQUENCY_SET()
#define PROFILING_SHOW()
#define PROFILING_FILE_SAVE()
#define RESET()
#else
#define BEGIN(STR)				BEGIN(STR)
#define END(STR)				END(STR)
#define FREQUENCY_SET()			FREQUENCY_SET()
#define PROFILING_SHOW()		PROFILING_SHOW()
#define PROFILING_FILE_SAVE()	PROFILING_FILE_SAVE()
#define RESET()					RESET()

#endif // !__PROFILING_CLASS_H__


#define SERVERPORT 3000
#define HEIGHT	23
#define WIDTH	81

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
};

// 전역 변수
int dMyID = 0;						// 내 ID
BOOL bMyCheck = FALSE;				// 내 아이디를 얻었는지 체크
int dPlayerCount = 0;				// 현재 플레이어 수 카운트
S_Player* s_PlayerList[64];			// 플레이어 구조체 리스트
char cBackBuffer[HEIGHT][WIDTH];	// 버퍼
HANDLE hConsole;

//콘솔 제어 함수
void cs_Initial(void);

// 콘솔 화면의 커서를 x,y 좌표로 이동
void cs_MoveCursor(int iPosy, int iPosx);

// 프로세스 함수
BOOL KeyProcess(SOCKET* sock);
BOOL NetworkProcess(SOCKET* sock);
void RenderingProcess();

// 패킷 처리 함수
void CreateStar(Packet* packet);
void Disconnect(Packet* packet);
void Move(Packet* packet);

int main()
{	
	setlocale(LC_ALL, "korean");
	FREQUENCY_SET(); // 주파수 구하기	

	// IP 입력받기
	TCHAR tAddrText[30] = _T("192.168.10.16");
	UINT size = sizeof(tAddrText);
	/*_tprintf(_T("IP 입력 : "));
	_tscanf_s(_T("%s"), tAddrText, size);*/

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		_tprintf(_T("WSAStartup() 에러!\n"));
		return 1;
	}

	// 클라이언트 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		_tprintf(_T("sock 소켓 생성 에러!\n"));
		return 1;
	}	

	// connect
	SOCKADDR_IN clinetaddr;
	ZeroMemory(&clinetaddr, sizeof(clinetaddr));
	clinetaddr.sin_family = AF_INET;
	clinetaddr.sin_port = htons(SERVERPORT);
	InetPton(AF_INET, tAddrText, &clinetaddr.sin_addr);
	DWORD dCheck = connect(sock, (SOCKADDR*)&clinetaddr, sizeof(clinetaddr));
	if (dCheck == SOCKET_ERROR)
	{
		_tprintf(_T("connect 에러!\n"));
		return 1;
	}
	else if (dCheck == EWOULDBLOCK)
	{
		_tprintf(_T("connect EWOULDBLOCK!\n"));
		return 1;
	}

	// 논블락 소켓으로 변경
	ULONG on = 1;
	dCheck = ioctlsocket(sock, FIONBIO, &on);
	if (dCheck == SOCKET_ERROR)
	{
		_tprintf(_T("논블락 소켓 변화 실패!!\n"));
		return 1;
	}

	cs_Initial();
	system("cls");
	
	// 인게임 처리
	while (1)
	{

		// 나의 키보드 입력
		BEGIN("KeyProcess");
		dCheck = KeyProcess(&sock);
		END("KeyProcess");
		if (dCheck == FALSE)
			break;		

		// 네트워크
		BEGIN("NetworkProcess");
		dCheck = NetworkProcess(&sock);
		END("NetworkProcess");
		if (dCheck == FALSE)
			break;		

		// 랜더링	
		BEGIN("RenderingProcess");
		RenderingProcess();	
		END("RenderingProcess");
	}	

	// 소켓 제거
	closesocket(sock);

	// 윈속 제거
	WSACleanup();	
    return 0;
}

BOOL KeyProcess(SOCKET* sock)
{
	Packet s_sendPacket;

	if (bMyCheck == TRUE)
	{
		int x, y, i;
		for (i = 0; i < dPlayerCount; ++i)
		{
			if (s_PlayerList[i]->m_ID == dMyID)
			{
				x = s_PlayerList[i]->m_Xpos;
				y = s_PlayerList[i]->m_Ypos;
				break;
			}
		}

		// 상
		if (GetAsyncKeyState(VK_UP) & 0x8000)
			y--;

		// 하
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			y++;

		// 좌
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
			x--;

		// 우
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
			x++;

		else if (GetAsyncKeyState(0x31) & 0x8000)
			PROFILING_FILE_SAVE(); // 숫자키 1을 누르면 프로파일링 파일 저장

		// x,y좌표가 달라졌다면. 즉, 키를 입력했다면 내 위치 이동 / 패킷 제작 후 send()
		if (s_PlayerList[i]->m_Xpos != x || s_PlayerList[i]->m_Ypos != y)
		{
			// 현재 위치가, 높이와 넓이를 벗어났는지 체크한다
			if (x >= 0 && x < WIDTH-1 && y >=0 && y < HEIGHT)
			{				
				// 내 현재 위치 갱신
				s_PlayerList[i]->m_Xpos = x;
				s_PlayerList[i]->m_Ypos = y;	

				cBackBuffer[s_PlayerList[i]->m_Ypos][s_PlayerList[i]->m_Xpos] = '*';

				// 패킷 제작
				s_sendPacket.m_ID = dMyID;
				s_sendPacket.m_Type = MOVE;
				s_sendPacket.m_Xpos = x;
				s_sendPacket.m_Ypos = y;

				// 패킷 send
				DWORD dCheck = send(*sock, (CHAR*)&s_sendPacket, sizeof(s_sendPacket), 0);
				if (dCheck == 0 || dCheck == SOCKET_ERROR)
				{
					_tprintf(_T("send 에러!\n"));
					return FALSE;
				}
				else if (dCheck == EWOULDBLOCK)
				{
					_tprintf(_T("send EWOULDBLOCK!\n"));
					return 1;
				}
			}
		}
	}
	return TRUE;
}

BOOL NetworkProcess(SOCKET* sock)
{
	DWORD dCheck;
	Packet s_revPacket;

	// Select 준비
	FD_SET rset;
	FD_ZERO(&rset);
	FD_SET(*sock, &rset);
	TIMEVAL tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	// Select()
	while (1)
	{
		// dCheck의 값이 0이 될 때 까지 select를 한다.
		dCheck = select(0, &rset, 0, 0, &tval);
		if (dCheck == SOCKET_ERROR)
		{
			_tprintf(_T("select 에러!\n"));
			return FALSE;
		}
		// dCHeck의 값이 0이면 리시브할게 없는거니 while문 종료
		else if (dCheck == 0)
			break;

		// recv()
		dCheck = recv(*sock, (CHAR*)&s_revPacket, 16, 0);
		if (dCheck == 0 || dCheck == SOCKET_ERROR)
		{
			_tprintf(_T("recv 에러!\n"));
			return FALSE;
		}
		else if (dCheck == EWOULDBLOCK)
		{
			_tprintf(_T("recv EWOULDBLOCK!\n"));
			return 1;
		}

		// 패킷 처리
		switch (s_revPacket.m_Type)
		{
			// 아이디 할당 
		case ID_GET:
			dMyID = s_revPacket.m_ID;			
			break;

			// 별 생성 
		case NEW_CONNECT:
			CreateStar(&s_revPacket);
			bMyCheck = TRUE;
			break;

			// 별 삭제(접속 종료)
		case DISCONNECT:
			Disconnect(&s_revPacket);
			break;

			// 이동
		case MOVE:
			Move(&s_revPacket);
			break;

		default:
			_tprintf(_T("이상한 패킷을 받았습니다!\n"));
			return FALSE;
		}
	}	

	return TRUE;
}

void RenderingProcess()
{
	int i;	
	
	system("cls");	
	for (i = 0; i < dPlayerCount; ++i)
	{	
		// 위치에 별 찍기		
		cs_MoveCursor(s_PlayerList[i]->m_Ypos, s_PlayerList[i]->m_Xpos);
		printf("%c", cBackBuffer[s_PlayerList[i]->m_Ypos][s_PlayerList[i]->m_Xpos]);
	}	
	
	Sleep(20);
}

// 접속 처리 함수 (별 생성)
void CreateStar(Packet* packet)
{
	S_Player* newPlayer = new S_Player;

	newPlayer->m_ID = packet->m_ID;
	newPlayer->m_Xpos = packet->m_Xpos;
	newPlayer->m_Ypos = packet->m_Ypos;

	s_PlayerList[dPlayerCount++] = newPlayer;

	cBackBuffer[newPlayer->m_Ypos][newPlayer->m_Xpos] = '*';
}

// 접속 종료 함수 (별 제거)
void Disconnect(Packet* packet)
{
	int i;
	for (i = 0; i < dPlayerCount; ++i)
		if (s_PlayerList[i]->m_ID == packet->m_ID)
			break;

	S_Player* newPlayer = s_PlayerList[i];
	delete newPlayer;

	if (i != (dPlayerCount - 1))
		s_PlayerList[i] = s_PlayerList[dPlayerCount - 1];

	dPlayerCount--;
}

// 이동 함수 (이동 패킷)
void Move(Packet* packet)
{
	// 이동시킬 플레이어 찾아내기
	int i, x, y;
	for (i = 0; i < dPlayerCount; ++i)
		if (s_PlayerList[i]->m_ID == packet->m_ID)
			break;

	// 플레이어를 찾은 후, x,y좌표 갱신
	s_PlayerList[i]->m_Xpos = packet->m_Xpos;
	s_PlayerList[i]->m_Ypos = packet->m_Ypos;
	cBackBuffer[s_PlayerList[i]->m_Ypos][s_PlayerList[i]->m_Xpos] = '*';
}

//콘솔 제어 함수
void cs_Initial(void)
{
	CONSOLE_CURSOR_INFO stConsoleCursor;
	// 화면의 커서 안보이게 설정
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1; // 커서 크기

	// 콘솔화면 핸들을 구한다.
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

// 콘솔 화면의 커서를 x,y 좌표로 이동
void cs_MoveCursor(int iPosy, int iPosx)
{
	COORD stCoord;
	stCoord.X = iPosx;
	stCoord.Y = iPosy;
	// 원하는 위치로 커서 이동
	SetConsoleCursorPosition(hConsole, stCoord);
}

