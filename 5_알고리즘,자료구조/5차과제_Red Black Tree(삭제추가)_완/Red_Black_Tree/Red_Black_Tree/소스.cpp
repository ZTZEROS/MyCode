// Red_Black_Tree.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "source.h"
#include "Red_Black_Tree.h"
#include <time.h>

#define ID_INSERT_EDIT		100
#define ID_INSERT_BUTTON	101

#define ID_DELETE_EDIT		102
#define ID_DELETE_BUTTON	103


// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
HWND hInsertEdit;								// 입력 에디트 핸들
HWND hDeleteEdit;								// 삭제 에디트 핸들
CRBT redblackTree;								// 레드 블랙 트리
RECT rt;


// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    // TODO: 여기에 코드를 입력합니다.	

	/*redblackTree.Insert(20);
	redblackTree.Insert(25);
	redblackTree.Insert(30);
	redblackTree.Insert(35);
	redblackTree.Insert(40);
	redblackTree.Insert(45);
	redblackTree.Insert(50);
	redblackTree.Insert(55);
	redblackTree.Insert(60);
	redblackTree.Insert(65);
	redblackTree.Insert(70);
	redblackTree.Insert(75);
	redblackTree.Insert(80);
	redblackTree.Insert(85);

	redblackTree.Insert(5);
	redblackTree.Insert(10);
	redblackTree.Insert(15);

	redblackTree.Insert(2);
	redblackTree.Insert(4);
	redblackTree.Insert(6);
	redblackTree.Insert(8);

	redblackTree.Insert(14);
	redblackTree.Insert(31);
	redblackTree.Insert(32);

	redblackTree.Delete(35);
	redblackTree.Delete(32);
	redblackTree.Delete(31);
	redblackTree.Delete(30);
	redblackTree.Delete(25);
	redblackTree.Delete(20);
	redblackTree.Delete(15);
	redblackTree.Delete(14);
	redblackTree.Delete(10);
	redblackTree.Delete(8);
	redblackTree.Delete(6);

	redblackTree.Delete(5);
	redblackTree.Delete(55);
	redblackTree.Delete(50);
	redblackTree.Delete(45);
	redblackTree.Delete(40);
	redblackTree.Delete(65);
	redblackTree.Delete(60);
	redblackTree.Delete(4);*/

	srand(time(NULL));
	for (int i = 1; i < 100; ++i)
		redblackTree.Insert((rand() % 30) +1);



	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REDBLACKTREE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_REDBLACKTREE);
	wcex.lpszClassName = _T("레드블랙트리");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	HWND hWnd = CreateWindowW(_T("레드블랙트리"), _T("레드블랙트리"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(hWnd, nCmdShow);

	GetClientRect(hWnd, &rt);

    MSG msg;

    // 기본 메시지 루프입니다.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
		TranslateMessage(&msg);
		DispatchMessage(&msg);      
    }

    return (int) msg.wParam;
}


//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		hInsertEdit = CreateWindowW(_T("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			10, 10, 100, 20, hWnd, (HMENU)ID_INSERT_EDIT, hInst, nullptr);

		CreateWindowW(_T("button"), _T("삽입"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			120, 10, 50, 20, hWnd, (HMENU)ID_INSERT_BUTTON, hInst, nullptr);

		hDeleteEdit = CreateWindowW(_T("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			10, 50, 100, 20, hWnd, (HMENU)ID_DELETE_EDIT, hInst, nullptr);

		CreateWindowW(_T("button"), _T("삭제"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			120, 50, 50, 20, hWnd, (HMENU)ID_DELETE_BUTTON, hInst, nullptr);

		break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다.
            switch (wmId)
            {

			// [삽입] 버튼을 눌렀을 때, 에디터에 있는 스트링을 가져와서 숫자로 변경 후, 레드 블랙 트리에 추가.
			case ID_INSERT_BUTTON:
			{				
				TCHAR str[20];
				GetWindowText(hInsertEdit, str, 20);
				int insertNum = _ttoi(str);

				// 삽입
				redblackTree.Insert(insertNum);

				// 다시 그리기
				InvalidateRect(hWnd, NULL, true);			
			}
			break;

			// [삭제] 버튼을 눌렀을 때, 데이터에 있는 스트링을 가져와서 숫자로 변경 후, 레드 블랙 트리에서 삭제.
			case ID_DELETE_BUTTON:
			{
				TCHAR str[20];
				GetWindowText(hDeleteEdit, str, 20);
				int DeletetNum = _ttoi(str);

				// 삭제
				redblackTree.Delete(DeletetNum);

				// 다시 그리기
				InvalidateRect(hWnd, NULL, true);
			}
			break;

				
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.
			GetClientRect(hWnd, &rt);
			redblackTree.Traversal(hdc, rt);

            EndPaint(hWnd, &ps);
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
