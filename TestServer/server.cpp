#include<iostream>
#include"WS2tcpip.h"

//Winsock을 사용하는 애플리케이션을 ws2_32,lib와 링크해줌
#pragma comment(lib, "ws2_32.lib")

//표준 라이브러리를 많이 사용하는 경우에 using 사용
using std::cout;
using std::cerr;
using std::endl;

//자신만의 자료형 생성, ePort라는 새로운 열거형(enum)을 정의
enum ePort { SERVER_PORT = 54000 };

int main() {
	//1.Winsock 동적 라이브러리(dll) 초기화

	//winsock 초기화 정보 구조체
	WSADATA wsaData;

	//winsock 초기화
	//WSAStartup(winsock버전, 초기화된 상태 저장하는 변수) 현재 버전2.2
	//MAKEWORD(2, 2) -> 2개의 인수를 WORD로 pack하는 매크로
	int iniResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	//winsock 초기화 오류 감지
	if (iniResult != 0) {
		cerr << "Can't Initialize winsock! Quiting" << endl;
		return -1;
	}

	//2.listening socket 생성
	//상대방과 통신하기위해 초기에 상대방의 소켓과 내 소켓 사이의 길을 뚫어줌
	//상대방과 연결될 소켓이 아니고, 길만 뚫는 제 3자의 socket(데이터 못보냄)
	//AF_INET : IPv4를 사용
	//SOCK_STREAN : 연결지향의 TCP/IP프로토콜 사용
	//IPPROTO_TCP : TCP프로토콜 사용
	SOCKET listeningSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//listening socket 생성 오류 감지
	if (listeningSock == INVALID_SOCKET) {
		cerr << "Can't create a socket! Quitting" << endl;
		WSACleanup();
		return -1;
	}

	//3.socket에 IP주소와 PORT번호를 Bind(한쌍으로 묶음)
	//운영체제에 어떤 소켓이 특정 주소와 전송 계층 포트를 쓰겠다는것을 알려주는 절차

	//sockaddr구조체에 IP주소와 port번호를 직접 저장하는것이 복잡하여 sockaddr_in 구조체 사용
	sockaddr_in hint{};
	hint.sin_family = AF_INET;
	//htonl, htons : 필요한 데이터가 big endian인데 little endian일 수 있어서 사용
	//INADDR_ANY -> 소켓이 동작하는 컴퓨터의 IP주소가 자동으로 할당, 모든 NIC의 IP주소에 바인딩함
	hint.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//클라이언트가 접속하게 될 PORT번호 주기
	hint.sin_port = htons(SERVER_PORT);

	//첫 인자 - 바인딩 될 소켓 : listeningsock
	//두번째 인자 - 소켓에 바인딩 할 IP주소 및 포트번호를 sockaddr타입의 주소로 받음
	//세번째 인자 - 두 번째 인자로 넘긴 sockaddr 타입의 크기
	int bindResult = bind(listeningSock, reinterpret_cast<sockaddr*>(&hint), sizeof(hint));

	//bind 오류 감지
	if (bindResult == SOCKET_ERROR) {
		cerr << "Can't bind a socket! Quitting" << endl;
		closesocket(listeningSock);
		WSACleanup();
		return -1;
	}

	//4.listen : 소켓이 연결을 accept할 수 있는 상태가 되도록하기
	//리스닝에 들어간 소켓은 외부에서 들어오는 클라이언트 요청 받을 수 있게됨
	//SOMAXCONN : 커널은 리스닝 소켓을 통해 들어오는 클라이언트의 요청을
	//			  connercion queue에 저장하는데, 여기에 대기할 수 있는 요청의 수
	int listenResult = listen(listeningSock, SOMAXCONN);
	//listen 오류 감지
	if (listenResult == SOCKET_ERROR) {
		cerr << "Can't listen a socket! Quitting" << endl;
		WSACleanup();
		return -1;
	}

	//5.클라이언트의 연결 요청이 들어오면 accept함수를 통해 연결 수락
	sockaddr_in clientSockInfo;
	int clientSize = sizeof(clientSockInfo);

	//첫 인자 : 리스닝 모드의 소켓 -> 이 소켓을 통해 들어오는 클라이언트의 요청을 accept
	//			accept가 성공하면 두번째 인장에 주소를 저장
	//내부적으로 클라이언트와의 통신에 사용할 수 있는 소켓clientSocket 생성 후 반환
	//listeningSock의 역할은 여기까지. 닫아도 이미 연결된 클라이언트와 통신 가능하므로 닫아주기(6번)
	SOCKET clientSocket = accept(listeningSock, reinterpret_cast<sockaddr*>(&clientSockInfo), &clientSize);

	//clientSocket 생성 오류 감지
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Can't accept a socket! Quitting" << endl;
		closesocket(listeningSock);
		WSACleanup();
		return -1;
	}

	//6.listeningSock 닫기(계속 다른 client의 연결 받을거면 닫지말자)
	int closeResult = closesocket(listeningSock);

	//7.클라이언트의 요청을 받고 수행할 동작을 구현
	char host[NI_MAXHOST];	         // 클라이언트의 host 이름
	char service[NI_MAXHOST];        // 클라이언트의 PORT 번호
	ZeroMemory(host, NI_MAXHOST);    // memset(host, 0, NI_MAXHOST)와 동일
	ZeroMemory(service, NI_MAXHOST);

	// clientSockInfo에 저장된 IP 주소를 통해 도메인 정보를 얻습니다. host 이름은 host에, 포트 번호는 service에 저장됩니다.
	// getnameinfo()는 성공 시 0을 반환합니다. 실패 시 0이 아닌 값을 반환합니다.
	if (getnameinfo((sockaddr*)&clientSockInfo, sizeof(clientSockInfo), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cout << host << " connected ON port " << service << endl;
	}
	else
	{
		inet_ntop(AF_INET, &clientSockInfo.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " << ntohs(clientSockInfo.sin_port) << endl;
	}


	// While loop: 클라이언트의 메세지를 받아서 출력 후 클라이언트에 다시 보냅니다.
	enum eBufSize { BUF_SIZE = 4096 };
	char buf[BUF_SIZE];

	while (true)
	{
		ZeroMemory(buf, BUF_SIZE);

		// Wait for client to send data
		// 메세지를 성공적으로 받으면 recv 함수는 메세지의 크기를 반환한다.
		int bytesReceived = recv(clientSocket, buf, BUF_SIZE, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "Error in recv(). Quitting" << endl;
			break;
		}

		if (bytesReceived == 0)
		{
			cout << "Client disconnected " << endl;
			break;
		}

		// Echo message back to client
		cout << buf << endl;
		send(clientSocket, buf, bytesReceived + 1, 0);
	}
}