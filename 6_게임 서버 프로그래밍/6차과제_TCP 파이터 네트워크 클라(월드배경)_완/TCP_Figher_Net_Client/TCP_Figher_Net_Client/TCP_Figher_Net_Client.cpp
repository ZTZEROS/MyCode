// Project1.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "TCP_Figher_Net_Client.h"

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "imm32.lib")

#include "windowsx.h"
#include "PlayerObject.h"
#include "ScreenDib.h"
#include "SpriteDib.h"
#include "Map.h"

#include "Network_Func.h"

#define WINDOW_WIDTH			640
#define WINDOW_HEIGHT			480

#define MAPSIZE_HEIGHT_PIXEl	6400
#define MAPSIZE_WIDTH_PIXEl		6400
#define TILE_SIZE				64

// 전역 변수
BOOL g_bAtiveApp;				// 앱이 활성화 상태인지 체크
HIMC g_hOldIMC;					// 한글 입력 IME창 제거.
HINSTANCE hInst;				// 현재 인스턴스입니다.
HWND g_hWnd;					// 메인 윈도우 입니다.
LONGLONG dlOneSecCheck_Old;		// 게임 시작 후 1초를 체크. 이전 시간
LONGLONG dlOneSecCheck_New;		// 게임 시작 후 1초를 체크. 현재 시간
DWORD dwOneFrame = 1000 / 50;	// 1프레임의 밀리 세컨드 단위. 

// 전역 구조체
RECT rt;						// 작업 영역 저장

// 맵의 전체 타일 갯수(넓이), 맵 전체 타일 갯수(높이), 윈도우 사이즈(넓이), 윈도우 사이즈(높이), 타일 1개의 사이즈를 입력받음.

// 전역 객체
BaseObject* g_pPlayerObject;																			// 내 플레이어 객체
CScreenDib* g_cScreenDib = CScreenDib::Getinstance(WINDOW_WIDTH, WINDOW_HEIGHT, 32);										// 싱글톤으로 CScreenDib형 객체 하나 얻기.
CSpritedib* g_cSpriteDib = CSpritedib::Getinstance(70, 0x00ffffff, 32);									// 싱글톤으로 CSpritedib형 객체 하나 얻기
CMap* g_cMap = CMap::Getinstance(MAPSIZE_WIDTH_PIXEl / TILE_SIZE, MAPSIZE_HEIGHT_PIXEl / TILE_SIZE,		// 싱글톤으로 Map형 객체 하나 얻기.
	WINDOW_WIDTH, WINDOW_HEIGHT, TILE_SIZE);

CList<BaseObject*> list;		// 객체 저장 리스트


// 함수 선언문
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR  CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
void Update();			// 게임 업데이트 함수
void KeyProcess();		// Update함수 안의 키 입력 체크 파트
void Action();			// Update함수 안의 Action 체크 파트
void Draw();			// Update함수 안의 Draw 파트(랜더링)

