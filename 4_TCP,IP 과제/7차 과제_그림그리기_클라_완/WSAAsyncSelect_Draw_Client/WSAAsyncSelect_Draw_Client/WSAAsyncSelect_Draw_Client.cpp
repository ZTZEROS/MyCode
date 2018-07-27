// WSAAsyncSelect_Draw_Client.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "WSAAsyncSelect_Draw_Client.h"
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32")
#include "RingBuff.h"
#pragma warning(disable:4996)

#define SERVERPORT 25000
#define WM_SOCK		WM_USER+1


// 페이로드
#pragma pack(push, 1)
struct st_Draw_Packet
{
	int iStartX;
	int iStartY;
	int iEndX;
	int iEndY;
};
#pragma pack(pop)

// 전역 변수:
HINSTANCE hInst;                // 현재 인스턴스입니다.
HWND g_hWnd;					// 메인 윈도우 입니다.
TCHAR g_tIP[30];				// 입력한 서버의 IP
SOCKET client_sock;				// 클라 소켓
BOOL bNowCheck;					// 그리기 가능 여부를 판단하기 위한 것
BOOL bConnectFlag = TRUE;		// 접속 여부 체크. TRUE면 접속 됨.
BOOL bSendFlag = FALSE;			// 샌드 가능 여부. TRUE면 가능.
int x, y, Oldx, Oldy;			// 그리기 좌표
CRingBuff SendBuff;				// 리시브 링버퍼
CRingBuff RecvBuff;				// 리시브 링버퍼


// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR  CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
int ProcRead();						// 데이터 받기
int ProcWrite();						// 데이터 보내기
void ErrorTextOut(const TCHAR*);		// 에러를 찍는 함수
BOOL ProcPacket();						// 리시브 안의 데이터를 처리한다. 샌드 링버퍼에 넣어야할지 아니면 다른걸해야할지 등..
void Draw(st_Draw_Packet);							// 화면에 그림 그리기 함수
int SendPacket(char* Buff, int size);				// Buff의 데이터를 샌드 버퍼에 넣기


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    // TODO: 여기에 코드를 입력합니다.	

	// IP 입력받는 다이얼로그 시작.
	INT_PTR EndCode = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDR), NULL, DialogProc);
	if (EndCode == IDCANCEL)	// 정상 종료.
		return 0;
	

    // 전역 문자열을 초기화합니다.
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WSAASYNCSELECTDRAWCLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WSAASYNCSELECTDRAWCLIENT);
	wcex.lpszClassName = _T("DrawClinet");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

    // 응용 프로그램 초기화를 수행합니다.
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	g_hWnd = CreateWindowW(_T("DrawClinet"), _T("다함께 그림그리기"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!g_hWnd)
		return FALSE;	

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);	

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
	{
		MessageBox(NULL, _T("윈속 초기화 실패"), _T("Error"), MB_ICONERROR);
		return 0;
	}

	// socket()
	client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_sock == INVALID_SOCKET)
	{
		MessageBox(NULL, _T("socket() 실패"), _T("Error"), MB_ICONERROR);
		return 0;
	}

	// connect()
	ULONG uIP;
	SOCKADDR_IN clientaddr;
	ZeroMemory(&clientaddr, sizeof(clientaddr));

	InetPton(AF_INET, g_tIP, &uIP);
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = uIP;
	clientaddr.sin_port = htons(SERVERPORT);
	int retval = connect(client_sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));
	if (retval == SOCKET_ERROR)
	{
		MessageBox(NULL, _T("connect() 실패"), _T("Error"), MB_ICONERROR);
		return TRUE;
	}

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(client_sock, g_hWnd, WM_SOCK, FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT);
	if (retval == SOCKET_ERROR)
	{
		MessageBox(NULL, _T("WSAAsyncSelect() 실패"), _T("Error"), MB_ICONERROR);
		return TRUE;
	}

    MSG msg;

    // 기본 메시지 루프입니다.
    while (GetMessage(&msg, nullptr, 0, 0))
    {       
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	closesocket(client_sock);
	WSACleanup();

    return (int) msg.wParam;
	
}

