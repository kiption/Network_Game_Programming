#pragma once
#include "../Server/Common/Common.h"
#include "../Server/Common/protocol.h"

char* SERVERIP = (char*)"127.0.0.1";

bool g_registerd = false;
bool g_gamestart = false;

SOCKET sock_forLS;
SOCKET sock_forGS;

HANDLE h_thread_event_LS = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE h_thread_event_GS = CreateEvent(NULL, FALSE, FALSE, NULL);

//==================================================
// Networking Thread Function (Client - Login Server)
//==================================================
DWORD WINAPI Network_WithLS_ThreadFunc(LPVOID arg)
{
	int retval;

	retval = WaitForSingleObject(h_thread_event_LS, INFINITE);	// ��ȣ ���°� �ɶ����� ���
	if (retval != WAIT_OBJECT_0) return -1;

	// ���� ���� �� connect
	sock_forLS = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_forLS == INVALID_SOCKET)
		err_quit("socket()");

	struct sockaddr_in login_server_addr;
	memset(&login_server_addr, 0, sizeof(login_server_addr));
	login_server_addr.sin_family = AF_INET;
	login_server_addr.sin_addr.s_addr = inet_addr(SERVERIP);
	login_server_addr.sin_port = htons(LOGIN_SERVER_PORT);
	retval = connect(sock_forLS, (struct sockaddr*)&login_server_addr, sizeof(login_server_addr));
	if (retval == SOCKET_ERROR)
		err_quit("connect()");


	// ������ ��ϵ� ������ �ִ��� �����.
	char registered;			// ��Ͽ���
	char input_name[NAME_LEN];	// �����̸� 
	while (1) {
		ASK_ALREADY_REGISTERED:
		cout << "�̹� ��ϵ� ������ �ֳ���? [��: Y | �ƴϿ�: N]: ";
		cin >> registered;

		if (registered == 'Y' || registered == 'y') {
			// �α��� �õ�
		LOGIN_NAME_INPUT_AGAIN:
			char temp[100];
			cout << "�̸��� �Է����ּ���: ";
			cin >> temp;

			if (strlen(temp) > NAME_LEN)
				cout << "�̸��� �ִ� " << NAME_LEN << "�ڸ� �ʰ��� �� �����ϴ�.\n" << endl;
			else
				strcpy(input_name, temp);

			// �α��� ������ �α��� ��û
			C2LS_LOGIN_PACKET login_packet;
			login_packet.size = sizeof(C2LS_LOGIN_PACKET);
			login_packet.type = C2LS_LOGIN;
			strcpy_s(login_packet.name, input_name);
			retval = send(sock_forLS, (char*)&login_packet, sizeof(C2LS_LOGIN_PACKET), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
			}

			// �����κ��� �α��� ���� ��Ŷ �ޱ�
			PACKET_INFO recv_info;
			retval = recv(sock_forLS, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK�� ����Ͽ� ���Ź��ۿ��� �������� �ʵ��� ��.
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
			}

			if (recv_info.type == LS2C_GAMESTART) {
				LS2C_GAMESTART_PACKET start_pack;

				retval = recv(sock_forLS, (char*)&start_pack, sizeof(LS2C_GAMESTART_PACKET), MSG_WAITALL);
				if (retval == SOCKET_ERROR) {
					err_display("recv()");
				}

				if (start_pack.start == START_APPROVAL) {
					cout << "�α����� �㰡�Ǿ����ϴ�.\n" << endl;
					g_gamestart = true;

					closesocket(sock_forLS);
					return 0;
				}
				else if (start_pack.start == START_DENY_UNKNOWNNAME) {
					cout << "��ϵ��� ���� �̸��Դϴ�. ��� �� �ٽ� �α����� �õ����ּ���.\n" << endl;
				}
				else if (start_pack.start == START_DENY_FULL) {
					cout << "���� ������ ��ȭ�����Դϴ�. ��� �� �ٽ� �õ��� �ּ���.\n" << endl;
				}
				else {
					cout << "�� �� ���� ������ �α��� ��û�� �����Ǿ����ϴ�. �ٽ� ������ �Է����ּ���.\n" << endl;
				}
			}
		}
		else if (registered == 'N' || registered == 'n') {
			// ���� ��� ����
		REGISTERED_NAME_INPUT_AGAIN:
			char temp_name[100];
			cout << "�̸��� �Է����ּ���: ";
			cin >> temp_name;

			if (strlen(temp_name) <= NAME_LEN) {
				strcpy(input_name, temp_name);
			}
			else {
				cout << "�̸��� �ִ� " << NAME_LEN << "�ڸ� �ʰ��� �� �����ϴ�.\n" << endl;
				goto REGISTERED_NAME_INPUT_AGAIN;	// �ٽ� �Է¹޵���.
			}

			// �α��� ������ ���� ��� ��û
			C2LS_REGISTER_PACKET register_pack;
			register_pack.size = sizeof(C2LS_REGISTER_PACKET);
			register_pack.type = C2LS_REGISTER;
			strcpy(register_pack.name, input_name);
			retval = send(sock_forLS, (char*)&register_pack, sizeof(C2LS_REGISTER_PACKET), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
			}

			// �����κ��� ���� ��ϿϷ� ��Ŷ �ޱ�
			PACKET_INFO recv_info;
			retval = recv(sock_forLS, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK�� ����Ͽ� ���Ź��ۿ��� �������� �ʵ��� ��.
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
			}

			if (recv_info.type == LS2C_REGISTER) {
				LS2C_REGISTER_PACKET register_result;
				retval = recv(sock_forLS, (char*)&register_result, sizeof(LS2C_REGISTER_PACKET), MSG_WAITALL);
				if (retval == SOCKET_ERROR) {
					err_display("recv()");
				}

				if (register_result.result) {
					cout << "������ ���������� ��ϵǾ����ϴ�.\n" << endl;
					g_registerd = true;
				}
				else {
					cout << "�̹� �����ϴ� �̸��Դϴ�. �ٸ� �̸��� �Է����ּ���.\n" << endl;
					goto REGISTERED_NAME_INPUT_AGAIN;
				}
			}
		}
		else {
			// �߸��� �� �Է�
			cout << "Y �Ǵ� N ���θ� �Է����ּ���." << endl;
		}
	}

	return 0;
}

//==================================================
// Networking Thread Function (Client - Game Server)
//==================================================
DWORD WINAPI Network_WithGS_ThreadFunc(LPVOID arg)
{
	int retval;

	retval = WaitForSingleObject(h_thread_event_GS, INFINITE);	// ��ȣ ���°� �ɶ����� ���
	if (retval != WAIT_OBJECT_0) return -1;

	// ���� ���� �� connect
	SOCKET GS_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (GS_sock == INVALID_SOCKET) err_quit("socket()");

	struct sockaddr_in gameserver_addr;
	memset(&gameserver_addr, 0, sizeof(gameserver_addr));
	gameserver_addr.sin_family = AF_INET;
	gameserver_addr.sin_addr.s_addr = inet_addr(SERVERIP);
	gameserver_addr.sin_port = htons(GAME_SERVER_PORT);
	retval = connect(GS_sock, (struct sockaddr*)&gameserver_addr, sizeof(gameserver_addr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// Recv
	while (1) {
	GS_RECV_AGAIN:
		PACKET_INFO recv_info;

		retval = recv(GS_sock, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK�� ����Ͽ� ���Ź��۸� ������ ���������� �ʵ���
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}
		else if (retval == 0) {
			goto GS_RECV_AGAIN;
		}

		switch (recv_info.type) {
		default:
			break;
		}
	}
	//====

	return 0;
}