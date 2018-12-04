// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "ArrayList.h"

using namespace std;

int main()
{
	// 리스트 생성 및 초기화
	ArrayList list;
	list.ListInit(100);

	// 데이터 3개 저장
	NameCard* Name;

	Name = CreateNameCard("song", "1234");
	list.ListInsert(Name);

	Name = CreateNameCard("jin", "4567");
	list.ListInsert(Name);

	Name = CreateNameCard("gyu", "8910");
	list.ListInsert(Name);


	// "jin"유저 검색 후 정보 출력
	cout << "\"Jin\" Search --> ";
	if (list.ListFirst(&Name))
	{
		if(Name->NameCompare("jin") == 0)
			Name->ShowInfo();

		else
		{

			while (list.ListNext(&Name))
			{
				if (Name->NameCompare("jin") == 0)
				{
					Name->ShowInfo();
					break;
				}
			}
		}
	}
	cout << endl;

	// jin의 전화번호 변경
	cout << "\"Jin\" Phone Num Change --> ";
	if (list.ListFirst(&Name))
	{
		if (Name->NameCompare("jin") == 0)
		{
			Name->ChangePhoneNum("0000");
			cout << "JinChangeOk" << endl;
		}

		else
		{

			while (list.ListNext(&Name))
			{
				if (Name->NameCompare("jin") == 0)
				{
					Name->ChangePhoneNum("0000");
					cout << "JinChangeOk" << endl;
					break;
				}
			}
		}
	}
	cout << endl;	

	// SONG 삭제	
	cout << "\"song\" delete --> ";
	if (list.ListFirst(&Name))
	{
		if (Name->NameCompare("song") == 0)
		{
			list.ListRemove(&Name);
			cout << "deleteOK" << endl;

			delete Name;			
		}

		else
		{
			while (list.ListNext(&Name))
			{
				if (Name->NameCompare("song") == 0)
				{
					list.ListRemove(&Name);	
					cout << "deleteOK" << endl;

					delete Name;					
					break;
				}
			}
		}
	}
	cout << endl;

	// 전부 출력
	cout << "List Count : " << list.ListCount() << endl;
	if (list.ListFirst(&Name))
	{
		Name->ShowInfo();

		while (list.ListNext(&Name))
		{
			Name->ShowInfo();
		}
	}

	return 0;
}
