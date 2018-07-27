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
bool LoadFile_UTF16(const TCHAR* FileName, TCHAR** pJson);

int main()
{
	_tsetlocale(LC_ALL, _T("Korean"));

	stUser* UserSave[2];
	TCHAR* tpJson = nullptr;

	// 파일에서 데이터 읽어오기
	if(LoadFile_UTF16(_T("json_test.txt"), &tpJson) == false)
		return 0;

	if (tpJson == nullptr)
	{
		fputs("LoadFile() 함수에서 읽어온 제이슨 파일이 nullptr.\n", stdout);
		return 0;
	}

	// Json데이터 파싱하기 (이미 UTF-16을 넣는중)
	GenericDocument<UTF16<>> Doc;
	Doc.Parse(tpJson);

	UINT64 AccountNo;
	const TCHAR* NickName;

	// AccountArray에 Account관련 데이터 넣기
	GenericValue<UTF16<>> &AccountArray = Doc[_T("Account")];
	SizeType Size = AccountArray.Size();

	// AccountArray에서 내 구조체로, Key값을 이용해, Value 가져오기
	for (SizeType i = 0; i < Size; ++i)
	{
		// 데이터 빼오기
		GenericValue<UTF16<>> &AccountObject = AccountArray[i];
		AccountNo = AccountObject[_T("AccountNo")].GetInt64();
		NickName = AccountObject[_T("NickName")].GetString();
		
		// 빼온 데이터를 내 구조체에 셋팅
		stUser* NewUser = new stUser;
		NewUser->m_AccountID = AccountNo;
		_tcscpy_s(NewUser->m_NickName, NICK_MAX_LEN, NickName);

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
bool LoadFile_UTF16(const TCHAR* FileName, TCHAR** pJson)
{
	FILE* fp;			// 파일 스트림
	size_t iFileCheck;	// 파일 오픈 여부 체크, 파일의 사이즈 저장. 총 2개의 옹도

	// 파일 열기
	iFileCheck = _tfopen_s(&fp, FileName, _T("rt, ccs=UTF-16LE"));
	if (iFileCheck != 0)
	{
		fputs("fopen 에러발생!\n", stdout);
		return false;
	}

	// 파일 읽어오기 위해 파일 크기만큼 동적할당
	fseek(fp, 0, SEEK_END);
	iFileCheck = ftell(fp);
	fseek(fp, 0, SEEK_SET);	

	// BOM코드 가져오기. (2바이트)
	// UTF-16LE의 BOM은 HxD로 볼때는 FF FE이다.
	// 근데, 읽어오면 리틀엔디안으로 저장되기 때문에, BOMCheck에는 FE FF가 들어있다.
	TCHAR BOMCheck;
	if (fread_s(&BOMCheck, sizeof(TCHAR), 2, 1, fp) != 1)
	{
		fputs("fread_s BOM코드 읽어오다 문제생김!\n", stdout);
		return false;
	}

	// BOM코드 존재하지 않는다면, 파일지시자를 다시 처음으로 보내서, 처음부터 다시 읽어와야한다.
	if (BOMCheck != 0xfeff)
		fseek(fp, 0, SEEK_SET);

	// BOM코드가 존재한다면, 이미 그만큼 이동한 것이니, BOM만큼(2바이트)을 버리기 위해 iFileCheck 재조정
	else
		iFileCheck -= 2; 

	*pJson = new TCHAR[iFileCheck];

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