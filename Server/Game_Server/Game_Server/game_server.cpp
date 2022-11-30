#include "../../Common/protocol.h"
#include "../../Common/Common.h"

#include <iostream>
#include <array>
#include <vector>
#include <queue>
#include "CalcMove.h"
#include "Global.h"

using namespace std;

DWORD WINAPI ProcessClient(LPVOID arg);		// 클라이언트 통신 스레드
DWORD WINAPI TimerThreadFunc(LPVOID arg);	// 타이머 스레드
DWORD WINAPI ServerTime_Update(LPVOID arg);	// 서버 시간 갱신 스레드

Coordinate basic_coordinate;	// 기본(초기) 좌표계

float START_TIME;						// 서버 프로그램이 켜진 시간
float SERVER_TIME;						// 서버 시간
constexpr int TIME_UPDATE_CYCLE = 100;	// 서버 시간 업데이트 주기 (ms단위)

//==================================================
//           [ 클라이언트 객체 정보 ]
//==================================================
enum { CL_STATE_EMPTY, CL_STATE_RUNNING };

class ClientINFO {
private:
	SOCKET		m_sock;

	int			m_id = 0;
	char		m_state;

	float		m_acceleator;
	float		m_yaw, m_pitch, m_roll;
	MyVector3D	m_pos;
	Coordinate	m_coordinate;
	float		m_timer;

	queue<int>m_myitem;

public:
	ClientINFO() {
		m_id = 0;
		m_sock = 0;
		m_state = CL_STATE_EMPTY;
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
	char		getState() { return m_state; }
	int			getId() { return m_id; }
	float		getAccel() { return m_acceleator; }
	MyVector3D	getPos() { return m_pos; }
	Coordinate	getCoordinate() { return m_coordinate; }
	float		getPitch() { return m_pitch; }
	float		getYaw() { return m_yaw; }
	float		getRoll() { return m_roll; }
	float		getTimer() {}
	int			getItemQueue() {
		if (m_myitem.empty()) return -1;	// 큐가 비어있으면 -1을
		return m_myitem.front();			// 아이템이 있다면 아이템의 고유번호를 반환합니다.
	}

	void		setSock(SOCKET sock) { m_sock = sock; }
	void		setState(char state) { m_state = state; }
	void		setAccel(float accel) { m_acceleator = accel; }
	void		setID(int id) { m_id = id; }
	void		setPos(MyVector3D pos) { m_pos = pos; }
	void		setCoordinate(Coordinate co) { m_coordinate = co; }
	void		setCoordinate(MyVector3D x, MyVector3D y, MyVector3D z) { m_coordinate.x_coordinate = x; m_coordinate.y_coordinate = y; m_coordinate.z_coordinate = z; }
	void		setPitch(float f) { m_pitch = f; }
	void		setYaw(float f) { m_yaw = f; }
	void		setRoll(float f) { m_roll = f; }
	void		setItemQueue(int type) { m_myitem.push(type); } // 아이템 먹을 시에 사용
	void		setItemRelease() { m_myitem.pop(); } // 사용한 아이템 방출

public:
	void		sendLoginInfoPacket(GS2C_LOGIN_INFO_PACKET packet);
	void		sendAddObjPacket(GS2C_ADD_OBJ_PACKET packet);
	void		sendUpdatePacket(GS2C_UPDATE_PACKET packet);
};
array<ClientINFO, MAX_USER> clients;
//==================================================

//==================================================
//             [ 아이템 객체 정보 ]
//==================================================
enum { ITEM_Booster, ITEM_Missile, ITEM_Bomb };
class ItemObject {
private:
	int			m_objtype;
	int			m_objOwner;
	float		m_yaw, m_pitch, m_roll;
	MyVector3D	m_pos;
	Coordinate	m_coordinate;
public:
	ItemObject() {
		m_objtype = -1;
		m_objOwner = -1;
		m_pos = { 0.f, 0.f, 0.f };
		m_yaw = m_pitch = m_roll = 0.f;
		MyVector3D tmp_rightvec = { 1.f, 0.f, 0.f };
		MyVector3D tmp_upvec = { 0.f, 1.f, 0.f };
		MyVector3D tmp_lookvec = { 0.f, 0.f, 1.f };
		m_coordinate.x_coordinate = tmp_rightvec;
		m_coordinate.y_coordinate = tmp_upvec;
		m_coordinate.z_coordinate = tmp_lookvec;
	};
	int			getObjType() { return m_objtype; }
	int			getObjOwner() { return m_objOwner; }
	MyVector3D	getPos() { return m_pos; }
	Coordinate	getCoordinate() { return m_coordinate; }
	float		getPitch() { return m_pitch; }
	float		getYaw() { return m_yaw; }
	float		getRoll() { return m_roll; }

