#include "../../Common/Common.h"
#include "../../Common/protocol.h"
#include <iostream>
#include <fstream>
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
};
std::array<SESSION, MAX_USER> clients;

// 클라이언트와 통신하는 스레드
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int client_id = -1;

	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	int retval;
	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		//test
		Recv_Again:
		PACKET_INFO recv_info;
		retval = recv(client_sock, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK을 사용하여 수신버퍼를 읽지만 가져오지는 않도록
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0) {
			goto Recv_Again;
		}

		switch (recv_info.type) {
		case C2LS_REGISTER: {
			C2LS_REGISTER_PACKET register_pack;
			retval = recv(client_sock, (char*)&register_pack, sizeof(C2LS_REGISTER_PACKET), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}

			// 유저 이름이 담긴 파일 open
			std::ofstream fout("userlist.txt");
			if (fout.fail()) {
				std::cerr << "userlist.txt 를 찾을 수 없습니다." << std::endl;
				exit(100);
			}

			// 클라이언트로부터 받은 이름 추가
			fout << register_pack.name << std::endl;
			fout.close();
			std::cout << "계정 추가 완료" << std::endl;

			// 계정 등록 결과 전송
			LS2C_REGISTER_PACKET result_pack;
			result_pack.size = sizeof(LS2C_REGISTER_PACKET);
			result_pack.type = LS2C_REGISTER;
			result_pack.result = true;

			retval = send(client_sock, (char*)&result_pack, sizeof(LS2C_REGISTER_PACKET), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
			}

			break;
		}
		case C2LS_LOGIN:
			C2LS_LOGIN_PACKET login_pack;
			retval = recv(client_sock, (char*)&login_pack, sizeof(C2LS_LOGIN_PACKET), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}

			// id 할당
			bool approval = true;
			for (int i = 0; i < MAX_USER; i++) {
				if (clients[i].getState() == SESSION_EMPTY) {
					client_id = i;

					clients[i].setState(SESSION_RUNNING);
					clients[i].setId(client_id);
					break;
				}

				if (i == MAX_USER - 1 && clients[i].getState() == SESSION_RUNNING) {
					std::cout << "Max Users Exceeded!" << std::endl;
					approval = false;
				}
			}

			clients[client_id].setName(login_pack.name);
			std::cout << "Clients[" << clients[client_id].getId() << "]'s Name: " << clients[client_id].getName() << std::endl;

			LS2C_GAMESTART_PACKET start_pack;
			start_pack.size = sizeof(LS2C_GAMESTART_PACKET);
			start_pack.type = LS2C_GAMESTART;
			if (approval)
				start_pack.start = START_APPROVAL;
			else
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
	serveraddr.sin_port = htons(LOGIN_SERVER_PORT);
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