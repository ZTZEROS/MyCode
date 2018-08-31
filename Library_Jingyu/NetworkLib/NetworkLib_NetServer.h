/*
락프리 적용된 Network Library
*/


#ifndef __NETWORK_LIB_NETSERVER_H__
#define __NETWORK_LIB_NETSERVER_H__

#include <windows.h>
#include "ProtocolBuff\ProtocolBuff(Net)_ObjectPool.h"
#include "LockFree_Stack\LockFree_Stack.h"

using namespace std;



namespace Library_Jingyu
{
	// --------------
	// CNetServer 클래스는, 내부 서버 간 통신에 사용된다.
	// 내부 서버간 통신은, 접속 받는쪽을 서버 / 접속하는 쪽을 클라로 나눠서 생각한다 (개념적으로)
	// 그 중 서버 부분이다.
	// --------------
	class CNetServer
	{
	private:

		// ----------------------
		// private 구조체 or enum 전방선언
		// ----------------------
		// 에러 enum값들
		enum class euError : int;

		// Session구조체 전방선언
		struct stSession;

		// ----------------------
		// private 변수들
		// ----------------------

		// Net서버의 Config들.
		BYTE m_bCode;
		BYTE m_bXORCode_1;
		BYTE m_bXORCode_2;

		// 리슨소켓
		SOCKET m_soListen_sock;

		// 워커, 엑셉트 스레드 핸들 배열
		HANDLE*	m_hAcceptHandle;
		HANDLE* m_hWorkerHandle;

		// IOCP 핸들
		HANDLE m_hIOCPHandle;

		// 워커스레드 수, 엑셉트 스레드 수
		int m_iA_ThreadCount;
		int m_iW_ThreadCount;


		// ----- 세션 관리용 -------
		// 세션 관리 배열
		stSession* m_stSessionArray;

		// 미사용 인덱스 관리 스택
		CLF_Stack<ULONGLONG>* m_stEmptyIndexStack;

		// --------------------------

		// 윈도우 에러 보관 변수, 내가 지정한 에러 보관 변수
		int m_iOSErrorCode;
		euError m_iMyErrorCode;

		// 최대 접속 가능 유저 수
		int m_iMaxJoinUser;

		// 현재 접속중인 유저 수
		ULONGLONG m_ullJoinUserCount;

		// 서버 가동 여부. true면 작동중 / false면 작동중 아님
		bool m_bServerLife;


	private:
		// ----------------------
		// private 함수들
		// ----------------------	
		// SendPacket, Disconnect 등 외부에서 호출하는 함수에서, 락거는 함수.
		// 실제 락은 아니지만 락처럼 사용.
		//
		// Parameter : SessionID
		// return : 성공적으로 세션 찾았을 시, 해당 세션 포인터
		//			실패 시 nullptr
		stSession* GetSessionLOCK(ULONGLONG SessionID);

		// SendPacket, Disconnect 등 외부에서 호출하는 함수에서, 락 해제하는 함수
		// 실제 락은 아니지만 락처럼 사용.
		//
		// parameter : 세션 포인터
		// return : 성공적으로 해제 시, true
		//		  : I/O카운트가 0이되어 삭제된 유저는, false
		bool GetSessionUnLOCK(stSession* NowSession);

		// 워커 스레드
		static UINT	WINAPI	WorkerThread(LPVOID lParam);

		// Accept 스레드
		static UINT	WINAPI	AcceptThread(LPVOID lParam);

		// 중간에 무언가 에러났을때 호출하는 함수
		// 1. 윈속 해제
		// 2. 입출력 완료포트 핸들 반환
		// 3. 워커스레드 종료, 핸들반환, 핸들 배열 동적해제
		// 4. 리슨소켓 닫기
		void ExitFunc(int w_ThreadCount);

		// RecvProc 함수. 큐의 내용 체크 후 PacketProc으로 넘긴다.
		void RecvProc(stSession* NowSession);

		// RecvPost함수
		//
		// return true : 성공적으로 WSARecv() 완료 or 어쨋든 종료된 유저는 아님
		// return false : I/O카운트가 0이되어서 종료된 유저임
		bool RecvPost(stSession* NowSession);

		// SendPost함수
		//
		// return true : 성공적으로 WSASend() 완료 or 어쨋든 종료된 유저는 아님
		// return false : I/O카운트가 0이되어서 종료된 유저임
		bool SendPost(stSession* NowSession);

		// 내부에서 실제로 유저를 끊는 함수.
		void InDisconnect(stSession* NowSession);

		// 각종 변수들을 리셋시킨다.
		void Reset();


