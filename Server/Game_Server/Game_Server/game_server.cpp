#include "../../Common/protocol.h"
#include "../../Common/Common.h"

#include <iostream>
#include <array>

#include "CalcMove.h"
#include "Global.h"

using namespace std;

DWORD WINAPI ProcessClient(LPVOID arg);
DWORD WINAPI ServerTimer(LPVOID arg);

Coordinate basic_coordinate;	// 기본(초기) 좌표계

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
	float		getPitch() { return m_pitch; }
	float		getYaw() { return m_yaw; }
	float		getRoll() { return m_roll; }
	float		getTimer() {}

	void		setSock(SOCKET socket) { m_sock = socket; }
	void		setState(bool state) { m_onlinestate = state; }
	void		setAccel(float accel) { m_acceleator = accel; }
	void		setID(int id) { m_id = id; }
	void		setPos(MyVector3D pos) { m_pos = pos; }
	void		setCoordinate(Coordinate co) { m_coordinate = co; }
	void		setCoordinate(MyVector3D x, MyVector3D y, MyVector3D z) { m_coordinate.x_coordinate = x; m_coordinate.y_coordinate = y; m_coordinate.z_coordinate = z; }
	void		setPitch(float f) { m_pitch = f; }
	void		setYaw(float f) { m_yaw = f; }
	void		setRoll(float f) { m_roll = f; }
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
	serveraddr.sin_port = htons(GAME_SERVER_PORT);
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
		cout << "\n[TCP 서버] 클라이언트 접속: IP 주소=" << addr << ", 포트 번호 = " << ntohs(clientaddr.sin_port) << endl;

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
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	// 클라이언트ID 할당
	int client_id = 0;
	for (int j = 0; j < MAX_USER; ++j) {
		if (clients[j].getId() == -1) {
			client_id = j;

			clients[client_id].setID(client_id);
			clients[client_id].setSock(client_sock);
			clients[client_id].setState(true);
			
			break;
		}
	}

	// 접속한 클라이언트의 정보를 초기화합니다.
	clients[client_id].setID(client_id);
	MyVector3D Pos = { 400 + 50 * client_id, 2, 400 + 50 * client_id };
	clients[client_id].setPos(Pos);

	// 접속한 클라이언트에게 초기 정보를 전달합니다.
	GS2C_LOGIN_INFO_PACKET S2CPacket;
	S2CPacket.size = sizeof(GS2C_LOGIN_INFO_PACKET);
	S2CPacket.type = GS2C_LOGIN_INFO;
	S2CPacket.id = clients[client_id].getId();

	S2CPacket.pos_x = clients[client_id].getPos().x;
	S2CPacket.pos_y = clients[client_id].getPos().y;
	S2CPacket.pos_z = clients[client_id].getPos().z;

	S2CPacket.right_vec_x = clients[client_id].getCoordinate().x_coordinate.x;
	S2CPacket.right_vec_y = clients[client_id].getCoordinate().x_coordinate.y;
	S2CPacket.right_vec_z = clients[client_id].getCoordinate().x_coordinate.z;

	S2CPacket.up_vec_x = clients[client_id].getCoordinate().y_coordinate.x;
	S2CPacket.up_vec_y = clients[client_id].getCoordinate().y_coordinate.y;
	S2CPacket.up_vec_z = clients[client_id].getCoordinate().y_coordinate.z;

	S2CPacket.look_vec_x = clients[client_id].getCoordinate().z_coordinate.x;
	S2CPacket.look_vec_y = clients[client_id].getCoordinate().z_coordinate.y;
	S2CPacket.look_vec_z = clients[client_id].getCoordinate().z_coordinate.z;

	retval = send(client_sock, reinterpret_cast<char*>(&S2CPacket), sizeof(GS2C_LOGIN_INFO_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
	cout << "Send Client[" << clients[client_id].getId() << "]'s Info - Pos:(" << S2CPacket.pos_x << ", " << S2CPacket.pos_y << ", " << S2CPacket.pos_z << "), "
		<< "LookVec:(" << S2CPacket.look_vec_x << ", " << S2CPacket.look_vec_y << ", " << S2CPacket.look_vec_z << ")." << endl;

	// Loop
	while (1) {
		// 이동 함수
		C2GS_KEYVALUE_PACKET ClientPushKey;
		retval = recv(client_sock, reinterpret_cast<char*>(&ClientPushKey), sizeof(C2GS_KEYVALUE_PACKET), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}
		
		enum { KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP, KEY_SPACE };
		for (int i = KEY_LEFT; i <= KEY_SPACE; ++i) {
			if ((ClientPushKey.key >> i) & 1) {

				int plus_minus = 1;	// 양수 음수

				switch (i) {
				case KEY_LEFT:
					plus_minus = -1;
					[[fallthrough]];
				case KEY_RIGHT:
				{
					// yaw 설정
					clients[client_id].setYaw(clients[client_id].getYaw() + ROTATE_SCALAR * plus_minus * PI / 360.0f);

					// right, up, look 벡터 회전계산
					MyVector3D rotate_result_x = calcRotate(basic_coordinate.x_coordinate
						, clients[client_id].getRoll(), clients[client_id].getPitch(), clients[client_id].getYaw());
					MyVector3D rotate_result_y = calcRotate(basic_coordinate.y_coordinate
						, clients[client_id].getRoll(), clients[client_id].getPitch(), clients[client_id].getYaw());
					MyVector3D rotate_result_z = calcRotate(basic_coordinate.z_coordinate
						, clients[client_id].getRoll(), clients[client_id].getPitch(), clients[client_id].getYaw());

					// right, up, look 벡터 회전결과 적용
					clients[client_id].setCoordinate(rotate_result_x, rotate_result_y, rotate_result_z);

					break;
				}
				case KEY_DOWN:
					plus_minus = -1;
					[[fallthrough]];
				case KEY_UP:
				{
					MyVector3D move_dir{ 0, 0, 0 };
					move_dir = { clients[client_id].getCoordinate().z_coordinate.x * plus_minus,
						clients[client_id].getCoordinate().z_coordinate.y * plus_minus, clients[client_id].getCoordinate().z_coordinate.z * plus_minus };

					MyVector3D Move_Vertical_Result = calcMove(clients[client_id].getPos(), move_dir, MOVE_SCALAR, clients[client_id].getAccel());
					clients[client_id].setPos(Move_Vertical_Result);

					break;
				}
				case KEY_SPACE:
					break;
				default:
					break;
				}

				// Send Packet
				GS2C_UPDATE_PACKET update_pack;
				update_pack.id = clients[client_id].getId();
				update_pack.type = GS2C_UPDATE;

				update_pack.pos_x = clients[client_id].getPos().x;
				update_pack.pos_y = clients[client_id].getPos().y;
				update_pack.pos_z = clients[client_id].getPos().z;

				update_pack.right_vec_x = clients[client_id].getCoordinate().x_coordinate.x;
				update_pack.right_vec_y = clients[client_id].getCoordinate().x_coordinate.y;
				update_pack.right_vec_z = clients[client_id].getCoordinate().x_coordinate.z;

				update_pack.up_vec_x = clients[client_id].getCoordinate().y_coordinate.x;
				update_pack.up_vec_y = clients[client_id].getCoordinate().y_coordinate.y;
				update_pack.up_vec_z = clients[client_id].getCoordinate().y_coordinate.z;

				update_pack.look_vec_x = clients[client_id].getCoordinate().z_coordinate.x;
				update_pack.look_vec_y = clients[client_id].getCoordinate().z_coordinate.y;
				update_pack.look_vec_z = clients[client_id].getCoordinate().z_coordinate.z;

				retval = send(client_sock, (char*)&update_pack, sizeof(GS2C_UPDATE_PACKET), 0);		// 서버로 전송합니다.
				if (retval == SOCKET_ERROR) {
					err_display("send()");
				}
			}
		}
	}

	// 소켓 닫기
	closesocket(client_sock);
	cout << "[TCP 서버] 클라이언트 종료: IP 주소= " << addr << ", 포트 번호 = " << ntohs(clientaddr.sin_port) << endl;
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


