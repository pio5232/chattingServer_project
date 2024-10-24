---------------
목표.
1. NetworkBase / NetServer / NetClient 완성
2. iocp를 활용한 메시지 처리 로직 완성
3. Player와 Echo 서버에 대한 작동 테스트
4. 채팅 서버 및 프로젝트 완성.
5. 테스트.

----------------
할 일.
WorkerThread 내부 함수에서 메시지를 어떻게 처리할 것인지.
Session에 대한 설정을 어떻게 할 것인지. 처리부분 생각하고 만들기.

IocpObjBase -> Dispatch 구현 및 Session 틀 잡고 Echo 서버까지 만들어서 테스트해보도록 한다.


-----------------
connect / discconect / accept 모두 비동기로 동작하도록 해보자. 구현 자체는 복잡하지만, 이해도 해볼 수 있고, 서버는 고정으로 메모리를 할당해놓고 사용하는 것이 좋다 -> 라는 취지에도 맞는 것 같음. 그리고 이렇게 비동기로 설정해놓으면 내 생각엔 io 자체에 대한 로직을 통일할 수 있음. (accept / connect / disconnect에 대한 로직을 따로 설정할 필요가 없으며, accept 담당 스레드도 만들지 않아도 된다.

일단 accpept / connect / disconnect(x)로 만들어서 테스트해보자.

또한 Session <-> Server의 순환 참조를 끊을 수 있는 방법을 생각해보자..

지금 모든 데이터는 Pool로 사용하고 있는데, 처음 만들 때는 new로 만들었다가 new로 어느정도 실행이 잘 되면, Pool로 바꾸는 과정을 테스트해보자.


connet / accept 

--------------------------------------------------

Send 어떻게 할 것인지에 대해서 생각해보고, EchoServer의 On~ 로직에 대한 구현 생각하기, Packet 핸들링 만들기.


ChatServer -> 내부의 PacketHandler로 전달, MakePacket<T>에서 타입에 따른 직렬화 처리 후 Send도 할 수 있도록하고,
현재 EchoServer의 경우 그냥 Recv하는 방식으로도 만들어보고, PacketHandler를 통해 처리하는 것도 만들어보자.