// 통신용 전역변수
TCHAR g_tIP[30];				// 입력한 서버의 IP

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	//AllocConsole();
	//freopen("CONOUT$", "wt", stdout);
	//printf("Console Ready!\n");

	// 비트맵 로드하기
	g_cSpriteDib->GameInit();

	// IP 입력받는 다이얼로그 시작.
	INT_PTR EndCode = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDR), NULL, DialogProc);
	if (EndCode == IDCANCEL)	// 정상 종료.
		return 0;	

	// timeBeginPeriod 적용
	timeBeginPeriod(1);

	// 윈도우 클래스 초기화
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TCPFIGHERNETCLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TCPFIGHERNETCLIENT);
	wcex.lpszClassName = TEXT("TCP_Fighter");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);


	// 응용 프로그램 초기화를 수행합니다.

	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	g_hWnd = CreateWindowW(TEXT("TCP_Fighter"), TEXT("TCP_Fighter"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!g_hWnd)
		return FALSE;

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	GetClientRect(g_hWnd, &rt);

	MSG msg;

	// 윈도우 크기를 640 X 480으로 변경시킨다.
	RECT WindowRect;
	WindowRect.top = 0;
	WindowRect.left = 0;
	WindowRect.right = 640;
	WindowRect.bottom = 480;

	AdjustWindowRectEx(&WindowRect, GetWindowStyle(g_hWnd), GetMenu(g_hWnd) != NULL, GetWindowExStyle(g_hWnd));	// 원하는 위치로 사이즈를 다시 잡아준다.
	MoveWindow(g_hWnd, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, TRUE);		// 윈도우를 이동시켜서 위치를 잡는다.

	// 네트워크 사용 준비. 윈속초기화/소켓생성/WSAAsyncSelect()/커넥트
	BOOL check = NetworkInit(g_hWnd, g_tIP);
	if (check == FALSE)
	{		
		list.clear();

		delete g_pPlayerObject;
		timeEndPeriod(1);

		NetworkClose();
		return 0;
	}

	// 게임용 기본 메시지 루프입니다.
	while (1)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		else
		{
			Update();
		}
	}

	list.clear();

	delete g_pPlayerObject;
	timeEndPeriod(1);

	NetworkClose();	
	//FreeConsole();
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		g_hOldIMC = ImmAssociateContext(hWnd, NULL);	// IMB 제거
		break;

		// 앱 활성화 여부 체크
	case WM_ACTIVATEAPP:
		g_bAtiveApp = (BOOL)wParam;
		break;

	case WM_COMMAND:
	{
		// 메뉴 선택을 구문 분석합니다.
		switch (LOWORD(wParam))
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
	{	
		BOOL closeCheck = NetworkProc(lParam);
		if (closeCheck == FALSE)
		{
			MessageBox(g_hWnd, _T("죽었거나, 서버가 꺼짐."), _T("접속 종료"), 0);
			PostQuitMessage(0);	 // 종료 메시지를 발생시킨다.
		}
	}
	break;

	case WM_DESTROY:
		ImmAssociateContext(hWnd, g_hOldIMC);	// IMB 원복.
		
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

void Update()
{
	static LONGLONG dlTextOutCheck = 0;		// 1초 단위 계산을 위한 것.
	static DWORD FPStemp = 0;				// 계산용 FPS
	static DWORD FPS = 0;					// 1초 단위로 실제 출력하는 FPS. 1초가 되면 FPS = FPStemp를 한다
	static DWORD dwLogicCountTemp = 0;		// 계산용 로직 작동 수
	static DWORD dwLogicCount = 0;			// 1초 단위로 실제 출력하는 로직 작동 수. 1초가 되면 dwLogicCount = dwLogicCountTemp를 한다.
	static LONGLONG dlOldTime = 0;			// 프레임 시작 시간.
	static LONGLONG dlNewTime = 0;			// 현재 시간
	static LONGLONG dlSkipSec = 0;			// 실제 경과한 시간
	BOOL bCheck = FALSE;					// 최초 루틴인지 아닌지 체크. TRUE면 최초 루틴.
	

	//---------------------------
	// 최초 루틴인지 체크 후 시간 체크
	// --------------------------	
	// 최초 루틴이라면
	if (dlOldTime == 0)
	{
		dlOldTime = timeGetTime();
		dlOneSecCheck_Old = dlOldTime;
		bCheck = TRUE;

		dlTextOutCheck = timeGetTime();
	}
	// 최초 루틴이 아니라면
	else
	{
		dlNewTime = timeGetTime();
		dlOneSecCheck_New = timeGetTime();
		dlOneSecCheck_Old = dlOneSecCheck_New;
	}		


	//---------------------------
	// 키 다운 체크
	// --------------------------
	if (g_bAtiveApp)
		KeyProcess();

	//---------------------------
	// 액션
	// --------------------------
	Action();			

	//---------------------------
	// 슬립 / 드로우 체크 (최초 루틴이 아닐 때만)
	//---------------------------
	if (bCheck == FALSE)
	{
		// 일단 경과 시간을 더한다.
		dlSkipSec += dlNewTime - dlOldTime;

		// 슬립 여부 체크
		// 경과 시간이, 20보다 적으면 (이전 시간까지 더해서) Sleep한다.
		if (dlSkipSec < dwOneFrame)
		{
			Sleep(dwOneFrame - dlSkipSec);
			dlNewTime += dwOneFrame - dlSkipSec;	// 슬립 후, 슬립 시간을 New시간에 더한다. 만약, 17밀리 세컨드가 나왔으면, 3슬립 후 New시간에 3밀리 세컨드를 더한다.
													// 그래야 즉, 다음 계산 시에는 20 밀리 세컨드가 나왔던 것 처럼 계산하기 위함이다.
			dlSkipSec = 0;					// 시간 만큼 쉬었으니, dlSkipSec을 0으로 만든다.
		}

		// Draw
		// 슬립 계산 후, 남은 경과 시간(dlSkipSec)이 20보다 크다면, 1프레임 스킵. 즉, 랜더링 안하고 로직만 처리
		if (dwOneFrame <= dlSkipSec)
			dlSkipSec -= dwOneFrame;	// 드로우를 스킵한다.

		// 슬립 계산 후, 남은 경과 시간(dlSkipSec)이 20보다 작다면, 드로우 후 다음 로직에서 남은 경과시간을 처리한다.
		else
		{			
			// 드로우 후 플립
			Draw();
			g_cScreenDib->DrawBuffer(g_hWnd, rt.left, rt.top);	// Flip
			FPStemp++;	
		}

		// New시간을 Old시간으로 복사
		dlOldTime = dlNewTime;
	}	

	//---------------------------
	// 스트링 표시를 위한 처리
	// --------------------------
	// 1초가 됐을 때 처리
	if (timeGetTime() - dlTextOutCheck >= 1000)
	{
		FPS = FPStemp;
		dwLogicCount = dwLogicCountTemp;
		dwLogicCountTemp = 0;
		FPStemp = 0;
		dlTextOutCheck = timeGetTime();
	}
	
	HDC hdc = GetDC(g_hWnd);
	TCHAR str[10];
	TCHAR str3[10];
	_itow_s(FPS, str, _countof(str), 10);				
	_itow_s(dwLogicCount, str3, _countof(str3), 10);
	TextOut(hdc, 0, 0, str, _tcslen(str));				// FPS 
	TextOut(hdc, 0, 30, str3, _tcslen(str3));			// 로직 처리 수
	ReleaseDC(g_hWnd, hdc);
		

	//---------------------------
	// 로직 처리 수 증가.
	// --------------------------
	dwLogicCountTemp++;
}

// Update함수 안의 키 입력 체크 파트
void KeyProcess()
{
	//---------------------------
	// 키보드 입력 파트 : 플레이어의 m_dwActionCur변수에 값 저장.
	// --------------------------
	if (g_pPlayerObject != NULL)
	{
		if (GetAsyncKeyState(0x5A) & 0x8000)					// Z버튼( 1번공격)
		{
			g_pPlayerObject->ActionInput(dfACTION_ATTACK_01_LEFT);
		}

		else if (GetAsyncKeyState(0x58) & 0x8000)					// X버튼( 2번공격)
		{
			g_pPlayerObject->ActionInput(dfACTION_ATTACK_02_LEFT);
		}

		else if (GetAsyncKeyState(0x43) & 0x8000)					// C버튼( 3번공격)
		{
			g_pPlayerObject->ActionInput(dfACTION_ATTACK_03_LEFT);
		}

		else if (GetAsyncKeyState(VK_LEFT) & 0x8000 && GetAsyncKeyState(VK_UP) & 0x8000)	// 좌상
		{
			g_pPlayerObject->ActionInput(dfACTION_MOVE_LU);
		}

		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && GetAsyncKeyState(VK_UP) & 0x8000)	// 우상
		{
			g_pPlayerObject->ActionInput(dfACTION_MOVE_RU);
		}

		else if (GetAsyncKeyState(VK_LEFT) & 0x8000 && GetAsyncKeyState(VK_DOWN) & 0x8000)	// 좌하
		{
			g_pPlayerObject->ActionInput(dfACTION_MOVE_LD);
		}

		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && GetAsyncKeyState(VK_DOWN) & 0x8000)	// 우하
		{
			g_pPlayerObject->ActionInput(dfACTION_MOVE_RD);
		}

		else if (GetAsyncKeyState(VK_UP) & 0x8000)						// 상
		{
			g_pPlayerObject->ActionInput(dfACTION_MOVE_UU);
		}

		else if (GetAsyncKeyState(VK_DOWN) & 0x8000)					// 하	
		{
			g_pPlayerObject->ActionInput(dfACTION_MOVE_DD);
		}

		else if (GetAsyncKeyState(VK_LEFT) & 0x8000)					// 좌
		{
			g_pPlayerObject->ActionInput(dfACTION_MOVE_LL);
		}

		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)					// 우
		{
			g_pPlayerObject->ActionInput(dfACTION_MOVE_RR);
		}

		else
			g_pPlayerObject->ActionInput(dfACTION_IDLE);				// 이동키 입력하지 않았다면,
	}

}

