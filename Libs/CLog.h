#define _CRT_SECURE_NO_WARNINGS 

#ifndef PRINT_LOG
#define PRINT_LOG

#include <iostream>
//#include <fstream>
//#include <format>
//#include <condition_variable>

namespace C_Utility
{
#define dfLOG_LEVEL_DEBUG 0
#define dfLOG_LEVEL_ERROR 1
#define dfLOG_LEVEL_SYSTEM 2


#define _LOG(LogLevel, fmt, ...)					\
	do{													\
		if( g_iLogLevel <= LogLevel)					\
		{												\
			wsprintf(g_szLogBuff, fmt, ##__VA_ARGS__);	\
			C_Utility::Log(g_szLogBuff,LogLevel);					\
		}												\
	}while (0)		

	static int g_iLogLevel = dfLOG_LEVEL_ERROR; // ��� ���� ����� �α� ����
	static WCHAR g_szLogBuff[1024]; // �α� ����� �Ƿ��� �ӽ� ����

	void Log(WCHAR* szString, int iLogLevel);
	
	class FileLogger
	{
	public:
		FileLogger() 
		{
			/*InitializeSRWLock(&_bufferLock);
			
			time_t t = std::time(nullptr);
			tm tm = *std::localtime(&t);
	
			_fileName = std::format("{}_{}_{} {}:{}:{}", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
			
			std::fstream fileStream(_fileName);*/
		}

	//	void WriteLog(std::string content)
	//	{
	//		// TODO : CV ���� ������� ����.
	//		{
	//			SRWLockGuard lockGuard(&_bufferLock);

	//			_logBuffer.push(content);
	//		}
	//		_cv.notify_one();
	//	
	//		// TODO : Condition -> OpenFile..Write
	//	}

		void EndLogging()
		{
			//_endFlag = true;
			//_cv.notify_one();
		}
		void Save()
		{
	//		std::string content = "";
	//		while (_endFlag)
	//		{
	//			// check cv �߰� �ʿ�
	//			
	//			std::fstream fileStream(_fileName, std::ios::app);

	//			while (1)
	//			{	
	//				{
	//					SRWLockGuard lockGuard(&_bufferLock);

	//					if (_logBuffer.empty())
	//						break;

	//					content = _logBuffer.front();
	//					_logBuffer.pop();
	//					
	//				}
	//				fileStream << content;
	//			}
	//		}
		}

	//private:
	//	SRWLOCK _bufferLock;
	//	std::string _fileName;
	//	volatile bool _endFlag = false;
	//	std::queue<std::string> _logBuffer;
	//	std::condition_variable _cv;
	};
}
#endif
