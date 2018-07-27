// Friend_Client.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"

#include <locale>
#include <conio.h>

#include "Network_Func.h"

// 메뉴
// 선택한 메뉴의 번호 리턴
int Menu();

int _tmain()
{
	_tsetlocale(LC_ALL, _T("Korean"));

	// 네트워크 연결 (윈속 초기화, 커넥트 등..)
	bool check = Network_Init();
	if (check == false)
		return 0;

	// 연결에 성공하면 메뉴 표시하기
	while (1)
	{
		system("cls");
		fputs("--- 친구 관리 메인 메뉴 ---\n", stdout);

		// 1. 로그인에 대한 정보 표시
		LoginShow();

		// 2. 메뉴 표시
		int SelectNum = Menu();

		// 3. 메뉴 선택에 따라 패킷 처리
		// 뭔가 문제가 생기면 false가 리턴된다.
		bool check = PacketProc(SelectNum);
		if (check == false)
			break;

		// 4. 메뉴에 대해 잘 처리가 되었으면. Press Any Key... 표시
		fputs("Press Any Key...", stdout);
		_getch();

	}

	fputs("비정상 종료\n", stdout);

    return 0;
}

// 메뉴
// 선택한 메뉴의 번호 리턴
int Menu()
{
	int SeletNum = 0;

	fputs("1. 회원 추가\n", stdout);
	fputs("2. 로그인\n", stdout);
	fputs("3. 회원 목록\n", stdout);
	fputs("4. 친구 목록\n", stdout);
	fputs("5. 받은 친구요청\n", stdout);
	fputs("6. 보낸 친구요청\n", stdout);
	fputs("7. 친구요청 보내기\n", stdout);
	fputs("8. 보낸 친구요청 취소\n", stdout);
	fputs("9. 받은 친구요청 수락\n", stdout);
	fputs("10. 받은 친구요청 거절\n", stdout);
	fputs("11. 친구 끊기\n", stdout);
	fputs(":", stdout);
	scanf_s("%d", &SeletNum);

	return SeletNum;
}

