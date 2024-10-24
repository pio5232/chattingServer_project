#include "pch.h"
#include "TlsProfiler.h"
#include <crtdbg.h>
#include "CCrashDump.h"
#include <thread>
#include <chrono>
#include "EchoServer.h"
#include <conio.h>

using namespace C_Network;
EchoServer server(NetAddress(std::wstring(L"127.0.0.1"),ServerPort),3000);



int main()
{
	server.InitHandler();

	server.Begin();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	while (1)
	{
		if (_kbhit())
		{
			char c = _getch();
			if (c == 'q' || c == 'Q')
				break;
		}
	}

	server.End();
}
