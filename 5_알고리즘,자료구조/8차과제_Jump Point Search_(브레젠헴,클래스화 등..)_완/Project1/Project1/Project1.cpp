// Project1.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Project1.h"
#include "Jump_Point_Search.h"

#define MAP_WIDTH			60
#define MAP_HEIGHT			30
#define MAP_TILE_RADIUS		15

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
Map stMapArray[MAP_HEIGHT][MAP_WIDTH];			// 맵 Array													
Map* stStartTile;								// 출발지 Tile
Map* stFinTile;									// 목적지 Tile
CFindSearch J_Search(stMapArray[0], MAP_WIDTH, MAP_HEIGHT, MAP_TILE_RADIUS, 300);		// JPS 생성자.
int iStartX, iStartY, iFinX, iFinY;														// 시작 타일, 도착 타일의 타일 기준 좌표(픽셀 기준 아님)

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void	CALLBACK	TimerProc(HWND, UINT, UINT_PTR, DWORD);
void PositionSet();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    // TODO: 여기에 코드를 입력합니다.

	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PROJECT1);
	wcex.lpszClassName = _T("점프포인트서치");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// 응용 프로그램 초기화를 수행합니다.
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.
	HWND hWnd = CreateWindowW(_T("점프포인트서치"), _T("점프포인트서치"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	// 최초 1번, Start 타일 위치, Final 타일 위치 셋팅 및 맵 데이터 셋팅
	PositionSet();

	// J_Search 객체에게 시작 타일, 종료 타일 위치 전달.
	J_Search.Init(iStartX, iStartY, iFinX, iFinY);

	// DC셋팅을 위해 hWnd전달.
	J_Search.DCSet(hWnd);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);      

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
	static Map* stNowTile = NULL;						// 마우스 L버튼 당시, 유저가 클릭한 타일을 저장한다.
	static bool bLButtonClickCheck = false;				// 마우스 L버튼 클릭 체크
	static bool bSpaveKeyDownCheck = false;				// 스페이스 키를 눌렀는지 체크.
	static int SaveX, SaveY;
	static int iLButtonDownX, iLButtonDownY;			// 마우스 L버튼을 눌렀을 때, 타일의 X,Y좌표

    switch (message)
    {

	case WM_SIZE:
		// DC 다시 셋팅
		J_Search.DCSet(hWnd);

		// 그리고 출력
		J_Search.GridShow();
		break;
	case WM_TIMER:
		J_Search.GridShow();
		break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다.
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

			case IDM_OBSTACLE_CLEAR:
			{
				// 모든 타일을 체크하며 장해물을 NONE으로 만든다
				for (int i = 0; i < MAP_HEIGHT; ++i)
					for (int j = 0; j < MAP_WIDTH; ++j)
						if (stMapArray[i][j].m_eTileType == OBSTACLE)
							stMapArray[i][j].m_eTileType = NONE;
			}
			J_Search.GridShow();
			break;

			case IDM_START:
			{				
				J_Search.Init(iStartX, iStartY, iFinX, iFinY);

				// 모든 타일을 체크하며 출발지, 도착지, 장해물을 제외한 모든 타일을 NONE으로 만든다
				for (int i = 0; i < MAP_HEIGHT; ++i)
				{
					for (int j = 0; j < MAP_WIDTH; ++j)
						if (stMapArray[i][j].m_eTileType != START && stMapArray[i][j].m_eTileType != FIN && stMapArray[i][j].m_eTileType != OBSTACLE)
							stMapArray[i][j].m_eTileType = NONE;
				}

				J_Search.StartNodeInsert();
				SetTimer(hWnd, 1, 300, TimerProc);				

			}
			break;

			case IDM_ONLY_PATH:
			{
				// 모든 타일을 체크하며 출발지, 도착지, 장해물을 제외한 모든 타일을 NONE으로 만든다
				for (int i = 0; i < MAP_HEIGHT; ++i)
				{
					for (int j = 0; j < MAP_WIDTH; ++j)
						if (stMapArray[i][j].m_eTileType != START && stMapArray[i][j].m_eTileType != FIN && stMapArray[i][j].m_eTileType != OBSTACLE)
							stMapArray[i][j].m_eTileType = NONE;
				}

				// 맵 타일만 다시 그린다.
				J_Search.GridShow();

				POINT point[100];	// 이 변수에 완성된 경로의 X,Y를 채워준다.
				int Count = J_Search.PathGet(iStartX, iStartY, iFinX, iFinY, point, true); // 현재 브레젠헴 알고리즘 적용중.

				// 완성된 경로를 그린다.
				J_Search.Out_Line(point, Count);

				// 모든 타일을 체크하며 출발지, 도착지, 장해물을 제외한 모든 타일을 NONE으로 만든다.
				// 길을 찾으며, 색을 바꿨는데, 반영 안되다가 WM_SIZE가 호출되면 색 있는걸로 그림이 다시 그려짐. 이상하니까 다시 초기화
				for (int i = 0; i < MAP_HEIGHT; ++i)
				{
					for (int j = 0; j < MAP_WIDTH; ++j)
						if (stMapArray[i][j].m_eTileType != START && stMapArray[i][j].m_eTileType != FIN && stMapArray[i][j].m_eTileType != OBSTACLE)
							stMapArray[i][j].m_eTileType = NONE;
				}
			}
			break;

			case IDM_CLEAR_COLOR:
				// 모든 타일을 체크하며 출발지, 도착지, 장해물을 제외한 모든 타일을 NONE으로 만든다
				for (int i = 0; i < MAP_HEIGHT; ++i)
					for (int j = 0; j < MAP_WIDTH; ++j)
						if (stMapArray[i][j].m_eTileType != START && stMapArray[i][j].m_eTileType != FIN && stMapArray[i][j].m_eTileType != OBSTACLE)
							stMapArray[i][j].m_eTileType = NONE;

				J_Search.GridShow();
				break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

	case WM_LBUTTONDOWN:
	{
		// 현재 타일을 이동시키거나, 장해물을 만든다.

		// 일단 현재 마우스 위치의 타일을 찾는다.
		// 현재 마우스 좌표 설정
		int iMouseY = HIWORD(lParam);
		int iMouseX = LOWORD(lParam);

		SaveX = iMouseX;
		SaveY = iMouseY;

		// 타일의 반지름 
		int iRadius = MAP_TILE_RADIUS;
		bool bCheck = false;

		// Tile을 뒤지면서 유저가 클릭한 타일을 찾는다.
		for (int i = 0; i < MAP_HEIGHT; ++i)
		{
			for (int j = 0; j < MAP_WIDTH; ++j)
			{
				// iMouseY, iMouseX와 현재 타일의 좌표를 체크해서 범위 내에 있으면 유저가 클릭한 타일을 찾은것
				if ((stMapArray[i][j].m_iMapX - iRadius) <= iMouseX && (stMapArray[i][j].m_iMapX + iRadius) >= iMouseX &&
					(stMapArray[i][j].m_iMapY - iRadius) <= iMouseY && (stMapArray[i][j].m_iMapY + iRadius) >= iMouseY)
				{
					iLButtonDownX = j;
					iLButtonDownY = i;
					// stNowTile에 현재 유저가 클릭한 타일 연결.
					stNowTile = &stMapArray[i][j];

					// 마우스 드래그 시, 장해물을 그리기 위해 마우스 L버튼 클릭 상태(bLButtonClickCheck)를 true로 만든다.
					bLButtonClickCheck = true;

					// break로 j for문 종료 후, i for문도 종료할 수 있도록, bCheck를 true로 만든다.
					bCheck = true;
					break;
				}
			}
			if (bCheck == true)
				break;
		}

		// 현재 타일이 NONE라면. 현재 타일을 장해물로 만든다.
		if (stNowTile->m_eTileType == NONE)
		{
			stNowTile->m_eTileType = OBSTACLE;
			J_Search.GridShow();
		}

		// 현재 타일이 장해물이라면, 현재 타일을 NONE으로 만든다.
		else if (stNowTile->m_eTileType == OBSTACLE)
		{
			stNowTile->m_eTileType = NONE;
			J_Search.GridShow();
		}
	}
	break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_CONTROL:
			bSpaveKeyDownCheck = true;
			break;
		default:
			break;
		}
		break;

	case WM_KEYUP:
		switch (wParam)
		{
		case VK_CONTROL:
			bSpaveKeyDownCheck = false;
			break;
		default:
			break;
		}
		break;

	case WM_MOUSEMOVE:
	{
		// 1. 현재 마우스 좌표 설정
		int iMouseY = HIWORD(lParam);
		int iMouseX = LOWORD(lParam);
		int iRadius = MAP_TILE_RADIUS;	// 타일의 반지름 
		bool bCheck = false;

		// 마우스 L버튼을 누른 상태이고, 스페이스 키를 안누른 상태라면, 마우스 이동 시 현재 타일에 장애물 등을 그린다.
		if (bLButtonClickCheck && bSpaveKeyDownCheck == false)
		{
			// 2. Tile을 뒤지면서 현재 유저의 마우스가 있는 타일을 찾는다.
			for (int i = 0; i < MAP_HEIGHT; ++i)
			{
				for (int j = 0; j < MAP_WIDTH; ++j)
				{
					// iMouseY, iMouseX와 현재 타일의 좌표를 체크해서 범위 내에 있으면 현재 유저의 마우스가 있는 위치를 찾아낸 것.
					if ((stMapArray[i][j].m_iMapX - iRadius) <= iMouseX && (stMapArray[i][j].m_iMapX + iRadius) >= iMouseX &&
						(stMapArray[i][j].m_iMapY - iRadius) <= iMouseY && (stMapArray[i][j].m_iMapY + iRadius) >= iMouseY)
					{

						// 찾은 타일이, 기존의 타일과 다르다면, 케이스에 따라 처리
						if (&(stMapArray[i][j]) != stNowTile)
						{
							// case 1. stNowTile가 START 타일이고, 찾은 타일이 NONE이라면, 현재 위치로 스타트 타일 설정하고 stNowTile 갱신
							if (stNowTile->m_eTileType == START && stMapArray[i][j].m_eTileType == NONE)
							{
								stNowTile->m_eTileType = NONE;
								stMapArray[i][j].m_eTileType = START;

								stNowTile = &(stMapArray[i][j]);

								iStartX = j;
								iStartY = i;
							}

							// case 2. stNowTile가 FIN 타일이고, 찾은 타일이 NONE이라면, 현재 위치로 목적지 타일을 설정하고 stNowTile 갱신
							else if (stNowTile->m_eTileType == FIN && stMapArray[i][j].m_eTileType == NONE)
							{
								stNowTile->m_eTileType = NONE;
								stMapArray[i][j].m_eTileType = FIN;

								stNowTile = &(stMapArray[i][j]);

								iFinX = j;
								iFinY = i;
							}

							// case 3. stNowTile가 NONE이라면, 현재 타일을 장해물 or NONE으로 설정하고 stNowTile갱신
							// 1개를 장해물로 설정했기 때문에 다시 그린다.
							//else if (stNowTile->m_eTileType == NONE)
							else if (stNowTile->m_eTileType == NONE || stNowTile->m_eTileType == OBSTACLE)
							{
								if (stMapArray[i][j].m_eTileType == NONE)
									stMapArray[i][j].m_eTileType = OBSTACLE;

								else if (stMapArray[i][j].m_eTileType == OBSTACLE)
									stMapArray[i][j].m_eTileType = NONE;

								stNowTile = &(stMapArray[i][j]);
							}

						}

						bCheck = true;
						break;
					}
				}
				if (bCheck == true)
					break;
			}

			J_Search.GridShow();
		}

		// 마우스 L버튼을 누른 상태이고, 스페이스 키를 누른 상태라면, 마우스 이동 시 브레젠험 알고리즘을 처리한다.
		else if (bLButtonClickCheck && bSpaveKeyDownCheck)
		{
			int i, j;
			// 2. Tile을 뒤지면서 현재 유저의 마우스가 있는 타일을 찾는다.
			for (i = 0; i < MAP_HEIGHT; ++i)
			{
				for (j = 0; j < MAP_WIDTH; ++j)
				{
					// iMouseY, iMouseX와 현재 타일의 좌표를 체크해서 범위 내에 있으면 현재 유저의 마우스가 있는 위치를 찾아낸 것.
					if ((stMapArray[i][j].m_iMapX - iRadius) <= iMouseX && (stMapArray[i][j].m_iMapX + iRadius) >= iMouseX &&
						(stMapArray[i][j].m_iMapY - iRadius) <= iMouseY && (stMapArray[i][j].m_iMapY + iRadius) >= iMouseY)
					{
						// 위치를 찾았으면 빠져나간다.
						bCheck = true;
						break;
						
					}
				}
				if (bCheck == true)
					break;
			}

			// 타일의 좌표를 넘긴다.
			J_Search.Bresenham_Line_Test(iLButtonDownX, iLButtonDownY, j, i);
		}
	}
	break;

	case WM_LBUTTONUP:
	{
		// 마우스 L 버튼 클릭상태 해제
		bLButtonClickCheck = false;
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

void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	J_Search.Jump_Point_Search_While();
}

void PositionSet()
{
	// 좌표 셋팅
	int iRadius = MAP_TILE_RADIUS;
	int TempiStartX = MAP_TILE_RADIUS;
	int TempiStartY = MAP_TILE_RADIUS;

	for (int i = 0; i < MAP_HEIGHT; ++i)
	{
		for (int j = 0; j < MAP_WIDTH; ++j)
		{

			stMapArray[i][j].m_iMapY = TempiStartY;
			stMapArray[i][j].m_iMapX = TempiStartX;

			TempiStartX += iRadius * 2;
		}

		TempiStartY += iRadius * 2;
		TempiStartX = iRadius;
	}

	stMapArray[10][20].m_eTileType = START;
	stMapArray[10][25].m_eTileType = FIN;

	iStartX = 20;
	iStartY = 10;
	iFinX = 25;
	iFinY = 10;

	stStartTile = &stMapArray[10][20];
	stFinTile = &stMapArray[10][25];
}