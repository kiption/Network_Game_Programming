#pragma once
#include "../Server/Common/Common.h"
#include "ObjINFO.h"

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
	sock_forGS = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_forGS == INVALID_SOCKET)
		err_quit("socket()");

	struct sockaddr_in game_server_addr;
	memset(&game_server_addr, 0, sizeof(game_server_addr));
	game_server_addr.sin_family = AF_INET;
	game_server_addr.sin_addr.s_addr = inet_addr(SERVERIP);
	game_server_addr.sin_port = htons(GAME_SERVER_PORT);
	retval = connect(sock_forGS, (struct sockaddr*)&game_server_addr, sizeof(game_server_addr));
	if (retval == SOCKET_ERROR)
		err_quit("connect()");

	// Recv
	while (1) {
		PACKET_INFO recv_info;
		retval = recv(sock_forGS, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK�� ����Ͽ� ���Ź��۸� ������ ���������� �ʵ���
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}
		else if (retval == 0) {
			break;
		}

		switch (recv_info.type) {
		case GS2C_LOGIN_INFO:
		{
			GS2C_LOGIN_INFO_PACKET login_info_pack;
			retval = recv(sock_forGS, (char*)&login_info_pack, sizeof(GS2C_LOGIN_INFO_PACKET), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
			}

			// �����κ��� �Ҵ���� id
			myID = login_info_pack.id;

			// Ŭ�� �� ��ü���� �����̳ʿ� ������ ���� ���� ���� �����մϴ�.
			players_info[myID].m_id = myID;
			players_info[myID].m_pos = { login_info_pack.pos_x, login_info_pack.pos_y, login_info_pack.pos_z };
			players_info[myID].m_right_vec = { login_info_pack.right_vec_x, login_info_pack.right_vec_y, login_info_pack.right_vec_z };
			players_info[myID].m_up_vec = { login_info_pack.up_vec_x, login_info_pack.up_vec_y, login_info_pack.up_vec_z };
			players_info[myID].m_look_vec = { login_info_pack.look_vec_x, login_info_pack.look_vec_y, login_info_pack.look_vec_z };
			players_info[myID].m_state = OBJ_ST_RUNNING;

			// Test Log
			cout << "[Recv MyInfo] ID: " << players_info[myID].m_id
				<< ", Pos: (" << players_info[myID].m_pos.x << ", " << players_info[myID].m_pos.y << ", " << players_info[myID].m_pos.z << ")"
				<< ", LookVec: (" << players_info[myID].m_look_vec.x << ", " << players_info[myID].m_look_vec.y << ", " << players_info[myID].m_look_vec.z << ")" << endl;

			break;
		}
		case GS2C_ADD_OBJ:
		{
			GS2C_ADD_OBJ_PACKET add_obj_pack;
			retval = recv(sock_forGS, (char*)&add_obj_pack, sizeof(GS2C_ADD_OBJ_PACKET), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
			}
			if (add_obj_pack.id == myID) break;

			// �߰��Ǵ� ��ü�� id
			int new_id = add_obj_pack.id;

			// Ŭ�� �� ��ü���� �����̳ʿ� ������ ���� ���� ���� �����մϴ�.
			players_info[new_id].m_id = new_id;
			players_info[new_id].m_pos = { add_obj_pack.pos_x, add_obj_pack.pos_y, add_obj_pack.pos_z };
			players_info[new_id].m_right_vec = { add_obj_pack.right_vec_x, add_obj_pack.right_vec_y, add_obj_pack.right_vec_z };
			players_info[new_id].m_up_vec = { add_obj_pack.up_vec_x, add_obj_pack.up_vec_y, add_obj_pack.up_vec_z };
			players_info[new_id].m_look_vec = { add_obj_pack.look_vec_x, add_obj_pack.look_vec_y, add_obj_pack.look_vec_z };
			players_info[new_id].m_state = OBJ_ST_RUNNING;

			// Test Log
			cout << "[Recv New Client's Info] ID: " << players_info[new_id].m_id
				<< ", Pos: (" << players_info[new_id].m_pos.x << ", " << players_info[new_id].m_pos.y << ", " << players_info[new_id].m_pos.z << ")"
				<< ", LookVec: (" << players_info[new_id].m_look_vec.x << ", " << players_info[new_id].m_look_vec.y << ", " << players_info[new_id].m_look_vec.z << ")" << endl;

			break;
		}
		case GS2C_UPDATE:
		{
			GS2C_UPDATE_PACKET update_pack;
			retval = recv(sock_forGS, (char*)&update_pack, sizeof(GS2C_UPDATE_PACKET), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
			}

			if (update_pack.id == myID) {	// �ڽ��� �̵�
				players_info[myID].m_pos = { update_pack.pos_x, update_pack.pos_y, update_pack.pos_z };

				players_info[myID].m_right_vec = { update_pack.right_vec_x, update_pack.right_vec_y, update_pack.right_vec_z };
				players_info[myID].m_up_vec = { update_pack.up_vec_x, update_pack.up_vec_y, update_pack.up_vec_z };
				players_info[myID].m_look_vec = { update_pack.look_vec_x, update_pack.look_vec_y, update_pack.look_vec_z };

				// Test Log
				//cout << "My Car moves to Pos(" << players_info[myID].m_pos.x << ", " << players_info[myID].m_pos.y << ", " << players_info[myID].m_pos.z << ")." << endl;
			}
			else if (update_pack.id >= 0 && update_pack.id <= MAX_USER) {	// �ٸ� �÷��̾��� �̵�
				players_info[update_pack.id].m_pos = { update_pack.pos_x, update_pack.pos_y, update_pack.pos_z };

				players_info[update_pack.id].m_right_vec = { update_pack.right_vec_x, update_pack.right_vec_y, update_pack.right_vec_z };
				players_info[update_pack.id].m_up_vec = { update_pack.up_vec_x, update_pack.up_vec_y, update_pack.up_vec_z };
				players_info[update_pack.id].m_look_vec = { update_pack.look_vec_x, update_pack.look_vec_y, update_pack.look_vec_z };
			}

			break;
		}
		default:
			break;
		}
	}
	//====

	return 0;
}