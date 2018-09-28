#ifndef __MONITOR_VIEW_H__
#define __MONITOR_VIEW_H__

//// 자식 윈도우 클래스 헤더 ////////////////////////////
#include "Protocol_Set\CommonProtocol.h"

// --------------- 
// 큐 모음
// --------------- 
namespace Library_Jingyu
{

#define QUEUE_LEN 100

	// 큐 모음
	typedef struct
	{
		int front;
		int rear;
		int Peek;
		int queArr[QUEUE_LEN] = { 0, };
	}Queue;

	// 초기화
	void Init(Queue* pq);
	// 다음 위치 확인
	int NextPos(int pos);
	// 디큐
	int Dequeue(Queue* pq);
	// 인큐
	void Enqueue(Queue* pq, int y);
	// 첫 큐 Peek
	bool FirstPeek(Queue* pq, int* Data);
	// 다음 큐 Peek
	bool NextPeek(Queue* pq, int* Data);
}

// --------------- 
// 모니터링 뷰어
// --------------- 
namespace Library_Jingyu
{

#define dfMAXCHILD		100
#define UM_PARENTBACKCHANGE WM_USER+1	// 자식이 부모에게 보내는 메시지
#define UM_CHILDFONTCHANGE	WM_USER+2   //부모가 자식에게 보내는 메시지


	class CMonitorGraphUnit
	{

	private:
		struct ColumnInfo
		{
			int ServerID;
			int DataType;
			int iLastVal = 0;			// 현재 큐에 들어온 가장 마지막 값
			TCHAR DataName[20];
			Queue queue;
		};

		ColumnInfo *CoInfo;

	public:

		enum TYPE
		{
			BAR_SINGLE_VERT,
			BAR_SINGLE_HORZ,
			BAR_COLUMN_VERT,
			BAR_COLUMN_HORZ,
			LINE_SINGLE,
			LINE_MULTI,
			PIE,
			ONOFF
		};

	public:

		CMonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, COLORREF BackColor, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight, LPCTSTR str);
		~CMonitorGraphUnit();

		/////////////////////////////////////////////////////////
		// 윈도우 프로시저
		/////////////////////////////////////////////////////////
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		/////////////////////////////////////////////////////////
		// 데이터 넣기, 처음 디폴트 값 추가세팅
		/////////////////////////////////////////////////////////
		void InsertData(int iData, int ServerID, int DataType);
		void AddData(int iMax, int MaxAleCount, int MinAleCount, LPCTSTR Unit);

		/////////////////////////////////////////////////////////
		// 컬럼 정보 최초 세팅
		/////////////////////////////////////////////////////////
		void SetColumnInfo(int iColumnCount, int ServerID[], int DataType[], TCHAR DataName[][20]);

		//------------------------------------------------------
		// 알람 시, 부모 배경 빨간색으로 칠하기 체크 함수
		//------------------------------------------------------
		void ParentBackCheck(int Data);		// 최대
		void ParentBackCheck_Min(int Data);	// 최소

	private:
		//------------------------------------------------------
		// Paint함수
		//------------------------------------------------------
		void Paint_LINE_SINGLE();
		void Paint_LINE_MULTI();
		void Paint_BAR_SINGLE_VERT();
		void Paint_BAR_COLUMN_VERT();
		void Paint_PIE();
		void paint_ONOFF();		

		//------------------------------------------------------
		// 기본 UI 세팅
		//------------------------------------------------------
		void UISet();

		//------------------------------------------------------
		// 부모 윈도우 핸들, 내 윈도우 핸들, 인스턴스 핸들
		//------------------------------------------------------
		HWND hWndParent;
		HWND hWnd;
		HINSTANCE hInstance;

		//------------------------------------------------------
		// 윈도우 위치,크기,색상, 그래프 타입 등.. 자료
		//------------------------------------------------------
		TYPE _enGraphType;
		RECT rt;
		RECT TitleRt;
		RECT LeftRt;
		RECT RightRt;	// LINE_MULTI타입 그래프에서만 사용된다. 화면 우측에 그래프 정보 표시하는 공간.
		RECT BottomRt;	// BAR_COLUMN_VERT타입 그래프에서만 사용된다. 화면 하단에 그래프 정보 표시하는 공간.
		RECT ClientRt;
		COLORREF BackColor;

		//------------------------------------------------------
		// 데이터
		//------------------------------------------------------	
		Queue queue;	// 큐	
		double iAddY;
		int Greedint[4];

		// 해당 윈도우가 알람을 울릴지 말지. 지정한 값보다 큰 값이 나오면 알람이 울리도록 한다.
		// false면 울리지 않음
		bool AleOnOff;			

		// 해당 윈도우가 알람을 울릴지 말지. 지정한 값보다 작은 값이 나오면 알람이 울리도록 한다.
		// false면 울리지 않음.
		bool MinAleOnOff;	

		int iColumnCount;		// 해당 윈도우의 컬럼 수 카운트

		//------------------------------------------------------
		// 타이틀에 표시될 텍스트
		//------------------------------------------------------	
		TCHAR tWinName[20];		// 윈도우 이름
		TCHAR tUnit[10];		// 표시 단위	

		//------------------------------------------------------
		// 그래프 Max값 , 경고 울리는 값
		//------------------------------------------------------
		bool MaxVariable;
		int iMax;
		double AleCount;
		double MinAleCount;
		bool bObjectCheck;

		//------------------------------------------------------
		// MemDC와 리소스
		//------------------------------------------------------
		HDC MemDC;
		HBRUSH BackBrush, TitleBrush;
		HBRUSH PieBrush[5];						// 파이 채우기 용 브러시
		HBITMAP MyBitmap, OldBitmap;
		HFONT MyFont;
		COLORREF NowFontColor, NormalFontColor;	// 사용할 폰트의 컬러 , 해당 윈도우의 기본 폰트 컬러
												// 따로 둔 이유는, 코드에서는 NowFontColor만 적용시키고 상황에 따라서 NowFontColor폰트가 NormalFontColor 혹은 빨간 폰트가 된다.
		HPEN* GraphPen;


		//------------------------------------------------------
		// 나의 this를 꺼내서 쓰는 포인터
		//------------------------------------------------------
		static CMonitorGraphUnit *pThis;
	};

}

#endif // !__MONITOR_VIEW_H__