// 윈 프록
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다.
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    
	case WM_SOCK:
		// 에러 먼저 체크
		if (WSAGETSELECTERROR(lParam))
		{
			ErrorTextOut(_T("WM_SOCK 에러 발생"));
			break;
		}

		// 소켓 처리
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
		{			
			int check = ProcRead();
			if (check == -1)
				PostMessage(hWnd, WM_COMMAND, wParam, IDM_EXIT);
			else
				ProcPacket();
			break;
		}

		case FD_WRITE:
			bSendFlag = TRUE;
			break;

		case FD_CONNECT:
			bConnectFlag = TRUE;
			break;

		case FD_CLOSE:
			ErrorTextOut(_T("서버 접속 종료"));
			PostMessage(hWnd, WM_COMMAND, wParam, IDM_EXIT);
			break;

		default:
			ErrorTextOut(_T("알수 없는 패킷"));
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		bNowCheck = TRUE;
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		break;

	case WM_LBUTTONUP:
		bNowCheck = FALSE;
		break;

	case WM_MOUSEMOVE:
		if (bNowCheck)
		{
			Oldx = LOWORD(lParam);
			Oldy = HIWORD(lParam);

			// 헤더, 페이로드 만들기
			st_Draw_Packet Payload;
			Payload.iStartX = x;
			Payload.iStartY = y;
			Payload.iEndX = Oldx;
			Payload.iEndY = Oldy;
			unsigned short Header = sizeof(Payload);

			// 만든 패킷을 샌드 버퍼에 넣기
			SendPacket((char*)&Header, 2);
			SendPacket((char*)&Payload, Header);

			// x,y 값 갱신
			x = Oldx;
			y = Oldy;

			// 샌드 버퍼의 데이터를 보내기
			int check = ProcWrite();
			if(check == -1)
				PostMessage(hWnd, WM_COMMAND, wParam, IDM_EXIT);
			
		}
		break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// IP 입력받는 다이얼로그 프록
INT_PTR  CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		ZeroMemory(g_tIP, sizeof(g_tIP));
		SetDlgItemText(hWnd, IDC_EDIT1, _T("127.0.0.1"));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		{	// 에디터 컨트롤에서 입력한 IP 얻기
			GetDlgItemText(hWnd, IDC_EDIT1, g_tIP, sizeof(g_tIP));			

			// 다이얼로그 종료. 다음 다이얼로그를 열어야하니까!
			EndDialog(hWnd, IDOK);
			return TRUE;
		}
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}

		return FALSE;
	}
	return FALSE;

}

// 데이터 받기
int ProcRead()
{
	// 만약, 아직 연결이 안됐는데 받으려고 하면 바로 리턴.
	if (bConnectFlag == FALSE)
	{
		ErrorTextOut(_T("아직 연결되지 않음."));
		return 0;
	}

	// recv()
	while (1)
	{
		// 연결이 됐다면 임시버퍼로 리시브
		char Buff[1000];
		int iRealrecvSize = recv(client_sock, Buff, sizeof(Buff), 0);

		// 에러가 발생했다면 에러처리
		if (iRealrecvSize == SOCKET_ERROR)
		{
			// WSAEWOULDBLOCK에러가 발생하면, 소켓 버퍼가 비어있다는 것. recv할게 없다는 것이니 while문 종료
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				break;

			// WSAEWOULDBLOCK에러가 아니면 뭔가 이상한 것이니 접속 종료
			else
			{
				ErrorTextOut(_T("ProcRead(). 뭔가 이상해서 종료"));
				return -1;
			}
		}

		// 값이 0이면 상대가 연결을 종료한 것.
		if (iRealrecvSize == 0)
		{
			ErrorTextOut(_T("ProcRead(). 연결 종료"));
			return -1;
		}

		int BuffArray = 0;
		// 임시 버퍼의 값을 리시브 링버퍼로 넣기.
		while (iRealrecvSize > 0)
		{			
			int EnqueueSize = RecvBuff.Enqueue(&Buff[BuffArray], iRealrecvSize);
			if (EnqueueSize == -1)
			{
				ErrorTextOut(_T("ProcRead(). 인큐 꽉참."));
				return -1;
			}

			iRealrecvSize -= EnqueueSize;
			BuffArray += EnqueueSize;
		}		
	}
		
	return 1;
	
}

// 데이터 보내기
int ProcWrite()
{
	// 샌드 할 수 있는 상태가 되었는지 체크
	if (bSendFlag == FALSE)
		return 0;

	// 샌드 큐의 데이터를 모두 꺼내서 send 한다.
	int SendQueueSize = SendBuff.GetUseSize();
	int BuffArray = 0;

	if(SendQueueSize >= 1000)
		ErrorTextOut(_T("ProcWrite(). 디버깅 테스트"));

	while (SendQueueSize > 0)
	{
		char Buff[1000];
		
		// Peek으로 Buff에 데이터를 꺼내온다.
		int size = SendBuff.Peek(&Buff[BuffArray], SendQueueSize);
		if(size >= 1000)
			ErrorTextOut(_T("ProcWrite(). 디버깅 테스트"));

		// 샌드 큐가 비었으면 다 꺼낸거니 정상적으로 리턴
		if (size == -1)
			break;

		// 뭔가 꺼낸게 있으면 send
		int sendSize = send(client_sock, Buff, size, 0);

		if(sendSize > 18)
			ErrorTextOut(_T("ProcWrite(). 디버깅 테스트"));

		// send 에러처리
		if (sendSize == SOCKET_ERROR)
		{
			// 우드블럭인지 체크.우드블럭이라면, Send불가능 상태가 된 것이니(내 소켓 버퍼나 상대 소켓 버퍼가 가득 찬 상황 ? )
			// 샌드 플레그를 FASLE로 만들고 나간다.
			if (sendSize == WSAEWOULDBLOCK)
			{
				ErrorTextOut(_T("ProcWrite(). 샌드 우드블럭"));
				bSendFlag = FALSE;
				return 0;
			}

			// 그 외 에러는 그냥 종료처리
			else
				return -1;
		}

		// 샌드에 성공했으면 성공한 사이즈 만큼 Remove한다
		SendBuff.RemoveData(sendSize);

		SendQueueSize -= sendSize;
		BuffArray += sendSize;
	}

	return 0;
}

