// NetServer_Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "NetworkLib\NetworkLib_NetServer.h"

#include <time.h>

using namespace Library_Jingyu;

class CChatServer	:public CNetServer
{	

public:
	virtual bool OnConnectionRequest(TCHAR* IP, USHORT port);
	
	virtual void OnClientJoin(ULONGLONG ClinetID);
	
	virtual void OnClientLeave(ULONGLONG ClinetID);
	
	virtual void OnRecv(ULONGLONG ClinetID, CProtocolBuff_Net* Payload);
	
	virtual void OnSend(ULONGLONG ClinetID, DWORD SendSize);
	
	virtual void OnWorkerThreadBegin();

	virtual void OnWorkerThreadEnd();
	
	virtual void OnError(int error, const TCHAR* errorStr);

};

bool CChatServer::OnConnectionRequest(TCHAR* IP, USHORT port)
{
	return true;
}

void CChatServer::OnClientJoin(ULONGLONG ClinetID)
{}

void CChatServer::OnClientLeave(ULONGLONG ClinetID)
{}

void CChatServer::OnRecv(ULONGLONG ClinetID, CProtocolBuff_Net* Payload)
{
	// 1. 페이로드 사이즈 얻기
	int Size = Payload->GetUseSize();

	// 2. 그 사이즈만큼 데이터 memcpy
	char* Text = new char[Size];
	Payload->GetData(Text, Size);

	// 3. 에코 패킷 만들기. Buff안에는 페이로드만 있다.
	CProtocolBuff_Net* Buff = CProtocolBuff_Net::Alloc();
	Buff->PutData(Text, Size);

	delete[] Text;

	// 4. 에코 패킷을 SendBuff에 넣기.
	SendPacket(ClinetID, Buff);
}

void CChatServer::OnSend(ULONGLONG ClinetID, DWORD SendSize)
{}

void CChatServer::OnWorkerThreadBegin()
{}

void CChatServer::OnWorkerThreadEnd()
{}

void CChatServer::OnError(int error, const TCHAR* errorStr)
{}

#define IP	L"0.0.0.0"
#define PORT	6000

#define CREATE_WORKER_THREAD	4
#define ACTIVE_WORKER_THREAD	2
#define CREATE_ACCEPT_THREAD	1

int _tmain()
{
	srand((UINT)time(NULL));
	CChatServer ChatS;

	BYTE HeadCode = 0x11;
	BYTE XORCode_1 = 0xc9;
	BYTE XORCode_2 = 0x88;

	if (ChatS.Start(IP, PORT, CREATE_WORKER_THREAD, ACTIVE_WORKER_THREAD, CREATE_ACCEPT_THREAD, false, 100, HeadCode, XORCode_1, XORCode_2) == false)
	{
		printf("Server OpenFail...\n");
	}


	Sleep(INFINITE);

	return 0;
 
}
