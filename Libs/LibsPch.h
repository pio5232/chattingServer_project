#pragma once

#include <iostream>

#include <WinSock2.h>
#include <Windows.h>
#include <thread>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

#include "Define.h"

#include "Utils.h"
#include "NetworkUtils.h"

#include "Session.h"

class LOGGGG
{
public:
	void WriteLog(std::string s)
	{
		log += s + "\n";
	}
	
public:
	static std::string log;
};
inline void CCrash(const WCHAR* resaon)
{
	wprintf(L"%s\n", resaon);

	int* bp = nullptr;
	*bp = (int)0x12345678;
}
