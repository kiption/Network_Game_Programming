#pragma once
#include "../Server/Common/Common.h"
#include "../Server/Common/protocol.h"

char* SERVERIP = (char*)"127.0.0.1";

bool g_gamestart = false;

HANDLE h_network_th = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE h_main_th = CreateEvent(NULL, FALSE, FALSE, NULL);

int retval;

DWORD WINAPI NetworkingThreadFunc(LPVOID arg)
{
	//==== Server
	SOCKET sock = (SOCKET)arg;

	C2LS_LOGIN_PACKET login_packet;
	login_packet.size = sizeof(C2LS_LOGIN_PACKET);
	login_packet.type = C2LS_LOGIN;
	strcpy_s(login_packet.name, "Player1");
	retval = send(sock, (char*)&login_packet, sizeof(C2LS_LOGIN_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}

	// Recv Login Packet
	while (1) {
		retval = WaitForSingleObject(h_network_th, INFINITE);	// 신호 상태가 될때까지 대기
		if (retval != WAIT_OBJECT_0) break;

		int recv_bytes = 0;
		PACKET_INFO recv_info;
		while (recv_bytes < 1) {
			retval = recv(sock, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK을 사용하여 수신버퍼를 읽지만 가져오지는 않도록
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

		/*SetEvent(h_main_th);*/
		//ResetEvent(h_network_th);	// 스레드 이벤트를 비신호 상태로 바꿈
	}

	//====

	return 0;
}