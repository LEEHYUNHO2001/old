#include<iostream>
#include"WS2tcpip.h"

//Winsock�� ����ϴ� ���ø����̼��� ws2_32,lib�� ��ũ����
#pragma comment(lib, "ws2_32.lib")

//ǥ�� ���̺귯���� ���� ����ϴ� ��쿡 using ���
using std::cout;
using std::cerr;
using std::endl;

//�ڽŸ��� �ڷ��� ����, ePort��� ���ο� ������(enum)�� ����
enum ePort { SERVER_PORT = 54000 };

int main() {
	//1.Winsock ���� ���̺귯��(dll) �ʱ�ȭ

	//winsock �ʱ�ȭ ���� ����ü
	WSADATA wsaData;

	//winsock �ʱ�ȭ
	//WSAStartup(winsock����, �ʱ�ȭ�� ���� �����ϴ� ����) ���� ����2.2
	//MAKEWORD(2, 2) -> 2���� �μ��� WORD�� pack�ϴ� ��ũ��
	int iniResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	//winsock �ʱ�ȭ ���� ����
	if (iniResult != 0) {
		cerr << "Can't Initialize winsock! Quiting" << endl;
		return -1;
	}

	//2.listening socket ����
	//����� ����ϱ����� �ʱ⿡ ������ ���ϰ� �� ���� ������ ���� �վ���
	//����� ����� ������ �ƴϰ�, �游 �մ� �� 3���� socket(������ ������)
	//AF_INET : IPv4�� ���
	//SOCK_STREAN : ���������� TCP/IP�������� ���
	//IPPROTO_TCP : TCP�������� ���
	SOCKET listeningSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//listening socket ���� ���� ����
	if (listeningSock == INVALID_SOCKET) {
		cerr << "Can't create a socket! Quitting" << endl;
		WSACleanup();
		return -1;
	}

	//3.socket�� IP�ּҿ� PORT��ȣ�� Bind(�ѽ����� ����)
	//�ü���� � ������ Ư�� �ּҿ� ���� ���� ��Ʈ�� ���ڴٴ°��� �˷��ִ� ����

	//sockaddr����ü�� IP�ּҿ� port��ȣ�� ���� �����ϴ°��� �����Ͽ� sockaddr_in ����ü ���
	sockaddr_in hint{};
	hint.sin_family = AF_INET;
	//htonl, htons : �ʿ��� �����Ͱ� big endian�ε� little endian�� �� �־ ���
	//INADDR_ANY -> ������ �����ϴ� ��ǻ���� IP�ּҰ� �ڵ����� �Ҵ�, ��� NIC�� IP�ּҿ� ���ε���
	hint.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//Ŭ���̾�Ʈ�� �����ϰ� �� PORT��ȣ �ֱ�
	hint.sin_port = htons(SERVER_PORT);

	//ù ���� - ���ε� �� ���� : listeningsock
	//�ι�° ���� - ���Ͽ� ���ε� �� IP�ּ� �� ��Ʈ��ȣ�� sockaddrŸ���� �ּҷ� ����
	//����° ���� - �� ��° ���ڷ� �ѱ� sockaddr Ÿ���� ũ��
	int bindResult = bind(listeningSock, reinterpret_cast<sockaddr*>(&hint), sizeof(hint));

	//bind ���� ����
	if (bindResult == SOCKET_ERROR) {
		cerr << "Can't bind a socket! Quitting" << endl;
		closesocket(listeningSock);
		WSACleanup();
		return -1;
	}

	//4.listen : ������ ������ accept�� �� �ִ� ���°� �ǵ����ϱ�
	//�����׿� �� ������ �ܺο��� ������ Ŭ���̾�Ʈ ��û ���� �� �ְԵ�
	//SOMAXCONN : Ŀ���� ������ ������ ���� ������ Ŭ���̾�Ʈ�� ��û��
	//			  connercion queue�� �����ϴµ�, ���⿡ ����� �� �ִ� ��û�� ��
	int listenResult = listen(listeningSock, SOMAXCONN);
	//listen ���� ����
	if (listenResult == SOCKET_ERROR) {
		cerr << "Can't listen a socket! Quitting" << endl;
		WSACleanup();
		return -1;
	}

	//5.Ŭ���̾�Ʈ�� ���� ��û�� ������ accept�Լ��� ���� ���� ����
	sockaddr_in clientSockInfo;
	int clientSize = sizeof(clientSockInfo);

	//ù ���� : ������ ����� ���� -> �� ������ ���� ������ Ŭ���̾�Ʈ�� ��û�� accept
	//			accept�� �����ϸ� �ι�° ���忡 �ּҸ� ����
	//���������� Ŭ���̾�Ʈ���� ��ſ� ����� �� �ִ� ����clientSocket ���� �� ��ȯ
	//listeningSock�� ������ �������. �ݾƵ� �̹� ����� Ŭ���̾�Ʈ�� ��� �����ϹǷ� �ݾ��ֱ�(6��)
	SOCKET clientSocket = accept(listeningSock, reinterpret_cast<sockaddr*>(&clientSockInfo), &clientSize);

	//clientSocket ���� ���� ����
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Can't accept a socket! Quitting" << endl;
		closesocket(listeningSock);
		WSACleanup();
		return -1;
	}

	//6.listeningSock �ݱ�(��� �ٸ� client�� ���� �����Ÿ� ��������)
	int closeResult = closesocket(listeningSock);

	//7.Ŭ���̾�Ʈ�� ��û�� �ް� ������ ������ ����
	char host[NI_MAXHOST];	         // Ŭ���̾�Ʈ�� host �̸�
	char service[NI_MAXHOST];        // Ŭ���̾�Ʈ�� PORT ��ȣ
	ZeroMemory(host, NI_MAXHOST);    // memset(host, 0, NI_MAXHOST)�� ����
	ZeroMemory(service, NI_MAXHOST);

	// clientSockInfo�� ����� IP �ּҸ� ���� ������ ������ ����ϴ�. host �̸��� host��, ��Ʈ ��ȣ�� service�� ����˴ϴ�.
	// getnameinfo()�� ���� �� 0�� ��ȯ�մϴ�. ���� �� 0�� �ƴ� ���� ��ȯ�մϴ�.
	if (getnameinfo((sockaddr*)&clientSockInfo, sizeof(clientSockInfo), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cout << host << " connected ON port " << service << endl;
	}
	else
	{
		inet_ntop(AF_INET, &clientSockInfo.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " << ntohs(clientSockInfo.sin_port) << endl;
	}


	// While loop: Ŭ���̾�Ʈ�� �޼����� �޾Ƽ� ��� �� Ŭ���̾�Ʈ�� �ٽ� �����ϴ�.
	enum eBufSize { BUF_SIZE = 4096 };
	char buf[BUF_SIZE];

	while (true)
	{
		ZeroMemory(buf, BUF_SIZE);

		// Wait for client to send data
		// �޼����� ���������� ������ recv �Լ��� �޼����� ũ�⸦ ��ȯ�Ѵ�.
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