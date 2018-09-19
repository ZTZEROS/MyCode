// PDH_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"

#include <Pdh.h>
#pragma comment(lib, "Pdh.lib")

#include <strsafe.h>


// 쿼리 스트링
/*

L"\\Process(이름)\\Thread Count"		// 스레드
L"\\Process(이름)\\Working Set"		// 사용 메모리

L"\\Memory\\Available MBytes"		// 사용가능

L"\\Memory\\Pool Nonpaged Bytes"	// 논페이지드

L"\\Network Interface(*)\\Bytes Received/sec"
L"\\Network Interface(*)\\Bytes Sent/sec"

*/

int _tmain()
{
	int a;
	a = 10;

	// 맴버 보관용
	PDH_HQUERY	_pdh_Query;

	// 측정 데이터 별 카운터 핸들 변수 준비
	PDH_HCOUNTER	_pdh_Counter_Handle_GameServer;
	PDH_HCOUNTER	_pdh_Counter_Handle_ChatServer;
	PDH_HCOUNTER	_pdh_Counter_NonPaged;

	// 측정 데이터의 저장용 변수
	double		_pdh_value_Handle_GameServer;
	double		_pdh_value_Handle_ChatServer;

	// ---------- 쿼리 오픈
	if (PdhOpenQuery(NULL, NULL, &_pdh_Query) != ERROR_SUCCESS)
	{
		DWORD Error = GetLastError();
		return 0;
	}

	// ---------- 카운터 등록
	if(PdhAddCounter(_pdh_Query, L"\\Memory\\Pool Nonpaged Bytes", NULL, &_pdh_Counter_NonPaged) != ERROR_SUCCESS)
	{
		DWORD Error = GetLastError();
		return 0;
	}

	// ---------- 데이터 갱신
	PdhCollectQueryData(_pdh_Query);

	// ---------- 데이터 뽑기.
	PDH_STATUS Status;

	PDH_FMT_COUNTERVALUE CounterValue;

	Status = PdhGetFormattedCounterValue(_pdh_Counter_NonPaged, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
	{
		//double 맴버에 저장 = CounterValue.doubleValue;
	}




	// PDH 카운터 등록.	
	//-PdhAddCounter(_pdh_Query, 쿼리스트링, NULL, &카운터);

	//PdhAddCounter(_pdh_Query, L"\\Process(이름)\\Handle Count", NULL, &_pdh_Counter_Handle_GameServer);
	//

	//// 데이터 갱신.
	//PdhCollectQueryData(_pdh_Query);

	//// 데이터 뽑기.
	//PDH_STATUS Status;
	//PDH_FMT_COUNTERVALUE CounterValue;

	//Status = PdhGetFormattedCounterValue(카운터 핸들, PDH_FMT_DOUBLE, NULL, &CounterValue);
	//if (Status == 0)
	//	double 맴버에 저장 = CounterValue.doubleValue;

	return 0;

}

