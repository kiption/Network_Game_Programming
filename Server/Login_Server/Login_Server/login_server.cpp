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

// Ŭ���̾�Ʈ�� ����ϴ� ������
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int client_id = -1;

	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	int retval;
	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		//test
		PACKET_INFO recv_info;
		retval = recv(client_sock, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK�� ����Ͽ� ���Ź��۸� ������ ���������� �ʵ���
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAECONNRESET) {
				// ���� ���� ��ó��
				clients[client_id].clearSession();

				// ���� �ݱ�
				closesocket(client_sock);
				std::cout << "[TCP ����] Ŭ���̾�Ʈ ������ ����: IP �ּ�= " << addr << ", ��Ʈ ��ȣ = " << ntohs(clientaddr.sin_port) << std::endl;
				return 0;
			}
			else {
				err_display("recv()");
			}
			break;
		}
		else if (retval == 0) {
			// ���� �ݱ�
			closesocket(client_sock);
			std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� ����: IP �ּ�= " << addr << ", ��Ʈ ��ȣ = " << ntohs(clientaddr.sin_port) << std::endl;
			return 0;
		}

		switch (recv_info.type) {
		case C2LS_REGISTER: {
			C2LS_REGISTER_PACKET register_pack;
			retval = recv(client_sock, (char*)&register_pack, sizeof(C2LS_REGISTER_PACKET), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}

			// �ߺ� üũ
			std::ifstream fin("userlist.txt");
			if (fin.fail()) {
				std::cerr << "userlist.txt �� ã�� �� �����ϴ�." << std::endl;
				exit(100);
			}

			bool usable_name = true;
			std::string saved_name;
			std::string recved_name = register_pack.name;
			while (fin >> saved_name) {
				std::cout << "Saved name: " << saved_name << std::endl;
				if (saved_name.compare(recved_name) == 0) {
					usable_name = false;
					break;
				}
			}

			// ��밡���� �̸��̶��, �Է¹��� �̸��� ���Ӱ� ����մϴ�.
			if (usable_name) {
				// ���� �̸��� ��� ���� open
				std::ofstream fout("userlist.txt", std::ios::app);
				if (fout.fail()) {
					std::cerr << "userlist.txt �� ã�� �� �����ϴ�." << std::endl;
					exit(100);
				}

				// Ŭ���̾�Ʈ�κ��� ���� �̸� �߰�
				fout << register_pack.name << "\n";
				fout.close();
				std::cout << "���� �߰� �Ϸ�" << std::endl;
			}

			// ���� ��� ��� ����
			LS2C_REGISTER_PACKET result_pack;
			result_pack.type = LS2C_REGISTER;
			if (usable_name)
				result_pack.result = true;
			else
				result_pack.result = false;

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

			// ���� �̸��� ��� ���� open
			std::ifstream fin("userlist.txt");
			if (fin.fail()) {
				std::cerr << "userlist.txt �� ã�� �� �����ϴ�." << std::endl;
				exit(100);
			}

			// Ŭ���̾�Ʈ�κ��� ���� �̸��� ��밡���� �̸����� Ȯ���մϴ�.
			std::string recved_name = login_pack.name;
			// 1. �̹� ���� ���� �ٸ� Ŭ���̾�Ʈ�� �̸����� Ȯ��.
			bool name_already_used = false;
			for (int i = 0; i < MAX_USER; i++) {
				if (clients[i].getName().compare(recved_name) == 0) {
					name_already_used = true;
					break;
				}
			}

			// 2. ���� ���� ���������Ͽ� �����ϴ� �� Ȯ��.
			bool name_exist = false;
			std::string saved_name;
			//std::string recved_name = login_pack.name;
			while (fin >> saved_name) {
				std::cout << "Saved name: " << saved_name << std::endl;
				if (saved_name.compare(recved_name) == 0) {
					std::cout << "���� [" << login_pack.name << "]�� Ȯ�εǾ����ϴ�." << std::endl;
					name_exist = true;
					break;
				}
			}
			fin.close();

			bool approval = true;
			if (!name_exist) {
				std::cout << "�������� �ʴ� �����Դϴ�. ��� �� �ٽ� �õ����ּ���." << std::endl;
			}
			else if (name_already_used) {
				std::cout << "�Է��� ���� [" << login_pack.name << "]�� �̹� �ٸ� �÷��̾ ��� ���Դϴ�." << std::endl;
			}
			else {
				// id �Ҵ�
				for (int i = 0; i < MAX_USER; i++) {
					if (clients[i].getState() == SESSION_EMPTY) {
						client_id = i;

						clients[client_id].setState(SESSION_RUNNING);
						clients[client_id].setId(client_id);
						clients[client_id].setName(login_pack.name);
						std::cout << "Clients[" << clients[client_id].getId() << "]'s Name: " << clients[client_id].getName() << std::endl;
						break;
					}

					if (i == MAX_USER - 1 && clients[i].getState() == SESSION_RUNNING) {
						std::cout << "Max Users Exceeded!" << std::endl;
						approval = false;
					}
				}
			}

			LS2C_GAMESTART_PACKET start_pack;
			start_pack.type = LS2C_GAMESTART;
			if (approval && name_exist && !name_already_used)
				start_pack.start = START_APPROVAL;
			else if (!name_exist)	// �Էµ� ���� �̸��� �������� �ʴ� �̸��϶�
				start_pack.start = START_DENY_UNKNOWNNAME;
			else if (!approval)		// ������ ��ȭ������ ��
				start_pack.start = START_DENY_FULL;
			else if (name_already_used)
				start_pack.start = START_DENY_ALREADYUSED;
			
			std::cout << "Packet Size: " << sizeof(start_pack) << std::endl;
			
			retval = send(client_sock, (char*)&start_pack, sizeof(LS2C_GAMESTART_PACKET), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
			}

			if (approval && name_exist)	break;
		}

	}

	// Session Clear
	short curr_state = clients[client_id].getState();
	if (curr_state == SESSION_RUNNING)
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
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", addr, ntohs(clientaddr.sin_port));

		// ������ ����
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