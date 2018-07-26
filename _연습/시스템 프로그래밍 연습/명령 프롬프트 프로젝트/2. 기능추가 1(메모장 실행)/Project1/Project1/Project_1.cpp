#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <locale.h>
#include <windows.h>

#define STR_LEN 256
#define CMD_TOKEN_NUM 10

TCHAR ERROR_CMD[] = TEXT("'%s'은(는) 실행할 수 있는 프로그램이 아닙니다. \n");

int CmdProcessing();
TCHAR* StrLower(TCHAR*);

int _tmain(int argc, TCHAR* argv[])
{
	// 한글 입력을 가능케 하기 위해
	_tsetlocale(LC_ALL, TEXT("koeran"));

	DWORD isExit;
	while (1)
	{
		isExit = CmdProcessing();

		if (isExit)
		{
			_fputts(TEXT("명령어 처리를 종료합니다. \n"), stdout);
			break;
		}
	}
	return 0;
}

TCHAR cmdString[STR_LEN];
TCHAR cmdTokenList[CMD_TOKEN_NUM][STR_LEN];
TCHAR seps[] = TEXT(" ,\t\n");

int CmdProcessing()
{
	_fputts(TEXT("Best Command Prompt>> "), stdout);
	_getts_s(cmdString);
	TCHAR* token = _tcstok(cmdString, seps);
	int tokenNum = 0;
	while (token != NULL)
	{
		_tcscpy(cmdTokenList[tokenNum++], StrLower(token));
		token = _tcstok(NULL, seps);
	}

	if (!_tcscmp(cmdTokenList[0], TEXT("exit")))
		return TRUE;

	else if (!_tccmp(cmdTokenList[0], TEXT("추가되는 명령어1")))
	{
	}

	else if (!_tccmp(cmdTokenList[0], TEXT("추가되는 명령어2")))
	{
	}

	// 지정되지 않은 명령어를 입력하면, 프로세스를 생성한다.
	// cmdTokenList[0](입력된 명령어)에 따라 표준 검색경로에서 실행파일을 찾아 생성한다.
	else 
	{
		STARTUPINFO si = { 0, };
		PROCESS_INFORMATION pi;
		si.cb = sizeof(si);

		BOOL isRun = CreateProcess(NULL, cmdTokenList[0], NULL, NULL,
			TRUE, 0, NULL, NULL, &si, &pi);

		if(isRun == FALSE)
			_tprintf(ERROR_CMD, cmdTokenList[0]);
	}

	return 0;
}

TCHAR* StrLower(TCHAR *pStr)
{
	TCHAR *ret = pStr;

	while (*pStr)
	{
		if (_istupper(*pStr))
			*pStr = _totlower(*pStr);

		pStr++;
	}

	return ret;
}