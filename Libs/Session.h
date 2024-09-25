#pragma once
namespace C_Network
{
	/*----------------------
			IocpObjBase
	----------------------*/
	class IocpObjBase : public std::enable_shared_from_this<IocpObjBase>
	{
	public:
		//virtual HANDLE GetHandle() abstract = 0;
		virtual void Dispatch(class IocpEvent* iocpEvent, int numOfBytes = 0) abstract = 0;
	};


	class Session : public IocpObjBase
	{
	public:
		void Dispatch(class IocpEvent* iocpEvent, int numOfBytes = 0);
	private:

	};

	/*----------------------
			IocpEvent 
	----------------------*/
	enum class IocpEventType : unsigned char
	{
		Connect,
		Disconnect,
		Accept,
		// PreRecv, 0 byte Recv
		Recv,
		Send,
	};

	class IocpEvent : public OVERLAPPED
	{
	public : 
		IocpEvent(IocpEventType type);
		void Init();

		SharedIocpBase _owner;
	
	private:
		IocpEventType _type;
	};


	class RecvEvent : public IocpEvent
	{
	public:
		RecvEvent() : IocpEvent(IocpEventType::Recv) {}
	};

	class SendEvent : public IocpEvent
	{
	public:
		SendEvent() : IocpEvent(IocpEventType::Send) {}
	};
}
