#pragma once

#ifndef RINGBUFFER_EXPORT
#define RINGBUFFER_DLLSPEC //__declspec(dllexport)
#else
#define RINGBUFFER_DLLSPEC //__declspec(dllimport)
#endif

namespace C_Utility
{
#define RINGBUFFER_DEFAULT_SIZE 10000
	class RINGBUFFER_DLLSPEC CRingBuffer
	{
	public:
		CRingBuffer();
		CRingBuffer(int iCapacity);
		~CRingBuffer();
		bool Resize(int newCapacity);
		int GetCapacity(); // �� �뷮
		int GetUseSize(); // ��� �뷮
		int GetFreeSize(); // ���� �뷮

		int DirectEnqueueSize();
		int DirectDequeueSize();

		bool EnqueueRetBool(char* chpData, int iSize);
		bool DequeueRetBool(char* chpDest, int iSize);
		bool PeekRetBool(char* chpDest, int iSize);

		int Enqueue(char* chpData, int iSize);
		int Dequeue(char* chpDest, int iSize);
		int Peek(char* chpDest, int iSize);

		int MoveRear(int iSize);
		int MoveFront(int iSize);

		bool MoveRearRetBool(int iSize);
		bool MoveFrontRetBool(int iSize);

		void ClearBuffer(void);
		char* GetFrontBufferPtr();
		char* GetRearBufferPtr();
		char* GetStartBufferPtr();

		bool IsEmpty();
		bool IsFull();
	public:
		int m_iCapacity;
		int m_iSize;
		int m_front;
		int m_rear;
		int m_count = 0;
		char* m_pBuffer;
	};
}
//#define WSAPRINT(x) CGetLastError(L##x)

//bool CGetLastError(const WCHAR* funcName);

//bool CGetLastError(const WCHAR* funcName)
//{
//	bool bRet = false;
//	int getLastError = WSAGetLastError();
//
//	// A(getLastError); //���߿� A�ڵ� �ֱ�. 
//	if (getLastError)
//	{
//		wprintf(L"%s GetLastError : %d \n", funcName, getLastError);
//		bRet = true;
//	}
//	return bRet;
//}