#include "../../Common/Common.h"
#include "../../Common/protocol.h"
#include <iostream>
#include <array>
#include <string>

#define SERVERPORT 9000
#define BUFSIZE    512

enum { SESSION_EMPTY, SESSION_RUNNING };
class SESSION {
private:
	short		m_id;
	SOCKET		m_sock;
	short		m_state;
	std::string	m_name;

public:
	SESSION() {
		m_id = -1;
		memset(&m_sock, 0, sizeof(m_sock));
		m_state = SESSION_EMPTY;
		m_name = "None";
	}

public:
	// Accessor
	short getId() { return m_id; }
	SOCKET getSock() { return m_sock; }
	short getState() { return m_state; }
	std::string getName() { return m_name; }

	void setId(short id) { m_id = id; }
	void setSock(SOCKET sock) { m_sock = sock; }
	void setState(char state) { m_state = state; }
	void setName(char* name) { m_name = name; }

public:
	// Function
	void clearSession() {
		m_id = -1;
		memset(&m_sock, 0, sizeof(m_sock));
		m_state = SESSION_EMPTY;
		m_name = "None";
	}
	void sendPacket(void* packet);
};
std::array<SESSION, MAX_USER> clients;

// 클라이언트와 통신하는 스레드
DWORD WINAPI ProcessClient(LPVOID arg)
{
	// id 할당
	int client_id = -1;
	for (int i = 0; i < MAX_USER; i++) {
		if (clients[i].getState() == SESSION_EMPTY) {
			client_id = i;

			clients[i].setState(SESSION_RUNNING);
			clients[i].setId(client_id);
			break;
		}

		if (i == MAX_USER - 1 && clients[i].getState() == SESSION_RUNNING) {
			std::cout << "Max Users Exceeded!" << std::endl;
			return 0;
		}
	}

	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	clients[client_id].setSock(client_sock);	// 소켓 정보 저장

	int retval;
	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	int recved_size = 0;

	while (1) {
		//test
		PACKET_INFO recv_info;
		retval = recv(client_sock, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK을 사용하여 수신버퍼를 읽지만 가져오지는 않도록
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		std::cout << "packet's size: " << recv_info.size << std::endl;
		if (recv_info.type == C2LS_LOGIN)
			std::cout << "This packet is C2LS_LOGIN Packet" << std::endl;

		recved_size += sizeof(PACKET_INFO);

		switch (recv_info.type) {
		case C2LS_LOGIN:
			C2LS_LOGIN_PACKET login_pack;
			retval = recv(client_sock, (char*)&login_pack, sizeof(C2LS_LOGIN_PACKET), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			clients[client_id].setName(login_pack.name);
			std::cout << "Clients[" << clients[client_id].getId() << "]'s Name: " << clients[client_id].getName() << std::endl;

			LS2C_GAMESTART_PACKET start_pack;
			start_pack.size = sizeof(LS2C_GAMESTART_PACKET);
			start_pack.type = LS2C_GAMESTART;
			start_pack.start = START_DENY;
			std::cout << "Packet Size: " << sizeof(start_pack) << std::endl;
			
			retval = send(client_sock, (char*)&start_pack, sizeof(LS2C_GAMESTART_PACKET), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
			}

			break;
		}

	}

	if (clients[client_id].getState() == SESSION_RUNNING)
		clients[client_id].clearSession();

	closesocket(client_sock);
	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;

	while (1) {
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));

		// 스레드 생성
		HANDLE hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
		if (hThread == NULL)
			closesocket(client_sock);
		else
			CloseHandle(hThread);
	}

	closesocket(listen_sock);

	WSACleanup();
	return 0;
}