#pragma once
#include "../Server/Common/Common.h"
#include "../Server/Common/protocol.h"

char* SERVERIP = (char*)"127.0.0.1";

bool g_gamestart = false;

HANDLE h_network_th = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE h_main_th = CreateEvent(NULL, FALSE, FALSE, NULL);

int retval;
char input_name[NAME_LEN];

DWORD WINAPI NetworkingThreadFunc(LPVOID arg)
{
	retval = WaitForSingleObject(h_network_th, INFINITE);	// ��ȣ ���°� �ɶ����� ���
	if (retval != WAIT_OBJECT_0) return -1;

	//==== Server
	SOCKET sock = (SOCKET)arg;

	// ���� ���� ����
	char registered;
	while (1) {
		cout << "�̹� ��ϵ� ������ �ֳ���? Y �Ǵ� N ���� �Է����ּ���: ";
		cin >> registered;

		if (!(registered == 'y' || registered == 'n' || registered == 'Y' || registered == 'N')) {
			cout << "Y �Ǵ� N ���θ� �Է����ּ���." << endl;
		}
		else {
			break;
		}
	}
	

	while (1) {
		char temp[100];
		cout << "�̸��� �Է����ּ���: ";
		cin >> temp;

		if (strlen(temp) > NAME_LEN) {
			cout << "�̸��� �ִ� " << NAME_LEN << "�ڸ� �ʰ��� �� �����ϴ�." << endl;
		}
		else {
			strcpy(input_name, temp);
			break;
		}
	}

	// ���� ��� ����
	if (registered == 'n' || registered == 'N') {
		C2LS_REGISTER_PACKET register_pack;
		register_pack.size = sizeof(C2LS_REGISTER_PACKET);
		register_pack.type = C2LS_REGISTER;
		strcpy(register_pack.name, input_name);
		retval = send(sock, (char*)&register_pack, sizeof(C2LS_REGISTER_PACKET), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
		}

		while (1) {
			int recv_bytes = 0;
			PACKET_INFO recv_info;
			while (recv_bytes < 1) {
				retval = recv(sock, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK�� ����Ͽ� ���Ź��۸� ������ ���������� �ʵ���
				if (retval == SOCKET_ERROR) {
					err_display("recv()");
				}
				recv_bytes = retval;

			}

			if (recv_info.type == LS2C_REGISTER) {
				LS2C_REGISTER_PACKET register_result;

				retval = recv(sock, (char*)&register_result, sizeof(LS2C_REGISTER_PACKET), MSG_WAITALL);
				if (retval == SOCKET_ERROR) {
					err_display("recv()");
					break;
				}

				if (register_result.result) {
					cout << "������ ���������� ��ϵǾ����ϴ�." << endl;
					break;
				}

			}
		}
	}
	
	// �α��� ����
	C2LS_LOGIN_PACKET login_packet;
	login_packet.size = sizeof(C2LS_LOGIN_PACKET);
	login_packet.type = C2LS_LOGIN;
	strcpy_s(login_packet.name, input_name);
	retval = send(sock, (char*)&login_packet, sizeof(C2LS_LOGIN_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}

	// Recv
	while (1) {
		int recv_bytes = 0;
		PACKET_INFO recv_info;
		while (recv_bytes < 1) {
			retval = recv(sock, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK�� ����Ͽ� ���Ź��۸� ������ ���������� �ʵ���
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
			}
			recv_bytes = retval;

		}

		switch (recv_info.type) {
		case LS2C_GAMESTART:
			LS2C_GAMESTART_PACKET start_pack;

			retval = recv(sock, (char*)&start_pack, sizeof(LS2C_GAMESTART_PACKET), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}

			if (start_pack.start == START_APPROVAL) {
				g_gamestart = true;
				SetEvent(h_main_th);
			}

			break;
		}
	}

	//====

	return 0;
}