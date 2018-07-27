// JsonTest.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"
#include "Header.h"

using namespace rapidjson;

// 파일에서 데이터 읽어오는 함수
// pJson에 읽어온 데이터가 저장된다.
bool LoadFile_UTF8(const char* FileName, char** pJson);

// UTF-8을 UTF-16으로 변경
bool UTF8toUTF16(const char* szText, TCHAR* szBuff, int iBuffLen);

int main()
{
	_tsetlocale(LC_ALL, _T("Korean"));

	stUser* UserSave[2];
	char* pJson = nullptr;

	// 파일에서 데이터 읽어오기
	if(LoadFile_UTF8("json_test.txt", &pJson) == false)
		return 0;

	if (pJson == nullptr)
	{
		fputs("LoadFile() 함수에서 읽어온 제이슨 파일이 nullptr.\n", stdout);
		return 0;
	}

	// Json데이터 파싱하기 (UTF-8 -> UTF-16)
	Document Doc;
	Doc.Parse(pJson);

	UINT64 AccountNo;
	TCHAR NickName[NICK_MAX_LEN];	

	// AccountArray에 Account관련 데이터 넣기
	Value &AccountArray = Doc["Account"];
	SizeType Size = AccountArray.Size();

	// AccountArray에서 내 구조체로, Key값을 이용해, Value 가져오기
	for (SizeType i = 0; i < Size; ++i)
	{
		// 데이터 빼오기
		Value &AccountObject = AccountArray[i];
		AccountNo = AccountObject["AccountNo"].GetInt64();
		UTF8toUTF16(AccountObject["NickName"].GetString(), NickName, NICK_MAX_LEN);

		// 빼온 데이터를 내 구조체에 셋팅
		stUser* NewUser = new stUser;
		NewUser->m_AccountID = AccountNo;
		_tcscpy_s(NewUser->m_NickName, _countof(NickName), NickName);
		
		// 내 구조체를 관리하는 구조체 포인터 배열에 추가
		UserSave[i] = NewUser;
	}

	// 셋팅한 데이터 출력하기
	for (SizeType i = 0; i < Size; ++i)
		_tprintf(_T("ID : %llu, 닉네임 : %s\n"), UserSave[i]->m_AccountID, UserSave[i]->m_NickName);

    return 0;
}

// 파일에서 데이터 읽어오는 함수
// pJson에 읽어온 데이터가 저장된다.
bool LoadFile_UTF8(const char* FileName, char** pJson)
{
	// UTF-8 파일 읽어오기
	FILE* fp;			// 파일 스트림
	size_t iFileCheck;	// 파일 오픈 여부 체크, 파일의 사이즈 저장. 총 2개의 옹도

	// 파일 열기
	iFileCheck = _tfopen_s(&fp, _T("json_test.txt"), _T("rt"));
	//iFileCheck = fopen_s(&fp, FileName, "rb, ccs=UTF-8");
	if (iFileCheck != 0)
	{
		fputs("fopen 에러발생!\n", stdout);
		return false;
	}

	// 파일 읽어오기 위해 파일 크기만큼 동적할당
	fseek(fp, 0, SEEK_END);
	iFileCheck = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	*pJson = new char[iFileCheck];

	// 파일에서 데이터 읽어오기.
	size_t iSize = fread_s(*pJson, iFileCheck, 1, iFileCheck, fp);
	if (iSize != iFileCheck)
	{
		fputs("fread_s 에러발생!\n", stdout);
		return false;
	}

	fclose(fp);

	return true;
}

// UTF-8을 UTF-16으로 변경
bool UTF8toUTF16(const char* szText, TCHAR* szBuff, int iBuffLen)
{
	int iRe = MultiByteToWideChar(CP_UTF8, 0, szText, (int)strlen(szText), szBuff, iBuffLen);
	if (iRe < iBuffLen)
		szBuff[iRe] = L'\0';
	return true;
}