// 텍스트 아웃으로 에러 찍는 함수
void ErrorTextOut(const TCHAR* ErrorLog)
{
	TCHAR Log[30];
	_tcscpy(Log, ErrorLog);
	HDC hdc = GetDC(g_hWnd);
	TextOut(hdc, 0, 0, ErrorLog, _tcslen(Log));
}

// 리시브 안의 데이터를 처리한다. 지금은 무조건 그림을 그린다.
BOOL ProcPacket()
{
	// 리시브 큐의 가장 앞 헤더 확인. 헤더에는 페이로드의 사이즈가 들어있다.
	unsigned short header;
	int Check = RecvBuff.Peek((char*)&header, 2);
	if (Check == -1)
	{
		ErrorTextOut(_T("리시브 큐 비었음"));
		return FALSE;
	}

	while (1)
	{
		// 패킷 1개가 완성될 수 있으면 그 때, 패킷 처리
		if (RecvBuff.GetUseSize() >= header + 2)
		{
			int DequeueSize;
			int BuffSize = 2;
			int BuffArray = 0;

			// 2바이트 헤더를 임시버퍼로 꺼내옴
			while (BuffSize > 0)
			{				
				DequeueSize = RecvBuff.Dequeue((char*)&header + BuffArray, BuffSize);
				if (DequeueSize == -1)
				{
					ErrorTextOut(_T("리시브 큐 비었음"));
					return FALSE;
				}

				BuffSize -= DequeueSize;
				BuffArray += DequeueSize;
			}

			if(header != 16)
				ErrorTextOut(_T("테스트 에러처리"));
			

			// 헤더에 들어있는 값(페이로드의 길이) 만큼 꺼내옴
			st_Draw_Packet DrawInfo;
			BuffSize = header;
			BuffArray = 0;

			while (BuffSize > 0)
			{
				DequeueSize = RecvBuff.Dequeue((char*)&DrawInfo + BuffArray, BuffSize);
				if (DequeueSize == -1)
				{
					ErrorTextOut(_T("리시브 큐 비었음"));
					return FALSE;
				}

				BuffSize -= DequeueSize;
				BuffArray += DequeueSize;
			}
			

			// 그리기 함수 호출 << 현재의 실질적인 패킷처리
			Draw(DrawInfo);
		}

		// 패킷 1개가 안되면 나중에 다시 처리.
		else
			break;

	}
	

	return TRUE;
}

// 화면에 그림 그리기 함수
void Draw(st_Draw_Packet DrawInfo)
{
	HDC hdc = GetDC(g_hWnd);

	MoveToEx(hdc, DrawInfo.iStartX, DrawInfo.iStartY, NULL);
	LineTo(hdc, DrawInfo.iEndX, DrawInfo.iEndY);
	
	ReleaseDC(g_hWnd, hdc);
}

// Buff의 데이터를 샌드 버퍼에 넣기
int SendPacket(char* Buff, int iSize)
{
	// Buff의 데이터를 샌드 큐로 이동시킨다.
	// Buff가 텅 빌때까지.
	int BuffArray = 0;

	while (iSize > 0)
	{
		// Buff에서 iSize만큼 샌드 버퍼로 인큐한다.
		int realEnqeueuSize = SendBuff.Enqueue(&Buff[BuffArray], iSize);

		// Enqueue() 함수는, 버퍼가 가득 찼으면 -1을 리턴한다.
		// 버퍼가 가득찰 정도면 뭔가 이상하니 그냥 접속 종료
		if (realEnqeueuSize == -1)
		{
			ErrorTextOut(_T("SendPacket(). 내 샌드 버퍼 가득참"));
			return -1;
		}
		iSize -= realEnqeueuSize;
		BuffArray += realEnqeueuSize;
	}

	return 0;
}