#include "pch.h"
#include "MasterServer/MasterServer.h"

using namespace Library_Jingyu;

int _tmain()
{
	CMatchServer_Lan MServer;

	if (MServer.ServerStart() == false)
		return 0;

	while (1)
	{
		Sleep(1000);

		// 화면 출력

	}

	return 0;   
}