// Update함수 안의 Action 체크 파트
void Action()
{
	// 범위 내에 있는 유저만 Actin 체크
	CList<BaseObject*>::iterator itor;
	for (itor = list.begin(); itor != list.end(); itor++)
		(*itor)->Action();

	// 리스트 안의 모든 객체를 Y값을 기준으로 정렬. Y값이 작을 수록 리스트의 앞으로 온다.
	// 즉, Y값이 작을 수록, 먼저 Draw된다.
	CList<BaseObject*>::iterator Nowitor = list.begin();
	CList<BaseObject*>::iterator Nextitor = Nowitor;
	Nextitor++;

	// 비교할 노드가 끝 노드가 아니라면 계속 반복.
	while (Nowitor != list.end())
	{
		// 다음 노드가 끝 노드라면 다음 노드를 현재 노드에 넣어서, 다음 로직 시작 시 while문이 종료되도록 한다.
		if (Nextitor == list.end())
		{
			Nowitor++;
			Nextitor = Nowitor;
			Nextitor++;
		}

		// 다음 노드가 끝 노드가 아니라면,
		else
		{

			// 만약, 현재 객체가 이펙트라면, 무조건 교체
			if ((*Nowitor)->GetObjectType() == 2)
			{
				// 값 교체
				Nowitor.ListSwap(Nextitor);

				// 다음 비교 위치 세팅
				Nowitor++;
				Nextitor = Nowitor;
				Nextitor++;
			}

			// 이펙트가 아니라면, 
			else
			{
				// 만약, <<쪽(Nowitor) 노드의 Y값이 더 작거나 같다면, Nextitor를 1 증가시킨다.		
				if ((*Nowitor)->GetCurY() <= (*Nextitor)->GetCurY())
					Nextitor++;

				// >>쪽(Nextitor) 노드의 Y값이 더 작다면, 값 교체
				// 교체 후, 다음 비교 위치 세팅
				else
				{
					// 값 교체
					Nowitor.ListSwap(Nextitor);

					// 다음 비교 위치 세팅
					Nowitor++;
					Nextitor = Nowitor;
					Nextitor++;
				}
			}
		}
	}
}

