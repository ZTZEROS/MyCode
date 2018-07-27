// SendData.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <Ws2tcpip.h>

#define SERVERDOMAIN	"procademyserver.iptime.org"
//#define SERVERPORT		12347 
#define SERVERPORT		10099 

struct st_PACKET_HEADER
{
	DWORD	dwPacketCode;		// 0x11223344	우리의 패킷확인 고정값

	WCHAR	szName[32];			// 본인이름, 유니코드 NULL 문자 끝
	WCHAR	szFileName[128];	// 파일이름, 유니코드 NULL 문자 끝
	int	iFileSize;
} sMyHeader;

BYTE* bImgBuf;

BOOL ImgRead(DWORD* dSize);

int main()
{	
	DWORD dCheck;
	DWORD dSize;
	BOOL bCheck;

	// --------------------------------
	// 이미지 읽어오기
	// --------------------------------
	bCheck = ImgRead(&dSize);
	if (!bCheck)
		return 1;

	// --------------------------------
	// 윈속 초기화
	// --------------------------------
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("WSAStartup fail\n");
		return 1;
	}

	// --------------------------------
	// 도메인으로 서버 ip 알아오기
	// --------------------------------
	// 도메인으로 IP 알아오기
	ADDRINFOW* pAddrInfo;
	SOCKADDR_IN* SockAddr;	

	dCheck = GetAddrInfo(_T(SERVERDOMAIN), _T("0"), NULL, &pAddrInfo);
	if (dCheck != 0)
	{
		int errorcode = WSAGetLastError();
		_tprintf(_T("GetAddrInfo Error: %d\n"), errorcode);
		return 1;
	}
	SockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;
	FreeAddrInfo(pAddrInfo);

	// --------------------------------
	// 서버와 연결하기
	// --------------------------------
	//내 헤더 세팅
	sMyHeader.dwPacketCode = 0x11223344;													// 0x11223344	우리의 패킷확인 고정값
	_tcscpy_s(sMyHeader.szName, _countof(sMyHeader.szName), _T("송진규"));					// 본인이름, 유니코드 NULL 문자 끝		
	_tcscpy_s(sMyHeader.szFileName, _countof(sMyHeader.szFileName), _T("SongFIle.bmp"));	// 파일이름, 유니코드 NULL 문자 끝						
	sMyHeader.iFileSize = dSize;												// 파일 사이즈

	//socket()
	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);	// TCP로 연결
	if(sock == INVALID_SOCKET)
	{
		int errorcode = WSAGetLastError();
		_tprintf(_T("socket() Error: %d\n"), errorcode);
		return 1;
	}

	//connect()
	/*TCHAR ServerIP[20] = _T("192.168.10.16");
	InetPton(AF_INET, ServerIP, &SockAddr->sin_addr.s_addr);*/
	SockAddr->sin_port = htons(SERVERPORT);	
	dCheck = connect(sock, (SOCKADDR*)SockAddr, sizeof(SOCKADDR_IN));
	if(dCheck == SOCKET_ERROR)
	{
		int errorcode = WSAGetLastError();
		_tprintf(_T("connect() Error: %d\n"), errorcode);
		return 1;
	}

	// 헤더 send	
	dCheck = send(sock, (const char*)&sMyHeader, sizeof(sMyHeader), 0);
	if (dCheck == SOCKET_ERROR)
	{
		int errorcode = WSAGetLastError();
		_tprintf(_T("header send() Error: %d\n"), errorcode);
		return 1;
	}

	_tprintf(_T("header send() ok\n"));

	//데이터 send
	BYTE MyBuff[1000];
	int iSize = sMyHeader.iFileSize;
	BYTE* save = bImgBuf;

	while (1)
	{
		// 가야 할 데이터가 1000 이상이라면
		if (iSize > 1000)
		{
			memcpy(&MyBuff, bImgBuf, 1000);
			dCheck = send(sock, (const char*)MyBuff, 1000, 0);
			if (dCheck == SOCKET_ERROR)
			{
				int errorcode = WSAGetLastError();
				_tprintf(_T("data send()_1 Error: %d\n"), errorcode);
				return 1;
			}

			iSize -= dCheck;
			bImgBuf += dCheck;
		}
		
		// 가야 할 데이터가 1000 이하라면, 마지막 send() 이다.
		else
		{
			memcpy(&MyBuff, bImgBuf, iSize);
			dCheck = send(sock, (const char*)MyBuff, iSize, 0);
			if (dCheck == SOCKET_ERROR)
			{
				int errorcode = WSAGetLastError();
				_tprintf(_T("data send()_2 Error: %d\n"), errorcode);
				return 1;
			}
			break;
		}

	}
	bImgBuf = save;

	_tprintf(_T("data send() ok\n"));

	delete bImgBuf;
	closesocket(sock);	
	WSACleanup();

    return 0;
}

BOOL ImgRead(DWORD* dSize)
{
	FILE *readFp;
	DWORD dCheck;
	BITMAPFILEHEADER hFileHeader;

	// --------------------------------
	// 이미지 읽어오기
	// --------------------------------
	// 이미지 전체 크기 알기위해 hFileHeader 읽어오기
	dCheck = fopen_s(&readFp, "Map.bmp", "rb");
	if (dCheck != NULL)
	{
		printf("fopen_s fail\n");
		return FALSE;
	}

	dCheck = fread_s(&hFileHeader, sizeof(hFileHeader), sizeof(hFileHeader), 1, readFp);
	if (dCheck != 1)
	{
		printf("hFileHeader_read fail\n");
		return FALSE;
	}
	fseek(readFp, 0, SEEK_SET);
	

	// *dSize 만큼 데이터 읽어오기
	*dSize = hFileHeader.bfSize;
	bImgBuf = new BYTE[*dSize];

	dCheck = fread_s(bImgBuf, *dSize, *dSize, 1, readFp);
	if (dCheck != 1)
	{
		printf("bImgBuf fail\n");
		return FALSE;
	}
	fclose(readFp);

	return TRUE;

}