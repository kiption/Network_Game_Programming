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

	retval = WaitForSingleObject(h_thread_event_LS, INFINITE);	// 신호 상태가 될때까지 대기
	if (retval != WAIT_OBJECT_0) return -1;

	// 소켓 생성 및 connect
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


	// 기존에 등록된 계정이 있는지 물어본다.
	char registered;			// 등록여부
	char input_name[NAME_LEN];	// 계정이름 
	while (1) {
	ASK_ALREADY_REGISTERED:
		cout << "이미 등록된 계정이 있나요? [예: Y | 아니오: N]: ";
		cin >> registered;

		if (registered == 'Y' || registered == 'y') {
			// 로그인 시도
		LOGIN_NAME_INPUT_AGAIN:
			char temp[100];
			cout << "이름을 입력해주세요: ";
			cin >> temp;

			if (strlen(temp) > NAME_LEN)
				cout << "이름은 최대 " << NAME_LEN << "자를 초과할 수 없습니다.\n" << endl;
			else
				strcpy(input_name, temp);

			// 로그인 서버에 로그인 요청
			C2LS_LOGIN_PACKET login_packet;
			login_packet.size = sizeof(C2LS_LOGIN_PACKET);
			login_packet.type = C2LS_LOGIN;
			strcpy_s(login_packet.name, input_name);
			retval = send(sock_forLS, (char*)&login_packet, sizeof(C2LS_LOGIN_PACKET), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
			}

			// 서버로부터 로그인 성공 패킷 받기
			PACKET_INFO recv_info;
			retval = recv(sock_forLS, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK을 사용하여 수신버퍼에서 삭제하지 않도록 함.
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
					cout << "로그인이 허가되었습니다.\n" << endl;
					g_gamestart = true;

					closesocket(sock_forLS);
					return 0;
				}
				else if (start_pack.start == START_DENY_UNKNOWNNAME) {
					cout << "등록되지 않은 이름입니다. 등록 후 다시 로그인을 시도해주세요.\n" << endl;
				}
				else if (start_pack.start == START_DENY_FULL) {
					cout << "현재 서버가 포화상태입니다. 잠시 후 다시 시도해 주세요.\n" << endl;
				}
				else {
					cout << "알 수 없는 이유로 로그인 요청이 거절되었습니다. 다시 계정을 입력해주세요.\n" << endl;
				}
			}
		}
		else if (registered == 'N' || registered == 'n') {
			// 계정 등록 절차
		REGISTERED_NAME_INPUT_AGAIN:
			char temp_name[100];
			cout << "이름을 입력해주세요: ";
			cin >> temp_name;

			if (strlen(temp_name) <= NAME_LEN) {
				strcpy(input_name, temp_name);
			}
			else {
				cout << "이름은 최대 " << NAME_LEN << "자를 초과할 수 없습니다.\n" << endl;
				goto REGISTERED_NAME_INPUT_AGAIN;	// 다시 입력받도록.
			}

			// 로그인 서버에 계정 등록 요청
			C2LS_REGISTER_PACKET register_pack;
			register_pack.size = sizeof(C2LS_REGISTER_PACKET);
			register_pack.type = C2LS_REGISTER;
			strcpy(register_pack.name, input_name);
			retval = send(sock_forLS, (char*)&register_pack, sizeof(C2LS_REGISTER_PACKET), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
			}

			// 서버로부터 계정 등록완료 패킷 받기
			PACKET_INFO recv_info;
			retval = recv(sock_forLS, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK을 사용하여 수신버퍼에서 삭제하지 않도록 함.
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
					cout << "계정이 성공적으로 등록되었습니다.\n" << endl;
					g_registerd = true;
				}
				else {
					cout << "이미 존재하는 이름입니다. 다른 이름을 입력해주세요.\n" << endl;
					goto REGISTERED_NAME_INPUT_AGAIN;
				}
			}
		}
		else {
			// 잘못된 값 입력
			cout << "Y 또는 N 으로만 입력해주세요." << endl;
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

	retval = WaitForSingleObject(h_thread_event_GS, INFINITE);	// 신호 상태가 될때까지 대기
	if (retval != WAIT_OBJECT_0) return -1;

	// 소켓 생성 및 connect
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
		retval = recv(sock_forGS, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK을 사용하여 수신버퍼를 읽지만 가져오지는 않도록
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

			// 서버로부터 할당받은 id
			myID = login_info_pack.id;

			// 클라 내 객체정보 컨테이너에 서버로 부터 받은 값을 저장합니다.
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

			// 추가되는 객체의 id
			int new_id = add_obj_pack.id;

			// 클라 내 객체정보 컨테이너에 서버로 부터 받은 값을 저장합니다.
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

			if (update_pack.id == myID) {	// 자신의 이동
				players_info[myID].m_pos = { update_pack.pos_x, update_pack.pos_y, update_pack.pos_z };

				players_info[myID].m_right_vec = { update_pack.right_vec_x, update_pack.right_vec_y, update_pack.right_vec_z };
				players_info[myID].m_up_vec = { update_pack.up_vec_x, update_pack.up_vec_y, update_pack.up_vec_z };
				players_info[myID].m_look_vec = { update_pack.look_vec_x, update_pack.look_vec_y, update_pack.look_vec_z };

				// Test Log
				//cout << "My Car moves to Pos(" << players_info[myID].m_pos.x << ", " << players_info[myID].m_pos.y << ", " << players_info[myID].m_pos.z << ")." << endl;
			}
			else if (update_pack.id >= 0 && update_pack.id <= MAX_USER) {	// 다른 플레이어의 이동
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