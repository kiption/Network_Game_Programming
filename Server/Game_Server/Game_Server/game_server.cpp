#include "../../Common/protocol.h"
#include "../../Common/Common.h"

#include <iostream>
#include <array>

#include "CalcMove.h"
#include "Global.h"

using namespace std;

DWORD WINAPI ProcessClient(LPVOID arg);
DWORD WINAPI ServerTimer(LPVOID arg);

// 클라이언트 객체 정보
class ClientINFO {
private:
	SOCKET		m_sock;

	int			m_id = 0;
	bool		m_onlinestate = false;

	float		m_acceleator;
	float		m_yaw, m_pitch, m_roll;
	MyVector3D	m_pos;
	Coordinate	m_coordinate;
	float		m_timer;

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
		m_acceleator = 2.0f;
	};

	SOCKET		getSock() { return m_sock; }
	bool		getState() { return m_onlinestate; }
	int			getId() { return m_id; }
	float		getAccel() { return m_acceleator; }
	MyVector3D	getPos() { return m_pos; }
	Coordinate	getCoordinate() { return m_coordinate; }
	float		getTimer() {}

	void		setSock(SOCKET socket) { m_sock = socket; }
	void		setState(bool state) { m_onlinestate = state; }
	void		setAccel(float accel) { m_acceleator = accel; }
	void		setID(int id) { m_id = id; }
	void		setPos(MyVector3D pos) { m_pos = pos; }
	void		setCoordinate(Coordinate co) { m_coordinate = co; }
};
array<ClientINFO, MAX_USER> clients;

// main( )
int main(int argc, char* argv[])
{
	int retval;
	::QueryPerformanceFrequency;
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
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

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread, tThread;

	tThread = CreateThread(NULL, 0, ServerTimer, 0, 0, NULL);

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
		if (hThread == NULL)
			closesocket(client_sock);
		else
			CloseHandle(hThread);
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 통신 스레드
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

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	retval = send(client_sock, reinterpret_cast<char*>(&S2CPacket), sizeof(GS2C_LOGIN_INFO_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}

	while (1) {
		// 이동 함수
		C2GS_KEYVALUE_PACKET ClientPushKey;
		retval = recv(client_sock, reinterpret_cast<char*>(&ClientPushKey), sizeof(C2GS_KEYVALUE_PACKET), 0);
		MyVector3D move_dir{ 0,0,0 };

		enum { KEY_AD, KEY_WS, KEY_SPACE };
		for (int i{}; i < 5; ++i) {
			if ((ClientPushKey.key >> i) & 1) {

				int DefaultDir = 1;

				if (i % 2 == 0) {
					DefaultDir = -1;
				}
				switch (i / 2) {
				case KEY_AD:
					/*move_dir = clients[id].getCoordinate().x_coordinate;
					MyVector3D Move_AD_Result = calcMove(clients[id].getPos(), move_dir * DefaultDir, MOVE_SCALAR);
					clients[id].setPos(Move_AD_Result);*/
					/*GS2C_ROTATE_PACKET S2CRotatePacket;

					float right_vec_x, right_vec_y, right_vec_z;
					float up_vec_x, up_vec_y, up_vec_z;
					float look_vec_x, look_vec_y, look_vec_z;


					S2CRotatePacket.id = clients[id].getId();
					S2CRotatePacket.pos_x = clients[id].getPos().x;
					S2CRotatePacket.pos_y = clients[id].getPos().y;
					S2CRotatePacket.pos_z = clients[id].getPos().z;
					for (int i{}; i < MAX_USER; ++i) {
						if (clients[i].getState()) {
							send(clients[i].getSock(), reinterpret_cast<char*>(&S2CMovePacket), sizeof(GS2C_MOVE_PACKET), 0);
						}

					}*/


					break;
				case KEY_WS:
					move_dir = { clients[id].getCoordinate().z_coordinate.x * DefaultDir,
						clients[id].getCoordinate().z_coordinate.y * DefaultDir, clients[id].getCoordinate().z_coordinate.z * DefaultDir };


					MyVector3D Move_WS_Result = calcMove(clients[id].getPos(), move_dir, MOVE_SCALAR, clients[id].getAccel());
					clients[id].setPos(Move_WS_Result);

					GS2C_MOVE_PACKET S2CMovePacket;
					S2CMovePacket.id = clients[id].getId();
					S2CMovePacket.pos_x = clients[id].getPos().x;
					S2CMovePacket.pos_y = clients[id].getPos().y;
					S2CMovePacket.pos_z = clients[id].getPos().z;
					for (int i{}; i < MAX_USER; ++i) {
						if (clients[i].getState()) {
							send(clients[i].getSock(), reinterpret_cast<char*>(&S2CMovePacket), sizeof(GS2C_MOVE_PACKET), 0);
						}

					}

					break;
				case KEY_SPACE:
					break;
				default:
					break;
				}
			}
		}
	}

	// 소켓 닫기
	closesocket(client_sock);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return 0;
}

DWORD WINAPI ServerTimer(LPVOID arg)
{
	while (1) {
		for (int i{}; i < 3; ++i) {
			if (clients[i].getState()) {




			}
		}
	}
}


