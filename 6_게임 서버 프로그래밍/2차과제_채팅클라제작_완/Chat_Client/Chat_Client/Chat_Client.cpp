// Chat_Client.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Chat_Client.h"
#include "Network_Func.h"

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
HWND g_hWndLobby;								// 로비 모달리스 다이얼로그의 핸들 저장 변수.
HWND g_hWndRoom;								// 룸 모달리스 다이얼로그의 핸들 저장 변수

// 통신용 전역변수
TCHAR g_tIP[30];								// 입력한 서버의 IP
TCHAR g_tNickName[dfNICK_MAX_LEN];				// 내 닉네임

// 이 코드 모듈에 들어 있는 함수의 정방향 선언.
INT_PTR CALLBACK    IPDialolgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    LobbyDialolgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    RoomDialolgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

    // TODO: 여기에 코드를 입력합니다.
	hInst = hInstance;

	// IP 입력받는 다이얼로그 시작.
	INT_PTR EndCode = DialogBox(hInst, MAKEINTRESOURCE(IDD_IP), NULL, IPDialolgProc);
	if (EndCode == IDCANCEL)	// 정상 종료.
		return 0;
		
	// 입력받은 IP와 닉네임으로 로비 모달리스 다이얼로그 생성	
	g_hWndLobby = CreateDialog(hInst, MAKEINTRESOURCE(IDD_LOBBY), NULL, LobbyDialolgProc);
	ShowWindow(g_hWndLobby, nCmdShow);

	// 필요한 정보, 네트워크 파트로 넘기기
	InfoGet(g_hWndLobby, &g_hWndRoom, hInst, nCmdShow);

	// 네트워크 사용 준비. 윈속초기화/소켓생성/WSAAsyncSelect()/커넥트
	BOOL check = NetworkInit(g_tIP, g_tNickName);
	if (check == FALSE)
	{
		NetworkClose();
		return 0;
	}

	// 각종 인자 넘기기. 로비 다이얼로그 핸들(메인 윈도우 개념), 인스턴스 넘기기

	// 다이얼로그 루프문
	MSG msg;
	BOOL bRet;
	while ((bRet = GetMessage(&msg, NULL,0,0)) != 0)
	{

		if (bRet == -1)
			break;

		bRet = FALSE;

		// 룸이 열려있는데, 엔터키 눌렀을 때 처리. 채팅송신 처리
		if (msg.message == WM_KEYDOWN)
			if (IsWindow(g_hWndRoom) && msg.wParam == VK_RETURN)
				ChatLogic();

		// 방 모달리스 다이얼로그가 열려있는지 체크 후 처리
		if (IsWindow(g_hWndRoom))
			bRet |= IsDialogMessage(g_hWndRoom, &msg);		

		// 로비 모달리스 다이얼로그가 열려있는지 체크 후 처리
		if (IsWindow(g_hWndLobby))
			bRet |= IsDialogMessage(g_hWndLobby, &msg);

		if (!bRet)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}

	return (int)msg.wParam;
}


// IP 입력받는 다이얼로그 프록 (모달 다이얼로그)
INT_PTR  CALLBACK IPDialolgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		ZeroMemory(g_tIP, sizeof(g_tIP));
		ZeroMemory(g_tNickName, sizeof(g_tNickName));
		SetDlgItemText(hWnd, IDC_EDIT1, _T("127.0.0.1"));
		SetDlgItemText(hWnd, IDC_EDIT2, _T("손님"));
		RECT rt;
		GetWindowRect(hWnd, &rt);
		rt.left = 800;
		rt.top = 400;

		MoveWindow(hWnd, rt.left, rt.top, rt.right, rt.bottom, TRUE);

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDOK:
			{	
				// 에디터 컨트롤에서 입력한 IP 얻기
				GetDlgItemText(hWnd, IDC_EDIT1, g_tIP, sizeof(g_tIP));

				// 에디터 컨트롤에서 입력한 닉네임 얻기
				GetDlgItemText(hWnd, IDC_EDIT2, g_tNickName, sizeof(g_tNickName));

				// 다이얼로그 종료. 다음 다이얼로그를 열어야하니까!
				EndDialog(hWnd, IDOK);
				return (INT_PTR)TRUE;
			}
			case IDCANCEL:
				EndDialog(hWnd, IDCANCEL);
				return (INT_PTR)TRUE;

		}

	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)FALSE;

}

// Lobby 모달리스 다이얼로그 프록
INT_PTR CALLBACK	LobbyDialolgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		RECT rt;
		GetWindowRect(hWnd, &rt);
		rt.left = 700;
		rt.top = 200;

		MoveWindow(hWnd, rt.left, rt.top, rt.right, rt.bottom, TRUE);
		return (INT_PTR)TRUE;	

	case WM_CLOSE:
		PostQuitMessage(0);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			// 리스트 박스라면
			case IDC_LIST1:
			{
				switch (HIWORD(wParam))
				{
					// 리스트의 항목을 더블클릭했다면
					case LBN_DBLCLK:
					{
						// 사용자가 선택한 행의 Index 얻기
						DWORD GetIndex = (DWORD)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);

						// Index의 방 번호 얻기
						DWORD RoomNo = (DWORD)SendMessage(GetDlgItem(hWnd, IDC_LIST1), LB_GETITEMDATA, GetIndex, 0);

						// 방 입장 요청 패킷 제작
						CProtocolBuff header(dfHEADER_SIZE);
						CProtocolBuff payload;
						CreatePacket_Req_RoomJoin((char*)&header, (char*)&payload, RoomNo);

						// 패킷을 SendBuff에 넣기
						SendPacket(&header, &payload);

						// SendBuff의 데이터 Send
						SendProc();

					}
					return (INT_PTR)TRUE;					

					default:
						return (INT_PTR)FALSE;
				}
			}

			// 방생성 버튼을 눌렀다면
			case IDC_CREATEBUTTON:
			{
				// 유저가 입력한 방 이름을 알아온다.
				TCHAR RoomName[ROOMNAME_SIZE];
				GetDlgItemText(hWnd, IDC_ROOMNAME, RoomName, ROOMNAME_SIZE);

				// "방 생성 요청" 패킷 제작
				CProtocolBuff header(dfHEADER_SIZE);
				CProtocolBuff payload;

				CreatePacket_Req_RoomCreate((char*)&header, (char*)&payload, RoomName);

				// 만든 패킷, SendBuff에 넣기
				SendPacket(&header, &payload);

				// SendBuff의 데이터 Send
				SendProc();

			}
			return (INT_PTR)TRUE;				

			default:
				return (INT_PTR)FALSE;
		}
	
	case WM_SOCK:
		{	
			BOOL closeCheck = NetworkProc(lParam);
			if (closeCheck == FALSE)
				PostQuitMessage(0);	 // 종료 메시지를 발생시킨다.

			return (INT_PTR)TRUE;
		}
		
	default:
		return (INT_PTR)FALSE;
	}

	return (INT_PTR)FALSE;
}

// Room 모달리스 다이얼로그 프록
INT_PTR CALLBACK    RoomDialolgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// 룸 나가기 버튼을 누르면
		case WM_CLOSE:
		{
			// "방 퇴장 요청" 패킷 제작
			CProtocolBuff header(dfHEADER_SIZE);
			CProtocolBuff payload;

			CreatePacket_Req_RoomLeave((char*)&header);

			// SendBuff에 넣기
			SendPacket(&header, &payload);

			// SendBuff의 데이터 Send하기
			SendProc();			
		}
			return (INT_PTR)TRUE;

		default:
			return (INT_PTR)FALSE;
	}

	return (INT_PTR)FALSE;
}