// Update함수 안의 Draw 파트(랜더링)
void Draw()
{
	//---------------------------
	// 내 백버퍼의 버퍼 및 정보 얻음
	// --------------------------
	BYTE* bypDest = g_cScreenDib->GetDibBuffer();
	int iDestWidth = g_cScreenDib->GetWidth();
	int iDestHeight = g_cScreenDib->GetHeight();
	int iDestPitch = g_cScreenDib->GetPitch();


	//---------------------------
	// 버퍼 포인터에 그림을 그린다.
	// --------------------------
	// 배경 그리기(Map 객체에게 맵을 그리게 한다)
	BOOL iCheck = g_cMap->MapDraw(bypDest, iDestWidth, iDestHeight, iDestPitch);
	if (!iCheck)
		exit(-1);

	// 내 카메라 좌표 구해서 Map객체에 저장하기
	// 내 캐릭터가 아직 없으면, 카메라 위치는 무조건 0으로 셋팅된다.
	if (g_pPlayerObject == nullptr)
		g_cMap->SetCameraPos(0, 0, g_cSpriteDib->ePLAYER_STAND_L01);
	else
		g_cMap->SetCameraPos(g_pPlayerObject->GetCurX(), g_pPlayerObject->GetCurY(), g_pPlayerObject->GetSprite());
		
	
	CList<BaseObject*>::iterator itor;
	for (itor = list.begin(); itor != list.end(); itor++)
	{
		(*itor)->Draw(bypDest, iDestWidth, iDestHeight, iDestPitch);

		// 호출 후, 이번에 호출한 객체가 이펙트이며, 재생이 끝났다면 해당 이펙트는 리스트에서 제외 후 메모리를 삭제한다.
		if ((*itor)->Get_bGetEndFrame() == TRUE && (*itor)->GetObjectType() == 2)
		{
			BaseObject* DeleteEffect = *itor;

			delete DeleteEffect;
			list.erase(itor);
			++itor;
		}

		/*
		int NowX = (*itor)->GetCurX();
		int NowY = (*itor)->GetCurY();

		// 해당 객체가 내 카메라 범위 내에 있을 때만 그린다.
		if (CameraX < NowX && NowX < CameraX + WINDOW_WIDTH &&
			CameraY < NowY && NowY < CameraY + WINDOW_HEIGHT)
		{

			(*itor)->Draw(bypDest, iDestWidth, iDestHeight, iDestPitch);

			// 호출 후, 이번에 호출한 객체가 이펙트이며, 재생이 끝났다면 해당 이펙트는 리스트에서 제외 후 메모리를 삭제한다.
			if ((*itor)->Get_bGetEndFrame() == TRUE && (*itor)->GetObjectType() == 2)
			{
				BaseObject* DeleteEffect = *itor;

				delete DeleteEffect;
				list.erase(itor);
				++itor;
			}
		}
		*/
	}

}