	public:
		// -----------------------
		// 생성자와 소멸자
		// -----------------------
		CNetServer();
		~CNetServer();

	public:
		// -----------------------
		// 외부에서 호출 가능한 함수
		// -----------------------

		// ----------------------------- 기능 함수들 ---------------------------

		// 서버 시작
		// [오픈 IP(바인딩 할 IP), 포트, 워커스레드 수, 활성화시킬 워커스레드 수, 엑셉트 스레드 수, TCP_NODELAY 사용 여부(true면 사용), 최대 접속자 수, 패킷 Code, XOR 1번코드, XOR 2번코드] 입력받음.
		//
		// return false : 에러 발생 시. 에러코드 셋팅 후 false 리턴
		// return true : 성공
		bool Start(const TCHAR* bindIP, USHORT port, int WorkerThreadCount, int ActiveWThreadCount, int AcceptThreadCount, bool Nodelay, int MaxConnect, 
					BYTE Code, BYTE XORCode1, BYTE XORCode2);

		// 서버 스탑.
		void Stop();

		// 지정한 유저를 끊을 때 호출하는 함수. 외부 에서 사용.
		// 라이브러리한테 끊어줘!라고 요청하는 것 뿐
		//
		// return true : 해당 유저에게 셧다운 잘 날림.
		// return false : 접속중이지 않은 유저를 접속해제하려고 함.
		bool Disconnect(ULONGLONG ClinetID);

		// 외부에서, 어떤 데이터를 보내고 싶을때 호출하는 함수.
		// SendPacket은 그냥 아무때나 하면 된다.
		// 해당 유저의 SendQ에 넣어뒀다가 때가 되면 보낸다.
		//
		// return true : SendQ에 성공적으로 데이터 넣음.
		// return true : SendQ에 데이터 넣기 실패.
		bool SendPacket(ULONGLONG ClinetID, CProtocolBuff_Net* payloadBuff);


		// ----------------------------- 게터 함수들 ---------------------------
		// 윈도우 에러 얻기
		int WinGetLastError();

		// 내 에러 얻기 
		int NetLibGetLastError();

		// 현재 접속자 수 얻기
		ULONGLONG GetClientCount();

		// 서버 가동상태 얻기
		// return true : 가동중
		// return false : 가동중 아님
		bool GetServerState();

		// 미사용 세션 관리 스택의 노드 얻기
		LONG GetStackNodeCount();

	public:
		// -----------------------
		// 순수 가상함수
		// -----------------------

		// Accept 직후, 호출된다.
		//
		// parameter : 접속한 유저의 IP, Port
		// return false : 클라이언트 접속 거부
		// return true : 접속 허용
		virtual bool OnConnectionRequest(TCHAR* IP, USHORT port) = 0;

		// 연결 후 호출되는 함수 (AcceptThread에서 Accept 후 호출)
		//
		// parameter : 접속한 유저에게 할당된 세션키
		// return : 없음
		virtual void OnClientJoin(ULONGLONG ClinetID) = 0;

		// 연결 종료 후 호출되는 함수 (InDIsconnect 안에서 호출)
		//
		// parameter : 유저 세션키
		// return : 없음
		virtual void OnClientLeave(ULONGLONG ClinetID) = 0;

		// 패킷 수신 완료 후 호출되는 함수.
		//
		// parameter : 유저 세션키, 받은 패킷
		// return : 없음
		virtual void OnRecv(ULONGLONG ClinetID, CProtocolBuff_Net* Payload) = 0;

		// 패킷 송신 완료 후 호출되는 함수
		//
		// parameter : 유저 세션키, Send 한 사이즈
		// return : 없음
		virtual void OnSend(ULONGLONG ClinetID, DWORD SendSize) = 0;

		// 워커 스레드가 깨어날 시 호출되는 함수.
		// GQCS 바로 하단에서 호출
		// 
		// parameter : 없음
		// return : 없음
		virtual void OnWorkerThreadBegin() = 0;

		// 워커 스레드가 잠들기 전 호출되는 함수
		// GQCS 바로 위에서 호출
		// 
		// parameter : 없음
		// return : 없음
		virtual void OnWorkerThreadEnd() = 0;

		// 에러 발생 시 호출되는 함수.
		//
		// parameter : 에러 코드(실제 윈도우 에러코드는 WinGetLastError() 함수로 얻기 가능. 없을 경우 0이 리턴됨)
		//			 : 에러 코드에 대한 스트링
		// return : 없음
		virtual void OnError(int error, const TCHAR* errorStr) = 0;

	};

}






#endif // !__NETWORK_LIB_NETSERVER_H__