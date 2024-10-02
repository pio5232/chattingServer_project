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
//using namespace C_Utility;
////C_Utility::CCrashDump dump;
//
//	const int alsize = 4;
//	const int ssssize = 8000; // 100 ~ 10000
//	const int threadCnt = 8;
//struct sts
//{
//	sts() {}
//	~sts() {}
//	char c[alsize]{};
//};
//int a = 1000;
//
//void CheckMyPool()
//{
//	{
//		std::thread t[threadCnt];
//
//		for (int j = 0; j < threadCnt; j++)
//		{
//			t[j] = std::thread([=]() {
//				int st = 0;
//				void** arr = new void* [ssssize];
//
//				auto start = std::chrono::high_resolution_clock::now();
//				while (st < a)
//				{
//					for (int i = 0; i < ssssize; i++)
//					{
//						arr[i] = static_cast<int*>(poolMgr.Alloc(sizeof(sts)));
//					}
//
//					for (int i = 0; i < ssssize; i++)
//					{
//						poolMgr.Free(arr[i]);
//					}
//
//					++st;
//				};
//
//				auto end = std::chrono::high_resolution_clock::now();
//				std::chrono::duration<double> elapsed = end - start;
//				printf("%d 스레드 %lf 초 경과\n", j, elapsed);
//				delete[] arr;
//				}
//			);
//		}
//
//		{
//			int st = 0;
//			void** arr = new void* [ssssize];
//			{
//				auto start = std::chrono::high_resolution_clock::now();
//				while (st < a)
//				{
//					for (int i = 0; i < ssssize; i++)
//					{
//						arr[i] = poolMgr.Alloc(sizeof(sts));
//					}
//
//					for (int i = 0; i < ssssize; i++)
//					{
//						poolMgr.Free(arr[i]);
//					}
//
//					++st;
//				}
//
//				auto end = std::chrono::high_resolution_clock::now();
//				std::chrono::duration<double> elapsed = end - start;
//				printf("main 스레드 %lf 초 경과\n", elapsed);
//			}
//			delete[] arr;
//		}
//		for (auto& th : t)
//		{
//			if (th.joinable())
//				th.join();
//		}
//	}
//}
//void CheckNew()
//{
//	std::thread t[threadCnt];
//
//	for (int j = 0; j < threadCnt; j++)
//	{
//		t[j] = std::thread([=]() {
//			std::cout << "thread 시작 =====================================\n";
//			int st = 0;
//			void** arr = new void* [ssssize];
//
//			auto start = std::chrono::high_resolution_clock::now();
//			while (st < a)
//			{
//				for (int i = 0; i < ssssize; i++)
//				{
//					arr[i] = new sts;
//				}
//
//				for (int i = 0; i < ssssize; i++)
//				{
//					delete (arr[i]);
//				}
//
//				++st;
//			};
//
//			auto end = std::chrono::high_resolution_clock::now();
//			std::chrono::duration<double> elapsed = end - start;
//			printf("%d 스레드 %lf 초 경과\n", j, elapsed);
//			delete[] arr;
//			}
//		);
//	}
//
//	{
//		int st = 0;
//		void** arr = new void* [ssssize];
//		{
//			auto start = std::chrono::high_resolution_clock::now();
//			while (st < a)
//			{
//				for (int i = 0; i < ssssize; i++)
//				{
//					arr[i] = new sts;
//				}
//
//				for (int i = 0; i < ssssize; i++)
//				{
//					delete (arr[i]);
//				}
//
//				++st;
//			}
//
//			auto end = std::chrono::high_resolution_clock::now();
//			std::chrono::duration<double> elapsed = end - start;
//			printf("new - main 스레드 %lf 초 경과\n", elapsed);
//		}
//		delete[] arr;
//
//		for (auto& th : t)
//		{
//			if (th.joinable())
//				th.join();
//		}
//	}
//}
//int main()
//{
//	std::cout << "main 시작 ======================================\n";
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//
//	CheckMyPool();
//
//	std::cout << "============================================\n";
//
//	CheckNew();
//}
