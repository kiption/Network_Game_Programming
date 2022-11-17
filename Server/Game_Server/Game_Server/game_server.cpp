#include "../../Common/protocol.h"
#include "../../Common/Common.h"

#include <iostream>
#include <array>

#include "CalcMove.h"
#include "Global.h"

using namespace std;

DWORD WINAPI ProcessClient(LPVOID arg);
void SCMoveProcess(int client_id);

// Ŭ���̾�Ʈ ��ü ����
class ClientINFO {
private:
	int			m_id = 0;
	SOCKET		m_sock;
	bool		m_onlinestate = false;

	MyVector3D	m_pos;
	float		m_yaw, m_pitch, m_roll;
	Coordinate	m_coordinate;

public:
	ClientINFO() {
		m_id = 0;
		m_sock = 0;
		m_onlinestate = false;
		m_pos = { 0.f, 0.f, 0.f };
		m_yaw = m_pitch = m_roll = 0.f;
		MyVector3D tmp_rightvec = { 1.f, 0.f, 0.f };
		MyVector3D tmp_upvec = { 0.f, 1.f, 0.f };
		MyVector3D tmp_lookvec = { 0.f, 0.f, 1.f };
		m_coordinate.x_coordinate = tmp_rightvec;
		m_coordinate.y_coordinate = tmp_upvec;
		m_coordinate.z_coordinate = tmp_lookvec;

	};

	bool		getState() { return m_onlinestate; }
	SOCKET		getSock() { return m_sock; }
	int			getId() { return m_id; }
	MyVector3D	getPos() { return m_pos; }
	Coordinate	getCoordinate() { return m_coordinate; }

	void		setState(bool state) { m_onlinestate = state; }
	void		setSock(SOCKET socket) { m_sock = socket; }
	void		setID(int id) { m_id = id; }
	void		setPos(MyVector3D pos) { m_pos = pos; }
	void		setCoordinate(Coordinate co) { m_coordinate = co; }
};
array<ClientINFO, MAX_USER> clients;

// main( )
int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVER_PORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// ������ ����
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
		if (hThread == NULL)
			closesocket(client_sock);
		else
			CloseHandle(hThread);
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}

// ��� ������
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int id = 0;
	for (int j = 0; j < MAX_USER; ++j) {
		if (clients[j].getId() == -1) {
			clients[j].setID(j);
			clients[j].setSock(reinterpret_cast<SOCKET>(arg));
			clients[j].setState(true);
			id = j;
			break;
		}
	}

	GS2C_LOGIN_INFO_PACKET S2CPacket;
	S2CPacket.id = clients[id].getId();

	S2CPacket.pos_x = 200 + 50 * id;
	S2CPacket.pos_y = 2;
	S2CPacket.pos_z = 200 + 50 * id;

	S2CPacket.right_vec_x = clients[id].getCoordinate().x_coordinate.x;
	S2CPacket.right_vec_y = clients[id].getCoordinate().x_coordinate.y;
	S2CPacket.right_vec_z = clients[id].getCoordinate().x_coordinate.z;

	S2CPacket.up_vec_x = clients[id].getCoordinate().y_coordinate.x;
	S2CPacket.up_vec_y = clients[id].getCoordinate().y_coordinate.y;
	S2CPacket.up_vec_z = clients[id].getCoordinate().y_coordinate.z;

	S2CPacket.look_vec_x = clients[id].getCoordinate().z_coordinate.x;
	S2CPacket.look_vec_y = clients[id].getCoordinate().z_coordinate.y;
	S2CPacket.look_vec_z = clients[id].getCoordinate().z_coordinate.z;


	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	retval = send(client_sock, reinterpret_cast<char*>(&S2CPacket), sizeof(S2CPacket), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}

	while (1) {
		// �̵� �Լ�
		SCMoveProcess(id);

		GS2C_MOVE_PACKET S2CMovePacket;
		S2CMovePacket.id = clients[id].getId();
		S2CMovePacket.pos_x = clients[id].getPos().x;
		S2CMovePacket.pos_y = clients[id].getPos().y;
		S2CMovePacket.pos_z = clients[id].getPos().z;


		for (int i{}; i < MAX_USER; ++i) {
			if (clients[i].getState()) {
				send(clients[i].getSock(), reinterpret_cast<char*>(&S2CMovePacket), sizeof(S2CMovePacket), 0);
			}
		}

		// ==== [��ö �ʵ�] ====
		// retval = recv(client_sock, (char*)&��Ŷ����ü ��ü�̸�, sizeof(��Ŷ����ü ����), 0);  <- �̷������� send/recv���ָ� ��.
		// ====				====

		// ������ �ޱ�
		//retval = recv(client_sock, buf, BUFSIZE, 0);
		//if (retval == SOCKET_ERROR) {
		//	err_display("recv()");
		//	break;
		//}
		//else if (retval == 0)
		//	break;

		// ������ ������
		//retval = send(client_sock, buf, retval, 0);
		//if (retval == SOCKET_ERROR) {
		//	err_display("send()");
		//	break;
		//}
	}

	// ���� �ݱ�
	closesocket(client_sock);
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return 0;
}

void SCMoveProcess(int client_id)
{
	// Ŭ���̾�Ʈ ������ ���� �� ����� �ۼ� ����
	// ���� Ű������ ����ġ ���� �ۼ�
	// �������ݿ��ٰ� ����� Ŭ���̾�Ʈ �� �ֱ�
	// ���Ĵ� �躤�ͷ�
	C2GS_KEYVALUE_PACKET ClientPushKey{};

	MyVector3D move_dir{ 0,0,0 };
	// ����
	// w,s Ű
	switch (ClientPushKey.type)
	{
		enum {KEY_WS, KEY_AD};
	case KEY_WS:
		move_dir = clients[client_id].getCoordinate().z_coordinate;
		MyVector3D Move_WS_Result = calcMove(clients[client_id].getPos(), move_dir, MOVE_SCALAR);
		clients[client_id].setPos(Move_WS_Result);
		break;
	case KEY_AD:
		move_dir = clients[client_id].getCoordinate().x_coordinate;
		MyVector3D Move_AD_Result = calcMove(clients[client_id].getPos(), move_dir, MOVE_SCALAR);
		clients[client_id].setPos(Move_AD_Result);
		break;
	default:
		break;
	}
}


