
#ifndef CSerialization
#define CSerialization


namespace C_Utility
{
#define CSERIALIZATION_DEFAULT_SIZE 100
#define CSERIALIZATION_MAX_SIZE 1600
	class CSerializationBuffer
	{
	public:
		CSerializationBuffer(int iBufferSize = CSERIALIZATION_DEFAULT_SIZE);
		~CSerializationBuffer();
		// ��Ŷ û��
		void Clear();

		// ��������.
		bool Resize();

		//���� ������ ���
		char* GetBufferPtr() const;

		// ���� ������ ���
		int GetBufferSize() const;

		// ���� ������� ������ ���
		int GetDataSize() const;

		int GetFreeSize() const;

		int GetData(char* chpDest, int iSize);
		int PutData(char* chpSrc, int iSrcSize);

		// ������ �����ε�
		CSerializationBuffer& operator<<(unsigned char ucValue);
		CSerializationBuffer& operator<<(char cValue);

		CSerializationBuffer& operator<<(unsigned short usValue);
		CSerializationBuffer& operator<<(short shValue);

		CSerializationBuffer& operator<<(unsigned int uiValue);
		CSerializationBuffer& operator<<(int iValue);

		CSerializationBuffer& operator<<(unsigned long ulValue);
		CSerializationBuffer& operator<<(long lValue);

		//CSerializationBuffer& operator<< (long long llValue);
		//CSerializationBuffer& operator<< (unsigned long long ullValue);

		CSerializationBuffer& operator<< (float fValue);

		CSerializationBuffer& operator<< (__int64 llValue);
		CSerializationBuffer& operator<< (double dValue);

		CSerializationBuffer& operator>>(unsigned char& ucValue);
		CSerializationBuffer& operator>>(char& cValue);

		CSerializationBuffer& operator>>(unsigned short& usValue);
		CSerializationBuffer& operator>>(short& shValue);

		CSerializationBuffer& operator>>(unsigned int& uiValue);
		CSerializationBuffer& operator>>(int& iValue);

		CSerializationBuffer& operator>>(unsigned long& ulValue);
		CSerializationBuffer& operator>>(long& lValue);
		
		CSerializationBuffer& operator>> (float& fValue);

		CSerializationBuffer& operator>> (__int64& llValue);
		CSerializationBuffer& operator>> (double& dValue);

		//CSerializationBuffer& operator>> (long long llValue);
		//CSerializationBuffer& operator>> (unsigned long long ullValue);

		// ���� ������ ���
		char* GetRearPtr() const;
		char* GetFrontPtr() const;
		// ���� Pos �̵�
		int MoveRearPos(int iSize);
		int MoveFrontPos(int iSize);
	private:

	public:
		int m_iBufferCapacity;
		int m_iDataSize;
		int m_iFront;
		int m_iRear;
		char* m_chpBuffer;
	};
}



#endif // !CSerialization


// ------------- ���纻 (��� ���� ����� �����ϱ� ���� ����)
#ifdef A_PREV
#ifndef CSerialization
#define CSerialization

#ifndef CSerialization_EXPORT
#define CSerialization_DLLSPEC __declspec(dllexport)
#else
#define CSerialization_DLLSPEC __declspec(dllimport)
#endif

namespace C_Utility
{
#define CSERIALIZATION_DEFAULT_SIZE 100
#define CSERIALIZATION_MAX_SIZE 1600
	class CSerialization_DLLSPEC CSerializationBuffer
	{
	public:
		CSerializationBuffer(int iBufferSize = CSERIALIZATION_DEFAULT_SIZE);
		~CSerializationBuffer();

		// ��Ŷ û��
		void Clear();

		// ��������.
		bool Resize();

		//���� ������ ���
		char* GetBufferPtr() const;

		// ���� ������ ���
		int GetBufferSize() const;

		// ���� ������� ������ ���
		int GetDataSize() const;

		int GetFreeSize() const;

		int GetData(char* chpDest, int iSize);
		int PutData(char* chpSrc, int iSrcSize);

		// ������ �����ε�
		CSerializationBuffer& operator<<(unsigned char ucValue);
		CSerializationBuffer& operator<<(char cValue);

		CSerializationBuffer& operator<<(unsigned short usValue);
		CSerializationBuffer& operator<<(short shValue);

		CSerializationBuffer& operator<<(unsigned int uiValue);
		CSerializationBuffer& operator<<(int iValue);

		CSerializationBuffer& operator<<(unsigned long ulValue);
		CSerializationBuffer& operator<<(long lValue);

		CSerializationBuffer& operator<< (float fValue);

		CSerializationBuffer& operator<< (__int64 llValue);
		CSerializationBuffer& operator<< (double dValue);

		CSerializationBuffer& operator>>(unsigned char& ucValue);
		CSerializationBuffer& operator>>(char& cValue);

		CSerializationBuffer& operator>>(unsigned short& usValue);
		CSerializationBuffer& operator>>(short& shValue);

		CSerializationBuffer& operator>>(unsigned int& uiValue);
		CSerializationBuffer& operator>>(int& iValue);

		CSerializationBuffer& operator>>(unsigned long& ulValue);
		CSerializationBuffer& operator>>(long& lValue);

		CSerializationBuffer& operator>> (float& fValue);

		CSerializationBuffer& operator>> (__int64& llValue);
		CSerializationBuffer& operator>> (double& dValue);

		// ���� ������ ���
		char* GetRearPtr() const;
		char* GetFrontPtr() const;
		// ���� Pos �̵�
		int MoveRearPos(int iSize);
		int MoveFrontPos(int iSize);
	private:

	public:
		char* m_chpBuffer;
		int m_iBufferCapacity;
		int m_iDataSize;
		int m_iFront;
		int m_iRear;
	};
}



#endif // !CSerialization
#endif