	void		setObjType(int type) { m_objtype = type; }
	void		setObjOwner(int num) { m_objOwner = num; }
	void		setPos(MyVector3D pos) { m_pos = pos; }
	void		setCoordinate(Coordinate co) { m_coordinate = co; }
	void		setCoordinate(MyVector3D x, MyVector3D y, MyVector3D z) { m_coordinate.x_coordinate = x; m_coordinate.y_coordinate = y; m_coordinate.z_coordinate = z; }
	void		setPitch(float f) { m_pitch = f; }
	void		setYaw(float f) { m_yaw = f; }
	void		setRoll(float f) { m_roll = f; }
};
vector<ItemObject> ObjectManager; // 미사일 지뢰 렌더링 정보들 담아두는 곳
//==================================================

//==================================================
//           [ 아이템 박스 객체 정보 ]
//==================================================
struct ItemBox {
	float		m_yaw, m_pitch, m_roll; // 회전 각도
	MyVector3D	m_pos;
	Coordinate	m_coordinate; // 회전 행렬
	bool		m_visible;
};
array<ItemBox, ITEMBOXNUM> ItemBoxArray;
//==================================================


//==================================================
//           [ Send Packet Functions ]
//==================================================
void ClientINFO::sendLoginInfoPacket(GS2C_LOGIN_INFO_PACKET packet) {
	int retval = send(m_sock, (char*)&packet, sizeof(GS2C_LOGIN_INFO_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		//err_display("send()");
	}
}
void ClientINFO::sendAddObjPacket(GS2C_ADD_OBJ_PACKET packet) {
	int retval = send(m_sock, (char*)&packet, sizeof(GS2C_ADD_OBJ_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		//err_display("send()");
	}
}
void ClientINFO::sendUpdatePacket(GS2C_UPDATE_PACKET packet) {
	int retval = send(m_sock, (char*)&packet, sizeof(GS2C_UPDATE_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		//err_display("send()");
	}
}
void sendUpdatePacket_toAllClient(int c_id) {	// 모든 클라이언트에게 보내는 함수
	GS2C_UPDATE_PACKET update_packet;
	// client_id번째 클라이언트 객체의 변경사항을 보낼 패킷에 담습니다.
	update_packet.id = clients[c_id].getId();
	update_packet.type = GS2C_UPDATE;
	update_packet.objtype = OBJ_TYPE_PLAYER;

	update_packet.pos_x = clients[c_id].getPos().x;
	update_packet.pos_y = clients[c_id].getPos().y;
	update_packet.pos_z = clients[c_id].getPos().z;

	update_packet.right_vec_x = clients[c_id].getCoordinate().x_coordinate.x;
	update_packet.right_vec_y = clients[c_id].getCoordinate().x_coordinate.y;
	update_packet.right_vec_z = clients[c_id].getCoordinate().x_coordinate.z;

	update_packet.up_vec_x = clients[c_id].getCoordinate().y_coordinate.x;
	update_packet.up_vec_y = clients[c_id].getCoordinate().y_coordinate.y;
	update_packet.up_vec_z = clients[c_id].getCoordinate().y_coordinate.z;

	update_packet.look_vec_x = clients[c_id].getCoordinate().z_coordinate.x;
	update_packet.look_vec_y = clients[c_id].getCoordinate().z_coordinate.y;
	update_packet.look_vec_z = clients[c_id].getCoordinate().z_coordinate.z;

	// client_id번째 클라이언트 객체의 변경된 사항을 모든 클라이언트들에게 전달합니다.
	for (int i = 0; i < MAX_USER; i++) {
		if (clients[i].getState() == CL_STATE_EMPTY) continue;

		clients[i].sendUpdatePacket(update_packet);
	}
}
//==================================================

//==================================================
//                   [ Main( ) ]
//==================================================
int main(int argc, char* argv[])
{
	// 서버 시간 초기화
	START_TIME = 0.0f;
	SERVER_TIME = 0.0f;
	// 서버시간을 업데이트시켜주는 스레드 생성
	HANDLE hTimeUpdateThread = CreateThread(NULL, 0, ServerTime_Update, 0, 0, NULL);

	// 주기적인 작업을 수행하는 스레드 생성
	HANDLE hTimerThreadThread = CreateThread(NULL, 0, TimerThreadFunc, 0, 0, NULL);

	// 아이템 박스의 위치값을 설정합니다.
	for (int i{}; i < 3; ++i) {
		ItemBoxArray[i].m_pos.x = 350 + i * 40;
		ItemBoxArray[i].m_pos.y = 20;
		ItemBoxArray[i].m_pos.z = MiddleZ;

		ItemBoxArray[3 + i].m_pos.x = MiddleX;
		ItemBoxArray[3 + i].m_pos.y = 20;
		ItemBoxArray[3 + i].m_pos.z = 2220 - i * 40;

		ItemBoxArray[6 + i].m_pos.x = 2240 - i * 40;
		ItemBoxArray[6 + i].m_pos.y = 20;
		ItemBoxArray[6 + i].m_pos.z = MiddleZ;

		ItemBoxArray[9 + i].m_pos.x = MiddleX;
		ItemBoxArray[9 + i].m_pos.y = 20;
		ItemBoxArray[9 + i].m_pos.z = 400 + i * 40;
	}

	// 통신 관련 초기작업들
	int retval;
	::QueryPerformanceFrequency;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(GAME_SERVER_PORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			//err_display("accept()");
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
//==================================================

//==================================================
//             [ 통신 스레드 함수 ]
//==================================================
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
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i].getState() == CL_STATE_EMPTY) {
			client_id = i;

			clients[client_id].setID(client_id);
			clients[client_id].setSock(client_sock);
			clients[client_id].setState(CL_STATE_RUNNING);

			break;
		}
	}

	// 새로 접속한 클라이언트의 정보를 초기화합니다.
	clients[client_id].setID(client_id);
	MyVector3D Pos = { 400 + 50 * client_id, 14.0, 400 + 50 * client_id };
	clients[client_id].setPos(Pos);

	// 새로 접속한 클라이언트에게 자신의 초기 정보를 전달합니다.
	GS2C_LOGIN_INFO_PACKET login_packet;
	login_packet.size = sizeof(GS2C_LOGIN_INFO_PACKET);
	login_packet.type = GS2C_LOGIN_INFO;
	login_packet.id = clients[client_id].getId();

	login_packet.pos_x = clients[client_id].getPos().x;
	login_packet.pos_y = clients[client_id].getPos().y;
	login_packet.pos_z = clients[client_id].getPos().z;

	login_packet.right_vec_x = clients[client_id].getCoordinate().x_coordinate.x;
	login_packet.right_vec_y = clients[client_id].getCoordinate().x_coordinate.y;
	login_packet.right_vec_z = clients[client_id].getCoordinate().x_coordinate.z;

	login_packet.up_vec_x = clients[client_id].getCoordinate().y_coordinate.x;
	login_packet.up_vec_y = clients[client_id].getCoordinate().y_coordinate.y;
	login_packet.up_vec_z = clients[client_id].getCoordinate().y_coordinate.z;

	login_packet.look_vec_x = clients[client_id].getCoordinate().z_coordinate.x;
	login_packet.look_vec_y = clients[client_id].getCoordinate().z_coordinate.y;
	login_packet.look_vec_z = clients[client_id].getCoordinate().z_coordinate.z;

	clients[client_id].sendLoginInfoPacket(login_packet);
	cout << "Send Client[" << clients[client_id].getId() << "]'s Info - Pos:(" << login_packet.pos_x << ", " << login_packet.pos_y << ", " << login_packet.pos_z << "), "
		<< "LookVec:(" << login_packet.look_vec_x << ", " << login_packet.look_vec_y << ", " << login_packet.look_vec_z << ")." << endl;


	// 새로 접속한 클라이언트에게 현재 접속해 있는 모든 클라이언트들의 객체 정보를 전달합니다.
	for (int i = 0; i < MAX_USER; i++) {
		if (clients[i].getState() == CL_STATE_EMPTY) continue;
		if (i == client_id) continue;

		GS2C_ADD_OBJ_PACKET add_others_packet;
		add_others_packet.size = sizeof(GS2C_ADD_OBJ_PACKET);
		add_others_packet.type = GS2C_ADD_OBJ;
		add_others_packet.id = clients[i].getId();
		add_others_packet.objtype = OBJ_TYPE_PLAYER;

		add_others_packet.pos_x = clients[i].getPos().x;
		add_others_packet.pos_y = clients[i].getPos().y;
		add_others_packet.pos_z = clients[i].getPos().z;

		add_others_packet.right_vec_x = clients[i].getCoordinate().x_coordinate.x;
		add_others_packet.right_vec_y = clients[i].getCoordinate().x_coordinate.y;
		add_others_packet.right_vec_z = clients[i].getCoordinate().x_coordinate.z;

		add_others_packet.up_vec_x = clients[i].getCoordinate().y_coordinate.x;
		add_others_packet.up_vec_y = clients[i].getCoordinate().y_coordinate.y;
		add_others_packet.up_vec_z = clients[i].getCoordinate().y_coordinate.z;

		add_others_packet.look_vec_x = clients[i].getCoordinate().z_coordinate.x;
		add_others_packet.look_vec_y = clients[i].getCoordinate().z_coordinate.y;
		add_others_packet.look_vec_z = clients[i].getCoordinate().z_coordinate.z;

		clients[client_id].sendAddObjPacket(add_others_packet);
	}

	// 현재 접속해 있는 모든 클라이언트들에게 새로 접속한 클라이언트의 객체 정보를 전달합니다.
	GS2C_ADD_OBJ_PACKET add_me_packet;
	add_me_packet.size = sizeof(GS2C_ADD_OBJ_PACKET);
	add_me_packet.type = GS2C_ADD_OBJ;
	add_me_packet.id = clients[client_id].getId();
	add_me_packet.objtype = OBJ_TYPE_PLAYER;

	add_me_packet.pos_x = clients[client_id].getPos().x;
	add_me_packet.pos_y = clients[client_id].getPos().y;
	add_me_packet.pos_z = clients[client_id].getPos().z;

	add_me_packet.right_vec_x = clients[client_id].getCoordinate().x_coordinate.x;
	add_me_packet.right_vec_y = clients[client_id].getCoordinate().x_coordinate.y;
	add_me_packet.right_vec_z = clients[client_id].getCoordinate().x_coordinate.z;

	add_me_packet.up_vec_x = clients[client_id].getCoordinate().y_coordinate.x;
	add_me_packet.up_vec_y = clients[client_id].getCoordinate().y_coordinate.y;
	add_me_packet.up_vec_z = clients[client_id].getCoordinate().y_coordinate.z;

	add_me_packet.look_vec_x = clients[client_id].getCoordinate().z_coordinate.x;
	add_me_packet.look_vec_y = clients[client_id].getCoordinate().z_coordinate.y;
	add_me_packet.look_vec_z = clients[client_id].getCoordinate().z_coordinate.z;

	for (int i = 0; i < MAX_USER; i++) {
		if (clients[i].getState() == CL_STATE_EMPTY) continue;
		if (i == client_id) continue;

		clients[i].sendAddObjPacket(add_me_packet);
	}

	// 현재 접속해 있는 모든 클라이언트들에게 아이템 박스 정보를 전달합니다.
	for (int i{}; i < ITEMBOXNUM; i++) {

		GS2C_ADD_OBJ_PACKET add_itembox_packet;
		add_itembox_packet.size = sizeof(GS2C_ADD_OBJ_PACKET);
		add_itembox_packet.type = GS2C_ADD_OBJ;
		add_itembox_packet.id = i;
		add_itembox_packet.objtype = OBJ_TYPE_ITEMBOX;

		add_itembox_packet.pos_x = ItemBoxArray[i].m_pos.x;
		add_itembox_packet.pos_y = ItemBoxArray[i].m_pos.y;
		add_itembox_packet.pos_z = ItemBoxArray[i].m_pos.z;

		for (int j = 0; j < MAX_USER; j++) {
			if (clients[j].getState() == CL_STATE_EMPTY) continue;

			clients[j].sendAddObjPacket(add_itembox_packet);
		}
	}

	//==================================================
	// Loop - Recv & Process Packets
	//==================================================
	while (1) {
		// 이동 함수
		C2GS_KEYVALUE_PACKET ClientPushKey;
		retval = recv(client_sock, reinterpret_cast<char*>(&ClientPushKey), sizeof(C2GS_KEYVALUE_PACKET), 0);
		if (retval == SOCKET_ERROR) {
			//err_display("recv()");
			closesocket(client_sock);
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

					// client_id번째 클라이언트 객체의 변경사항을 보낼 패킷에 담습니다.
					sendUpdatePacket_toAllClient(client_id);

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

					// client_id번째 클라이언트 객체의 변경사항을 보낼 패킷에 담습니다.
					sendUpdatePacket_toAllClient(client_id);

					break;
				}
				case KEY_SPACE:
				{
					if (clients[client_id].getItemQueue() == -1) // 플레이어가 현재 가진 아이템이 없다면 아무일도 일어나지 않는다.
						break;

					switch (clients[client_id].getItemQueue()) {
					case ITEM_Missile:
					case ITEM_Bomb:
					{
						// 서버에서 관리하는 아이템객체(미사일, 지뢰) 컨테이너에 새롭게 추가되는 객체 정보 저장
						ItemObject temp;

						temp.setObjType(clients[client_id].getItemQueue());
						temp.setObjOwner(client_id);
						temp.setPos(clients[client_id].getPos());
						temp.setYaw(clients[client_id].getYaw());
						temp.setRoll(clients[client_id].getRoll());
						temp.setPitch(clients[client_id].getPitch());
						temp.setCoordinate(clients[client_id].getCoordinate());

						ObjectManager.push_back(temp);

						// 새롭게 추가되는 객체 정보를 모든 클라이언트에게 전달
						GS2C_ADD_OBJ_PACKET add_obj_packet;

						add_obj_packet.size = sizeof(GS2C_ADD_OBJ_PACKET);
						add_obj_packet.type = GS2C_ADD_OBJ;
						add_obj_packet.id = clients[client_id].getId();
						add_obj_packet.objtype = clients[client_id].getItemQueue();

						add_obj_packet.pos_x = temp.getPos().x;
						add_obj_packet.pos_y = temp.getPos().y;
						add_obj_packet.pos_z = temp.getPos().z;

						add_obj_packet.right_vec_x = temp.getCoordinate().x_coordinate.x;
						add_obj_packet.right_vec_y = temp.getCoordinate().x_coordinate.y;
						add_obj_packet.right_vec_z = temp.getCoordinate().x_coordinate.z;

						add_obj_packet.up_vec_x = temp.getCoordinate().y_coordinate.x;
						add_obj_packet.up_vec_y = temp.getCoordinate().y_coordinate.y;
						add_obj_packet.up_vec_z = temp.getCoordinate().y_coordinate.z;

						add_obj_packet.look_vec_x = temp.getCoordinate().z_coordinate.x;
						add_obj_packet.look_vec_y = temp.getCoordinate().z_coordinate.y;
						add_obj_packet.look_vec_z = temp.getCoordinate().z_coordinate.z;

						for (int i = 0; i < MAX_USER; i++) {
							if (clients[i].getState() == CL_STATE_EMPTY) continue;

							clients[i].sendAddObjPacket(add_obj_packet);
						}
						break;
					}
					case ITEM_Booster:
					{
						// 부스터는 나중에...
						break;
					}
					//CaseEnd
					}
					//SwitchEnd

					break;
				}
				//CaseEnd
				}
				//SwitchEnd

			}
		}
	}
	//==================================================

	// 소켓 닫기
	closesocket(client_sock);
	cout << "[TCP 서버] 클라이언트 종료: IP 주소= " << addr << ", 포트 번호 = " << ntohs(clientaddr.sin_port) << endl;
	return 0;
}
//==================================================

//==================================================
//            [ 타이머 스레드 함수 ]
//      일정 주기마다 수행해야하는 작업들은
//             이곳에서 처리합니다.
//==================================================
DWORD WINAPI TimerThreadFunc(LPVOID arg)
{
	while (1) {
		Sleep(100);
	}
}
//==================================================

//==================================================
//   [ 서버 시간을 흐르게 해주는 스레드 함수 ]
// 		서버 시간을 흐르게 하는 코드입니다.
//		특별한 일 없으면 건들지 말아주세요.
//==================================================
DWORD WINAPI ServerTime_Update(LPVOID arg)
{
	START_TIME = (float)clock() / CLOCKS_PER_SEC;
	while (1) {
		SERVER_TIME = (float)clock() / CLOCKS_PER_SEC - START_TIME;	// 서버 시간 업데이트
		Sleep(TIME_UPDATE_CYCLE);									// 0.2초 대기 (서버 시간 업데이트 주기)
	}
}
//==================================================

