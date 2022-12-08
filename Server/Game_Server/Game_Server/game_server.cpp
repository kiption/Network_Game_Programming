#include "../../Common/protocol.h"
#include "../../Common/Common.h"

#include <iostream>
#include <array>
#include <vector>
#include <queue>
#include <time.h>
#include "CalcMove.h"
#include "Global.h"
#include "Collision.h"

using namespace std;

CRITICAL_SECTION critical_section;
DWORD WINAPI ProcessClient(LPVOID arg);				// 클라이언트 통신 스레드
DWORD WINAPI TimerThreadFunc(LPVOID arg);			// 타이머 스레드
DWORD WINAPI ServerTime_Update(LPVOID arg);			// 서버 시간 갱신 스레드
DWORD WINAPI CollideCheck_ThreadFunc(LPVOID arg);	// 충돌 검사 스레드

Coordinate basic_coordinate;				// 기본(초기) 좌표계

float START_TIME;							// 서버 프로그램이 켜진 시간
float SERVER_TIME;							// 서버 시간
constexpr int TIME_UPDATE_CYCLE = 100;		// 서버 시간 업데이트 주기 (ms단위)

void collisioncheck_Player2ItemBox(int client_id);
void collisioncheck_Player2Player(int client_id);
void collisioncheck_Player2Missile(int client_id);
//==================================================
//           [ 클라이언트 객체 정보 ]
//==================================================
enum { CL_STATE_EMPTY, CL_STATE_RUNNING };
class ClientINFO {
private:
	SOCKET		m_sock;

	int			m_id = 0;
	char		m_state;

	float		m_timer;
	float		m_accelerator;
	float		m_limit_acc;
	Coordinate	m_coordinate;
	queue<int>  m_myitem;

	bool		m_item_cooldown;	// 아이템 사용 쿨타임
	bool		m_booster_on;		// 부스터 사용 여부
	bool		m_lose_control;		// 조작 가능 여부 (충돌 연출때에는 조작이 불가능합니다.)

	bool		m_reduce_acc;

public:
	float		m_yaw, m_pitch, m_roll;
	MyVector3D	m_pos;
	BoundingOrientedBox xoobb;

public:
	// Initialize
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
		m_accelerator = 0.0f;
		m_limit_acc = LIMIT_ACCELERATOR;
		xoobb = { XMFLOAT3(m_pos.x,m_pos.y,m_pos.z), XMFLOAT3(6.0f,6.0f,6.0f), XMFLOAT4(0.0f,0.0f,0.0f,1.0f) };

		m_item_cooldown = false;
		m_booster_on = false;
		m_lose_control = false;
	};

public:
	// Accessor Func
	// 1. Get
	SOCKET		getSock() { return m_sock; }
	char		getState() { return m_state; }
	int			getId() { return m_id; }
	float		getAccel() { return m_accelerator; }
	float		getLimitAcc() { return m_limit_acc; }
	MyVector3D	getPos() { return m_pos; }
	Coordinate	getCoordinate() { return m_coordinate; }
	float		getPitch() { return m_pitch; }
	float		getYaw() { return m_yaw; }
	float		getRoll() { return m_roll; }
	int			getItemQueue() {
		if (m_myitem.empty()) return -1;	// 큐가 비어있으면 -1을
		return m_myitem.front();			// 아이템이 있다면 아이템의 고유번호를 반환합니다.
	}
	int			getHowManyItem() { return m_myitem.size(); }
	bool		getItemCooldown() { return m_item_cooldown; }
	bool		getBoosterOn() { return m_booster_on; }
	bool		getLoseControl() { return m_lose_control; }
	bool		getReduceAcc() { return m_reduce_acc; }

	// 2. Set
	void		setSock(SOCKET sock) { m_sock = sock; }
	void		setState(char state) { m_state = state; }
	void		setAccel(float accel) { m_accelerator = accel; }
	void		setLimitAcc(float acc) { m_limit_acc = acc; }
	void		setID(int id) { m_id = id; }
	void		setPos(MyVector3D pos) { m_pos = pos; }
	void		setCoordinate(Coordinate co) { m_coordinate = co; }
	void		setCoordinate(MyVector3D x, MyVector3D y, MyVector3D z) { m_coordinate.x_coordinate = x; m_coordinate.y_coordinate = y; m_coordinate.z_coordinate = z; }
	void		setPitch(float f) { m_pitch = f; }
	void		setYaw(float f) { m_yaw = f; }
	void		setRoll(float f) { m_roll = f; }
	void		setItemQueue(int type) { m_myitem.push(type); } // 아이템 먹을 시에 사용
	void		setItemRelease() { m_myitem.pop(); } // 사용한 아이템 방출
	void		setItemCooldown(bool b) { m_item_cooldown = b; }
	void		setBoosterOn(bool b) { m_booster_on = b; }
	void		setLoseControl(bool b) { m_lose_control = b; }
	void		setReduceAcc(bool b) { m_reduce_acc = b; }

public:
	// Networking Func
	void		sendLoginInfoPacket(GS2C_LOGIN_INFO_PACKET packet);
	void		sendAddObjPacket(GS2C_ADD_OBJ_PACKET packet);
	void		sendUpdatePacket(GS2C_UPDATE_PACKET packet);
	void		sendRemoveObjPacket(GS2C_REMOVE_OBJ_PACKET packet);
};
array<ClientINFO, MAX_USER> clients;
//==================================================

//==================================================
//             [ 아이템 객체 정보 ]
//==================================================
enum { ITEM_Booster, ITEM_Missile, ITEM_Bomb };
constexpr int ITEM_VARIETY = 3; // 아이템 종류 수
class ItemObject {
	int			m_id;
	int			m_objtype;
	int			m_objOwner;
	float		m_yaw, m_pitch, m_roll;
	MyVector3D	m_pos;
	Coordinate	m_coordinate;
	bool		m_running;
public:
	BoundingOrientedBox xoobb;
public:
	ItemObject() {
		m_id = -1;
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
		m_running = false;
	};
	int			getID() { return m_id; }
	int			getObjType() { return m_objtype; }
	int			getObjOwner() { return m_objOwner; }
	MyVector3D	getPos() { return m_pos; }
	Coordinate	getCoordinate() { return m_coordinate; }
	float		getPitch() { return m_pitch; }
	float		getYaw() { return m_yaw; }
	float		getRoll() { return m_roll; }
	bool		getRunning() { return m_running; }

	void		setID(int id) { m_id = id; }
	void		setObjType(int type) { m_objtype = type; }
	void		setObjOwner(int num) { m_objOwner = num; }
	void		setPos(MyVector3D pos) { m_pos = pos; }
	void		setCoordinate(Coordinate co) { m_coordinate = co; }
	void		setCoordinate(MyVector3D x, MyVector3D y, MyVector3D z) { m_coordinate.x_coordinate = x; m_coordinate.y_coordinate = y; m_coordinate.z_coordinate = z; }
	void		setPitch(float f) { m_pitch = f; }
	void		setYaw(float f) { m_yaw = f; }
	void		setRoll(float f) { m_roll = f; }
	void		setRunning(bool b) { m_running = b; }

	void		returnToInitialState() {	// 초기상태로 복구하는 함수
		m_id = -1;
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
		m_running = false;
	}
};
array<ItemObject, MissileNum> MissileArray;
array<ItemObject, BombNum> BombArray;
//==================================================

//==================================================
//           [ 아이템 박스 객체 정보 ]
//==================================================
struct ItemBox {
	float		m_yaw, m_pitch, m_roll; // 회전 각도
	MyVector3D	m_pos;
	Coordinate	m_coordinate; // 회전 행렬
	bool		m_visible = true;
	BoundingOrientedBox xoobb;
};
array<ItemBox, ITEMBOXNUM> ItemBoxArray;
//==================================================

//==================================================
//            [ 서버 이벤트 관련 ]
//==================================================
constexpr int EV_TYPE_QUEUE_ERROR = 99; // type error
enum { EV_TYPE_REFRESH, EV_TYPE_MOVE, EV_TYPE_ROTATE, EV_TYPE_REMOVE, EV_TYPE_HIT };											// 이벤트 타입
enum { EV_TARGET_CLIENTS, EV_TARGET_MISSILE, EV_TARGET_BOMB, EV_TARGET_ITEMBOX };												// 이벤트 적용 대상
enum { EV_DTARGET_NONE, EV_DTARGET_ITEMCOOLDOWN, EV_DTARGET_BOOSTER, EV_DTARGET_CONTROL, EV_DTARGET_ACC, EV_DTARGET_BOOSTEND };	// Extra Info
constexpr int EV_DTARGET_ALL = 999;	// Extra Info
struct ServerEvent {	// 타이머스레드에서 처리할 이벤트
	// setServerEvent함수 인자에 입력해야하는 정보
	char	ev_type;														// 이벤트 종류
	float	ev_duration;													// 이벤트 지속시간 (이벤트 종료조건을 수동으로 주고싶다면 0을 넣고, flag에 NoCount를 넣어주세요.)
	char	ev_target;														// 이벤트 적용 대상
	char	ev_target_detail;												// 세부 적용 대상
	int		target_num;														// 적용 대상이 배열, 벡터 등에서 몇번째 칸에 있는 지
	int		extra_info;														// 추가적인 정보가 필요한 경우 입력하세요.

	// setServerEvent함수를 통해 자동으로 입력되는 정보
	float	ev_start_time;
	bool	auto_ev_end;
};
queue<ServerEvent> ServerEventQueue;		// 서버 이벤트 큐

enum { NoFlag, NoCount, SetStartTimeToExInfo };
void setServerEvent(char type, float sec, char target, char target_detail, int t_num, int ex_info, char flag) {
	ServerEvent* temp = new ServerEvent;
	temp->ev_type = type;
	temp->ev_duration = sec;
	temp->ev_target = target;
	temp->ev_target_detail = target_detail;
	temp->target_num = t_num;
	temp->extra_info = ex_info;

	if (flag == SetStartTimeToExInfo)
		temp->ev_start_time = ex_info;
	else
		temp->ev_start_time = SERVER_TIME;

	if (flag == NoCount)
		temp->auto_ev_end = false;
	else
		temp->auto_ev_end = true;


	ServerEventQueue.push(*temp);
}
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
void ClientINFO::sendRemoveObjPacket(GS2C_REMOVE_OBJ_PACKET packet) {
	int retval = send(m_sock, (char*)&packet, sizeof(GS2C_REMOVE_OBJ_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		//err_display("send()");
	}
}

void sendPlayerUpdatePacket_toAllClient(int c_id) {	// 모든 클라이언트에게 c_id번째 클라이언트의 업데이트 정보를 보내는 함수
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
void sendItemBoxUpdatePacket_toAllClient(int itembox_id) {	// 모든 클라이언트에게 c_id번째 클라이언트의 업데이트 정보를 보내는 함수
	GS2C_UPDATE_PACKET update_packet;
	// itembox_id번째 아이템박스 객체의 변경사항을 보낼 패킷에 담습니다.
	update_packet.id = itembox_id;
	update_packet.type = GS2C_UPDATE;
	update_packet.objtype = OBJ_TYPE_ITEMBOX;

	update_packet.pos_x = ItemBoxArray[itembox_id].m_pos.x;
	update_packet.pos_y = ItemBoxArray[itembox_id].m_pos.y;
	update_packet.pos_z = ItemBoxArray[itembox_id].m_pos.z;

	update_packet.right_vec_x = ItemBoxArray[itembox_id].m_coordinate.x_coordinate.x;
	update_packet.right_vec_y = ItemBoxArray[itembox_id].m_coordinate.x_coordinate.y;
	update_packet.right_vec_z = ItemBoxArray[itembox_id].m_coordinate.x_coordinate.z;

	update_packet.up_vec_x = ItemBoxArray[itembox_id].m_coordinate.y_coordinate.x;
	update_packet.up_vec_y = ItemBoxArray[itembox_id].m_coordinate.y_coordinate.y;
	update_packet.up_vec_z = ItemBoxArray[itembox_id].m_coordinate.y_coordinate.z;

	update_packet.look_vec_x = ItemBoxArray[itembox_id].m_coordinate.z_coordinate.x;
	update_packet.look_vec_y = ItemBoxArray[itembox_id].m_coordinate.z_coordinate.y;
	update_packet.look_vec_z = ItemBoxArray[itembox_id].m_coordinate.z_coordinate.z;

	// itembox_id번째 아이템박스 객체의 변경된 사항을 모든 클라이언트들에게 전달합니다.
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
	// 임계영역 초기화
	InitializeCriticalSection(&critical_section);

	// 서버 시간 초기화
	START_TIME = 0.0f;
	SERVER_TIME = 0.0f;

	// 서버시간을 업데이트시켜주는 스레드 생성
	HANDLE hTimeUpdateThread = CreateThread(NULL, 0, ServerTime_Update, 0, 0, NULL);

	// 주기적인 작업을 수행하는 타이머 스레드 생성
	HANDLE hTimerThreadThread = CreateThread(NULL, 0, TimerThreadFunc, 0, 0, NULL);

	// 충돌 검사를 수행하는 스레드 생성
	HANDLE hCollideThreadThread = CreateThread(NULL, 0, CollideCheck_ThreadFunc, 0, 0, NULL);


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
	// 아이템 박스의 bb를 설정합니다.
	for (int i = 0; i < ITEMBOXNUM; i++) {
		ItemBoxArray[i].xoobb = BoundingOrientedBox(XMFLOAT3(ItemBoxArray[i].m_pos.x, ItemBoxArray[i].m_pos.y, ItemBoxArray[i].m_pos.z),
			XMFLOAT3(6.0f, 6.0f, 6.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		cout << "Create itembox's oobb - " << i << endl;
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

	// 임계영역 삭제
	DeleteCriticalSection(&critical_section);

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
	clients[client_id].xoobb = BoundingOrientedBox(XMFLOAT3(clients[client_id].getPos().x, clients[client_id].getPos().y, clients[client_id].getPos().z),
		XMFLOAT3(6.0f, 6.0f, 6.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

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
	// 아이템 박스가 제자리에서 계속 회전을 하도록 타이머에 이벤트로 넣어줍니다.
	setServerEvent(EV_TYPE_ROTATE, INFINITY, EV_TARGET_ITEMBOX, EV_DTARGET_ALL, 0, 0, NoCount);

	//==================================================
	// Loop - Recv & Process Packets
	//==================================================
	while (1) {

		PACKET_INFO recv_info;
		retval = recv(client_sock, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK을 사용하여 수신버퍼를 읽지만 가져오지는 않도록
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}
		else if (retval == 0) {
			break;
		}

		switch (recv_info.type) {
		case C2GS_KEYVALUE:
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
						// 제어권을 잃은 상태에선 조작이 불가능 합니다.
						if (clients[client_id].getLoseControl())
							break;

						// yaw 설정
						float temp_yaw = clients[client_id].getYaw() + ROTATE_SCALAR * plus_minus * PI / 360.0f;
						if (temp_yaw >= 360.0f)
							temp_yaw -= 360.0f;
						clients[client_id].setYaw(temp_yaw);

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
						sendPlayerUpdatePacket_toAllClient(client_id);

						break;
					}
					case KEY_DOWN:
						plus_minus = -1;
						[[fallthrough]];
					case KEY_UP:
					{
						// 제어권을 잃은 상태에선 조작이 불가능 합니다.
						if (clients[client_id].getLoseControl())
							break;

						clients[client_id].setReduceAcc(false);
						float getacc = clients[client_id].getAccel();

						if (getacc <= 0.0f) {
							getacc = 0.015f;
							clients[client_id].setAccel(getacc);
							//cout << getacc << endl;
						}
						else if (getacc >= clients[client_id].getLimitAcc()) {
							getacc = clients[client_id].getLimitAcc();
							clients[client_id].setAccel(getacc);
							//cout << getacc << endl;
						}
						else {
							if (clients[client_id].getBoosterOn()) {
								getacc += 0.05f;
							}
							else {
								getacc += 0.018f;
							}
							clients[client_id].setAccel(getacc);
							//cout << getacc << endl;
						}


						MyVector3D move_dir{ 0, 0, 0 };
						move_dir = { clients[client_id].getCoordinate().z_coordinate.x * plus_minus,
							clients[client_id].getCoordinate().z_coordinate.y * plus_minus, clients[client_id].getCoordinate().z_coordinate.z * plus_minus };

						MyVector3D Move_Vertical_Result{ 0,0,0 };
						Move_Vertical_Result = calcMove(clients[client_id].getPos(), move_dir, clients[client_id].getAccel());

						// 좌표 업데이트
						clients[client_id].setPos(Move_Vertical_Result);
						// BB 업데이트
						clients[client_id].xoobb = BoundingOrientedBox(XMFLOAT3(clients[client_id].getPos().x, clients[client_id].getPos().y, clients[client_id].getPos().z),
							XMFLOAT3(6.0f, 6.0f, 6.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

						// 클라이언트에게 전달
						sendPlayerUpdatePacket_toAllClient(client_id);

						break;
					}
					case KEY_SPACE:
					{
						// 제어권을 잃은 상태에선 조작이 불가능 합니다.
						if (clients[client_id].getLoseControl())
							break;

						
						if (clients[client_id].getItemCooldown())		// 아이템 사용 쿨타임 상태라면 아무일도 일어나지 않는다.
							break;
						if (clients[client_id].getItemQueue() == -1)	// 플레이어가 현재 가진 아이템이 없다면 아무일도 일어나지 않는다.
							break;

						EnterCriticalSection(&critical_section);
						int used_item = clients[client_id].getItemQueue();

						clients[client_id].setItemCooldown(true);
						setServerEvent(EV_TYPE_REFRESH, ITEM_COOLDOWN_DURATION, EV_TARGET_CLIENTS, EV_DTARGET_ITEMCOOLDOWN, client_id, 0, 0);	// 아이템 사용 쿨타임

						switch (used_item) {
						case ITEM_Booster:
						{
							if (clients[client_id].getBoosterOn())	// 이미 부스터가 켜져있으면 사용할 수 없습니다.
								break;

							clients[client_id].setItemRelease();
							cout << "Use Item[Booster, " << used_item << "]." << endl;

							LeaveCriticalSection(&critical_section);

							clients[client_id].setBoosterOn(true);
							clients[client_id].setLimitAcc(BOOSTER_ACCELERATOR);
							setServerEvent(EV_TYPE_REFRESH, BOOSTER_DURATION, EV_TARGET_CLIENTS, EV_DTARGET_BOOSTER, client_id, 0, 0);	// 부스터 지속시간

							break;
						}
						case ITEM_Missile:
						{
							clients[client_id].setItemRelease();
							cout << "Use Item[Missile, " << used_item << "]." << endl;

							// id 할당
							int missile_id = -1;
							for (int i = 0; i < MissileNum; i++) {
								if (!MissileArray[i].getRunning()) {	// 빈칸을 찾았으면 id를 설정하고, for루프를 빠져나옵니다.
									missile_id = i;
									break;
								}
							}
							cout << "missile id: " << missile_id << endl;//test

							// 새롭게 추가되는 미사일의 정보 저장
							MissileArray[missile_id].setID(missile_id);
							MissileArray[missile_id].setObjType(used_item);
							MissileArray[missile_id].setObjOwner(client_id);
							MyVector3D missile_first_pos = clients[client_id].getPos();
							MyVector3D missile_lookvec = clients[client_id].getCoordinate().z_coordinate;
							MyVector3D missile_final_pos = calcMove(missile_first_pos, missile_lookvec, 10.f);
							MissileArray[missile_id].setPos(missile_final_pos);
							MissileArray[missile_id].setYaw(clients[client_id].getYaw());
							MissileArray[missile_id].setRoll(clients[client_id].getRoll());
							MissileArray[missile_id].setPitch(clients[client_id].getPitch());
							MissileArray[missile_id].setCoordinate(clients[client_id].getCoordinate());
							MissileArray[missile_id].setRunning(true);
							MissileArray[missile_id].xoobb
								= BoundingOrientedBox(XMFLOAT3(MissileArray[missile_id].getPos().x, MissileArray[missile_id].getPos().y, MissileArray[missile_id].getPos().z)
								, XMFLOAT3(6.0f, 6.0f, 6.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

							LeaveCriticalSection(&critical_section);

							// 새롭게 추가되는 미사일 객체 정보를 모든 클라이언트에게 전달
							GS2C_ADD_OBJ_PACKET add_missile_packet;

							add_missile_packet.size = sizeof(GS2C_ADD_OBJ_PACKET);
							add_missile_packet.type = GS2C_ADD_OBJ;
							add_missile_packet.id = MissileArray[missile_id].getID();
							add_missile_packet.objtype = used_item;

							add_missile_packet.pos_x = MissileArray[missile_id].getPos().x;
							add_missile_packet.pos_y = MissileArray[missile_id].getPos().y;
							add_missile_packet.pos_z = MissileArray[missile_id].getPos().z;

							add_missile_packet.right_vec_x = MissileArray[missile_id].getCoordinate().x_coordinate.x;
							add_missile_packet.right_vec_y = MissileArray[missile_id].getCoordinate().x_coordinate.y;
							add_missile_packet.right_vec_z = MissileArray[missile_id].getCoordinate().x_coordinate.z;

							add_missile_packet.up_vec_x = MissileArray[missile_id].getCoordinate().y_coordinate.x;
							add_missile_packet.up_vec_y = MissileArray[missile_id].getCoordinate().y_coordinate.y;
							add_missile_packet.up_vec_z = MissileArray[missile_id].getCoordinate().y_coordinate.z;

							add_missile_packet.look_vec_x = MissileArray[missile_id].getCoordinate().z_coordinate.x;
							add_missile_packet.look_vec_y = MissileArray[missile_id].getCoordinate().z_coordinate.y;
							add_missile_packet.look_vec_z = MissileArray[missile_id].getCoordinate().z_coordinate.z;

							for (int i = 0; i < MAX_USER; i++) {
								if (clients[i].getState() == CL_STATE_EMPTY) continue;

								clients[i].sendAddObjPacket(add_missile_packet);
							}

							// 미사일의 타이머 설정
							setServerEvent(EV_TYPE_MOVE, MISSILE_DURATION - 0.5f, EV_TARGET_MISSILE, 0, missile_id, 0, 0); // 미사일 이동

							break;
						}
						case ITEM_Bomb:
						{
							clients[client_id].setItemRelease();
							cout << "Use Item[Bomb, " << used_item << "]." << endl;
							
							// id 할당
							int bomb_id = -1;
							for (int i = 0; i < BombNum; i++) {
								if (!BombArray[i].getRunning()) {	// 빈칸을 찾았으면 id를 설정하고, for루프를 빠져나옵니다.
									bomb_id = i;
									break;
								}
							}

							// 새롭게 추가되는 지뢰의 정보 저장
							BombArray[bomb_id].setID(bomb_id);
							BombArray[bomb_id].setObjType(used_item);
							BombArray[bomb_id].setObjOwner(client_id);
							MyVector3D bomb_first_pos = clients[client_id].getPos();
							MyVector3D bomb_lookvec = clients[client_id].getCoordinate().z_coordinate;
							MyVector3D bomb_final_pos = calcMove(bomb_first_pos, bomb_lookvec, 10.f);
							BombArray[bomb_id].setPos(bomb_final_pos);
							BombArray[bomb_id].setYaw(clients[client_id].getYaw());
							BombArray[bomb_id].setRoll(clients[client_id].getRoll());
							BombArray[bomb_id].setPitch(clients[client_id].getPitch());
							BombArray[bomb_id].setCoordinate(clients[client_id].getCoordinate());
							BombArray[bomb_id].setRunning(true);
							LeaveCriticalSection(&critical_section);

							// 새롭게 추가되는 미사일 객체 정보를 모든 클라이언트에게 전달
							GS2C_ADD_OBJ_PACKET add_bomb_packet;

							add_bomb_packet.size = sizeof(GS2C_ADD_OBJ_PACKET);
							add_bomb_packet.type = GS2C_ADD_OBJ;
							add_bomb_packet.id = BombArray[bomb_id].getID();
							add_bomb_packet.objtype = used_item;

							add_bomb_packet.pos_x = BombArray[bomb_id].getPos().x;
							add_bomb_packet.pos_y = BombArray[bomb_id].getPos().y;
							add_bomb_packet.pos_z = BombArray[bomb_id].getPos().z;

							add_bomb_packet.right_vec_x = BombArray[bomb_id].getCoordinate().x_coordinate.x;
							add_bomb_packet.right_vec_y = BombArray[bomb_id].getCoordinate().x_coordinate.y;
							add_bomb_packet.right_vec_z = BombArray[bomb_id].getCoordinate().x_coordinate.z;

							add_bomb_packet.up_vec_x = BombArray[bomb_id].getCoordinate().y_coordinate.x;
							add_bomb_packet.up_vec_y = BombArray[bomb_id].getCoordinate().y_coordinate.y;
							add_bomb_packet.up_vec_z = BombArray[bomb_id].getCoordinate().y_coordinate.z;

							add_bomb_packet.look_vec_x = BombArray[bomb_id].getCoordinate().z_coordinate.x;
							add_bomb_packet.look_vec_y = BombArray[bomb_id].getCoordinate().z_coordinate.y;
							add_bomb_packet.look_vec_z = BombArray[bomb_id].getCoordinate().z_coordinate.z;

							for (int i = 0; i < MAX_USER; i++) {
								if (clients[i].getState() == CL_STATE_EMPTY) continue;

								clients[i].sendAddObjPacket(add_bomb_packet);
							}

							// 지뢰의 타이머 설정
							setServerEvent(EV_TYPE_REMOVE, BOMB_DURATION, EV_TARGET_BOMB, 0, bomb_id, 0, 0);
							break;
						}
						default:
							LeaveCriticalSection(&critical_section);
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
			break;
		case C2GS_KEYUPVALUE:
			C2GS_KEYUPVALUE_PACKET ClientUpKey;
			retval = recv(client_sock, reinterpret_cast<char*>(&ClientUpKey), sizeof(C2GS_KEYUPVALUE_PACKET), 0);
			if (retval == SOCKET_ERROR) {
				//err_display("recv()");
				closesocket(client_sock);
			}

			clients[client_id].setReduceAcc(true);
			setServerEvent(EV_TYPE_REFRESH, 0, EV_TARGET_CLIENTS, EV_DTARGET_ACC, client_id, 0, 0);

			break;
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
		EnterCriticalSection(&critical_section);	// 임계영역 진입
		ServerEvent *new_event = new ServerEvent;
		if (ServerEventQueue.empty()) {
			LeaveCriticalSection(&critical_section);	// 임계영역 탈출
			continue;
		}
		else {
			*new_event = ServerEventQueue.front();
			ServerEventQueue.pop();
		}
		
		int target = new_event->target_num;
		switch (new_event->ev_type) {
		case EV_TYPE_REFRESH:
		{
			if (SERVER_TIME >= new_event->ev_start_time + new_event->ev_duration) {
				// 이벤트 시간이 끝났다면 타겟 타입에 맞는 후처리 작업을 해줍니다.
				switch (new_event->ev_target) {
				case EV_TARGET_CLIENTS:
					if (new_event->ev_target_detail == EV_DTARGET_CONTROL) {
						clients[target].setLoseControl(false);
						cout << "Client[ " << target << "] Gets Control Back" << endl;
					}
					else if (new_event->ev_target_detail == EV_DTARGET_ITEMCOOLDOWN) {
						clients[target].setItemCooldown(false);
						cout << "Client[ " << target << "]'s Item Cooldown is End." << endl;
					}
					else if (new_event->ev_target_detail == EV_DTARGET_BOOSTER) {
						setServerEvent(EV_TYPE_REFRESH, 0.1, EV_TARGET_CLIENTS, EV_DTARGET_BOOSTEND, target, 0, 0);
					}
					else if (new_event->ev_target_detail == EV_DTARGET_BOOSTEND) {
						// 부스터가 끝났을 때 가속도 상한을 원래 수준으로 서서히 낮춰줍니다.
						if (clients[target].getAccel() <= LIMIT_ACCELERATOR) {
							clients[target].setBoosterOn(false);
							clients[target].setLimitAcc(LIMIT_ACCELERATOR);
							cout << "Client[ " << target << "]'s Booster is End." << endl;
							break;
						}
						clients[target].setAccel(clients[target].getAccel() - 0.6);
						clients[target].setLimitAcc(clients[target].getAccel());

						setServerEvent(EV_TYPE_REFRESH, 0.1, EV_TARGET_CLIENTS, EV_DTARGET_BOOSTEND, target, 0, 0);
					}
					else if (new_event->ev_target_detail == EV_DTARGET_ACC) {
						// 이동키를 떼었을때 가속도를 점점 감소시킵니다.
						if (clients[target].getAccel() <= 0) {
							clients[target].setAccel(0.0f);
							break;
						}
						if (!clients[target].getReduceAcc()) {
							break;
						}

						clients[target].setAccel(clients[target].getAccel() - 0.8);

						setServerEvent(EV_TYPE_REFRESH, 0, EV_TARGET_CLIENTS, EV_DTARGET_ACC, target, 0, 0);
					}
					else {
						cout << "[Event Error] Unknown Event's Extra Info." << endl;
					}
					break;
				case EV_TARGET_ITEMBOX:
					cout << "ItemBox[" << target << "] is Refreshed." << endl;
					ItemBoxArray[target].m_pos.y = 20.0f;
					ItemBoxArray[target].m_visible = true;

					// 아이템 박스의 변경사항을 모든 클라이언트에게 전달합니다.
					sendItemBoxUpdatePacket_toAllClient(target);
					break;
				}
			}
			else {
				// 아직 이벤트 시간이 끝나지 않았다면 정보를 그대로 유지한채 이벤트 큐에 다시 넣어줍니다.
				setServerEvent(new_event->ev_type, new_event->ev_duration, new_event->ev_target, new_event->ev_target_detail, new_event->target_num
					, new_event->ev_start_time, SetStartTimeToExInfo);
			}
			break;
		}
		case EV_TYPE_MOVE:
		{
			switch (new_event->ev_target) {
			case EV_TARGET_MISSILE:
				if (SERVER_TIME < new_event->ev_start_time + new_event->ev_duration) {	// 지속시간이 끝날 때까지 지속적으로 움직입니다.
					// 미사일을 앞으로 움직입니다.
					MyVector3D move_dir{ 0, 0, 0 };
					move_dir = { MissileArray[target].getCoordinate().z_coordinate.x,
								 MissileArray[target].getCoordinate().z_coordinate.y,
								 MissileArray[target].getCoordinate().z_coordinate.z };

					MyVector3D Missile_Move_Result = calcMove(MissileArray[target].getPos(), move_dir, MISSILE_MOVE_SCALAR);

					// 변경된 좌표로 업데이트합니다.
					MissileArray[target].setPos(Missile_Move_Result);
					// BB 업데이트
					MissileArray[target].xoobb
						= BoundingOrientedBox(XMFLOAT3(MissileArray[target].getPos().x, MissileArray[target].getPos().y, MissileArray[target].getPos().z)
							, XMFLOAT3(6.0f, 6.0f, 6.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

					// client_id번째 클라이언트 객체의 변경사항을 보낼 패킷에 담습니다.
					GS2C_UPDATE_PACKET missile_update_pack;

					missile_update_pack.id = target;
					missile_update_pack.type = GS2C_UPDATE;
					missile_update_pack.objtype = OBJ_TYPE_MISSILE;

					missile_update_pack.pos_x = MissileArray[target].getPos().x;
					missile_update_pack.pos_y = MissileArray[target].getPos().y;
					missile_update_pack.pos_z = MissileArray[target].getPos().z;

					missile_update_pack.right_vec_x = MissileArray[target].getCoordinate().x_coordinate.x;
					missile_update_pack.right_vec_y = MissileArray[target].getCoordinate().x_coordinate.y;
					missile_update_pack.right_vec_z = MissileArray[target].getCoordinate().x_coordinate.z;

					missile_update_pack.up_vec_x = MissileArray[target].getCoordinate().y_coordinate.x;
					missile_update_pack.up_vec_y = MissileArray[target].getCoordinate().y_coordinate.y;
					missile_update_pack.up_vec_z = MissileArray[target].getCoordinate().y_coordinate.z;

					missile_update_pack.look_vec_x = MissileArray[target].getCoordinate().z_coordinate.x;
					missile_update_pack.look_vec_y = MissileArray[target].getCoordinate().z_coordinate.y;
					missile_update_pack.look_vec_z = MissileArray[target].getCoordinate().z_coordinate.z;

					for (int i = 0; i < MAX_USER; i++) {
						if (clients[i].getState() == CL_STATE_EMPTY) continue;

						clients[i].sendUpdatePacket(missile_update_pack);
					}

					// Test Log
					//cout << "Missile[" << target << "] moves to Pos(" <<
					//	MissileArray[target].getPos().x << ", " <<
					//	MissileArray[target].getPos().y << ", " <<
					//	MissileArray[target].getPos().z << ")." << endl;

					// 이동을 마치면 다시 큐에 넣습니다.
					setServerEvent(new_event->ev_type, new_event->ev_duration, new_event->ev_target, new_event->ev_target_detail, new_event->target_num
						, new_event->ev_start_time, SetStartTimeToExInfo);
				}
				else {
					// 지속시간이 끝났으면 미사일을 제거합니다.
					MissileArray[target].setPos(MyVector3D{ 0.f, 0.f, 0.f });
					MissileArray[target].xoobb
						= BoundingOrientedBox(XMFLOAT3(MissileArray[target].getPos().x, MissileArray[target].getPos().y, MissileArray[target].getPos().z)
							, XMFLOAT3(6.0f, 6.0f, 6.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
					MissileArray[target].setRunning(false);

					// 제거 패킷을 보냅니다.
					GS2C_REMOVE_OBJ_PACKET missile_remove_pack;

					missile_remove_pack.size = sizeof(GS2C_REMOVE_OBJ_PACKET);
					missile_remove_pack.type = GS2C_REMOVE_OBJ;
					missile_remove_pack.id = target;
					missile_remove_pack.objtype = OBJ_TYPE_MISSILE;

					for (int i = 0; i < MAX_USER; i++) {
						if (clients[i].getState() == CL_STATE_EMPTY) continue;

						clients[i].sendRemoveObjPacket(missile_remove_pack);
					}

					cout << "Missile[" << target << "] is Removed." << endl;//test
				}
				break;
			}
			break;
		}
		case EV_TYPE_ROTATE:
		{
			if (SERVER_TIME < new_event->ev_start_time + new_event->ev_duration) {
				switch (new_event->ev_target) {
				case EV_TARGET_ITEMBOX:
					// 아이템박스를 회전시킵니다.
					// yaw 설정
					for (int i = 0; i < ITEMBOXNUM; i++) {
						float temp_yaw = ItemBoxArray[i].m_yaw + ITEMBOX_ROTATE_SCALAR * PI / 360.0f;
						if (temp_yaw >= 360.0f)
							temp_yaw -= 360.0f;
						ItemBoxArray[i].m_yaw = temp_yaw;

						// right, up, look 벡터 회전계산
						MyVector3D rotate_result_x = calcRotate(basic_coordinate.x_coordinate
							, ItemBoxArray[i].m_roll, ItemBoxArray[i].m_pitch, ItemBoxArray[i].m_yaw);
						MyVector3D rotate_result_y = calcRotate(basic_coordinate.y_coordinate
							, ItemBoxArray[i].m_roll, ItemBoxArray[i].m_pitch, ItemBoxArray[i].m_yaw);
						MyVector3D rotate_result_z = calcRotate(basic_coordinate.z_coordinate
							, ItemBoxArray[i].m_roll, ItemBoxArray[i].m_pitch, ItemBoxArray[i].m_yaw);

						// right, up, look 벡터 회전결과 적용
						ItemBoxArray[i].m_coordinate.x_coordinate = rotate_result_x;
						ItemBoxArray[i].m_coordinate.y_coordinate = rotate_result_y;
						ItemBoxArray[i].m_coordinate.z_coordinate = rotate_result_z;

						// client_id번째 클라이언트 객체의 변경사항을 보낼 패킷에 담습니다.
						GS2C_UPDATE_PACKET itembox_update_pack;

						itembox_update_pack.id = i;
						itembox_update_pack.type = GS2C_UPDATE;
						itembox_update_pack.objtype = OBJ_TYPE_ITEMBOX;

						itembox_update_pack.pos_x = ItemBoxArray[i].m_pos.x;
						itembox_update_pack.pos_y = ItemBoxArray[i].m_pos.y;
						itembox_update_pack.pos_z = ItemBoxArray[i].m_pos.z;

						itembox_update_pack.right_vec_x = ItemBoxArray[i].m_coordinate.x_coordinate.x;
						itembox_update_pack.right_vec_y = ItemBoxArray[i].m_coordinate.x_coordinate.y;
						itembox_update_pack.right_vec_z = ItemBoxArray[i].m_coordinate.x_coordinate.z;

						itembox_update_pack.up_vec_x = ItemBoxArray[i].m_coordinate.y_coordinate.x;
						itembox_update_pack.up_vec_y = ItemBoxArray[i].m_coordinate.y_coordinate.y;
						itembox_update_pack.up_vec_z = ItemBoxArray[i].m_coordinate.y_coordinate.z;

						itembox_update_pack.look_vec_x = ItemBoxArray[i].m_coordinate.z_coordinate.x;
						itembox_update_pack.look_vec_y = ItemBoxArray[i].m_coordinate.z_coordinate.y;
						itembox_update_pack.look_vec_z = ItemBoxArray[i].m_coordinate.z_coordinate.z;

						for (int i = 0; i < MAX_USER; i++) {
							if (clients[i].getState() == CL_STATE_EMPTY) continue;

							clients[i].sendUpdatePacket(itembox_update_pack);
						}
					}

					// 회전을 마치면 다시 큐에 넣습니다.
					setServerEvent(new_event->ev_type, new_event->ev_duration, new_event->ev_target, new_event->ev_target_detail, new_event->target_num,
						new_event->ev_start_time, SetStartTimeToExInfo);

					break;
				}
			}
			break;
		}
		case EV_TYPE_REMOVE:
		{
			if (SERVER_TIME >= new_event->ev_start_time + new_event->ev_duration) {
				// 이벤트 시간이 끝났다면 타겟 타입에 맞는 후처리 작업을 해줍니다.
				switch (new_event->ev_target) {
				case EV_TARGET_BOMB:
					cout << "Bomb[" << target << "] is Removed." << endl;//test

					BombArray[target].returnToInitialState();

					// 제거 패킷을 모든 클라이언트에게 전달합니다.
					GS2C_REMOVE_OBJ_PACKET rm_bomb_packet;
					rm_bomb_packet.size = sizeof(GS2C_REMOVE_OBJ_PACKET);
					rm_bomb_packet.type = GS2C_REMOVE_OBJ;
					rm_bomb_packet.id = target;
					rm_bomb_packet.objtype = OBJ_TYPE_BOMB;

					for (int i = 0; i < MAX_USER; i++) {
						if (clients[i].getState() == CL_STATE_EMPTY) continue;

						clients[i].sendRemoveObjPacket(rm_bomb_packet);
					}

					break;
				}
			}
			else {
				// 아직 이벤트 시간이 끝나지 않았다면 정보를 그대로 유지한채 이벤트 큐에 다시 넣어줍니다.
				setServerEvent(new_event->ev_type, new_event->ev_duration, new_event->ev_target, new_event->ev_target_detail, new_event->target_num,
					new_event->ev_start_time, SetStartTimeToExInfo);
			}
			break;
		}
		case EV_TYPE_HIT:
		{
			if (new_event->ev_target == EV_TARGET_CLIENTS) {
				// 플레이어 객체를 위로 띄웠다가 다시 떨어뜨립니다.
				float theta = (SERVER_TIME - new_event->ev_start_time) * 30.0f;
				if (theta >= 180.0f) theta = 180.0f;
				MyVector3D hit_motion_pos = { 0, 0, 0 };
				hit_motion_pos.x = clients[target].getPos().x;
				hit_motion_pos.y = 14 + 100 * sin(theta * PI / 180.0f);
				hit_motion_pos.z = clients[target].getPos().z;

				clients[target].setPos(hit_motion_pos);
				

				if (theta <= 90.0f) {
					// 플레이어 객체를 x축기준으로 회전시킵니다.
					// pitch 설정
					float temp_pitch = clients[target].getYaw() + theta * 6.0f * PI / 180.0f;
					if (temp_pitch >= 360.0f)
						temp_pitch -= 360.0f;
					clients[target].setPitch(temp_pitch);
				}
				else {
					clients[target].setPitch(0.f);
				}

				// right, up, look 벡터 회전계산
				MyVector3D rotate_result_x = calcRotate(basic_coordinate.x_coordinate
					, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());
				MyVector3D rotate_result_y = calcRotate(basic_coordinate.y_coordinate
					, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());
				MyVector3D rotate_result_z = calcRotate(basic_coordinate.z_coordinate
					, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());

				// right, up, look 벡터 회전결과 적용
				clients[target].setCoordinate(rotate_result_x, rotate_result_y, rotate_result_z);
				
				sendPlayerUpdatePacket_toAllClient(target);

				if (theta < 180.0f) {
					setServerEvent(new_event->ev_type, new_event->ev_duration, new_event->ev_target, new_event->ev_target_detail, new_event->target_num,
						new_event->ev_start_time, SetStartTimeToExInfo);
				}
				else {	// 연출 끝
					clients[target].setLoseControl(false);
				}
			}
			break;
		}
		//case end
		}
		//switch end

		LeaveCriticalSection(&critical_section);	// 임계영역 탈출
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
		Sleep(TIME_UPDATE_CYCLE);									// 잠시 대기 (서버 시간 업데이트 주기)
	}
}
//==================================================

//==================================================
//           [ 충돌 검사 스레드 함수 ]
// 	         서버에서 관리하는 객체들의
//      충돌 검사를 담당하는 스레드함수 입니다.
//==================================================
DWORD WINAPI CollideCheck_ThreadFunc(LPVOID arg)
{
	while (1) {
		for (int i = 0; i < MAX_USER; i++) {
			if (clients[i].getState() != CL_STATE_RUNNING)
				continue;

			// Player - Map 충돌

			// Player - Player 충돌
			collisioncheck_Player2Player(i);

			// Player - Missile 충돌
			collisioncheck_Player2Missile(i);

			// Player - Bomb 충돌

			// Player - ItemBox 충돌
			collisioncheck_Player2ItemBox(i);
		}
	}
}
//==================================================

//==================================================
//             [ 충돌 관련 함수들 ]
// 	  충돌 체크 및 후처리에 관련된 함수들입니다.
//==================================================
void TerrainExitCollision(MyVector3D vec, float veclocity, float scarla)
{
	float Currentvelocity = veclocity / 2;
	vec.x = Currentvelocity;
	vec.z = Currentvelocity;

	if (vec.y != 10.0)
	{
		vec.y -= scarla;
	}

}

void collisioncheck_Player2ItemBox(int client_id)
{
	for (int i = 0; i < ITEMBOXNUM; i++)
	{
		// 플레이어에게서 너무 멀리 떨어져있는 아이템박스는 충돌체크 대상에서 제외합니다.
		if (ItemBoxArray[i].m_pos.x < clients[client_id].getPos().x - 150
			|| ItemBoxArray[i].m_pos.x > clients[client_id].getPos().x + 150) continue;
		if (ItemBoxArray[i].m_pos.y < clients[client_id].getPos().y - 100
			|| ItemBoxArray[i].m_pos.y > clients[client_id].getPos().y + 100) continue;
		if (ItemBoxArray[i].m_pos.z < clients[client_id].getPos().z - 150
			|| ItemBoxArray[i].m_pos.z > clients[client_id].getPos().z + 150) continue;
		// 이미 누군가가 최근에 충돌한 적있는 아이템박스는 충돌체크 대상에서 제외합니다.
		if (!ItemBoxArray[i].m_visible) continue;

		// 충돌체크 & 후처리
		if (ItemBoxArray[i].xoobb.Intersects(clients[client_id].xoobb))
		{
			EnterCriticalSection(&critical_section);
			// 아이템 박스를 안보이게 위치를 조정하고, 충돌체크 대상에서 제외되도록 설정합니다.
			ItemBoxArray[i].m_pos.y = ItemBoxArray[i].m_pos.y - 500;
			ItemBoxArray[i].m_visible = false;

			// 아이템 박스의 변경사항을 모든 클라이언트에게 전달합니다.
			sendItemBoxUpdatePacket_toAllClient(i);

			// 충돌한 아이템박스는 5.0초 후에 초기상태로 돌아옵니다.
			setServerEvent(EV_TYPE_REFRESH, 5.0f, EV_TARGET_ITEMBOX, 0, i, 0, 0);

			// 충돌한 플레이어는 갖고 있는 아이템이 2개 미만일 때에만 새로운 아이템을 얻을 수 있습니다.
			if (clients[client_id].getHowManyItem() < 2) {
				srand(static_cast<unsigned int>(SERVER_TIME) * i);
				//int new_item = rand() % 3;
				int new_item = 1;
				clients[client_id].setItemQueue(new_item);
				cout << "Collide ItemBox[" << i << "], and... ";
				cout << "Get New Item(type: " << new_item << ")." << endl;
			}
			else {
				cout << "You Have Too Many Items..." << endl;
			}
			LeaveCriticalSection(&critical_section);
		}
	}

}

void collisioncheck_Player2Player(int client_id)
{
	for (int i{}; i < MAX_USER; i++)
	{
		// 자기 자신은 충돌체크 대상에서 제외합니다.
		if (i == client_id) continue;

		// 플레이어에게서 너무 멀리 떨어져있는 다른 플레이어의 객체는 충돌체크 대상에서 제외합니다.
		if (clients[i].m_pos.x < clients[client_id].getPos().x - 150
			|| clients[i].m_pos.x > clients[client_id].getPos().x + 150) continue;
		if (clients[i].m_pos.y < clients[client_id].getPos().y - 100
			|| clients[i].m_pos.y > clients[client_id].getPos().y + 100) continue;
		if (clients[i].m_pos.z < clients[client_id].getPos().z - 150
			|| clients[i].m_pos.z > clients[client_id].getPos().z + 150) continue;

		// 충돌 체크 & 후처리
		if (clients[client_id].xoobb.Intersects(clients[i].xoobb))
		{
			EnterCriticalSection(&critical_section);
			if (clients[client_id].getLoseControl() && clients[i].getLoseControl())
				continue;

			cout << "Collide Player : " << client_id << "&" << i << endl;

			// 두 객체의 제어권을 잠시 없앱니다.
			clients[client_id].setLoseControl(true);
			//clients[i].setLoseControl(true);
			setServerEvent(EV_TYPE_REFRESH, 0.7f, EV_TARGET_CLIENTS, EV_DTARGET_CONTROL, client_id, 0, 0);
			//setServerEvent(EV_TYPE_REFRESH, 0.7f, EV_TARGET_CLIENTS, EV_DTARGET_CONTROL, i, 0, 0);

			// 충돌 연출
			MyVector3D reflect_dir{ 0, 0, 0 };
			reflect_dir = { clients[client_id].getCoordinate().z_coordinate.x * (-1.0f),
							clients[client_id].getCoordinate().z_coordinate.y * (-1.0f),
							clients[client_id].getCoordinate().z_coordinate.z * (-1.0f) };

			MyVector3D reflect_Result{ 0,0,0 };
			reflect_Result = calcMove(clients[client_id].getPos(), reflect_dir, REFLECT_SCALAR);

			clients[client_id].setPos(reflect_Result);
			clients[client_id].xoobb = BoundingOrientedBox(XMFLOAT3(clients[client_id].getPos().x, clients[client_id].getPos().y, clients[client_id].getPos().z),
				XMFLOAT3(6.0f, 6.0f, 6.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

			// 플레이어의 변경사항을 모든 클라이언트에게 전달합니다.
			sendPlayerUpdatePacket_toAllClient(client_id);
			LeaveCriticalSection(&critical_section);
		}
	}
}

void collisioncheck_Player2Missile(int client_id)
{
	for (int i = 0; i < MissileNum; i++) {
		// 작동하지 않는 미사일은 충돌체크 대상에서 제외합니다.
		if (!MissileArray[i].getRunning()) continue;

		// 자신을 생성한 플레이어는 공격하지 않습니다.
		if (MissileArray[i].getObjOwner() == client_id) continue;

		// 플레이어에게서 너무 멀리 떨어져있는 미사일은 충돌체크 대상에서 제외합니다.
		if (MissileArray[i].getPos().x < clients[client_id].getPos().x - 150
			|| MissileArray[i].getPos().x > clients[client_id].getPos().x + 150) continue;
		if (MissileArray[i].getPos().y < clients[client_id].getPos().y - 100
			|| MissileArray[i].getPos().y > clients[client_id].getPos().y + 100) continue;
		if (MissileArray[i].getPos().z < clients[client_id].getPos().z - 150
			|| MissileArray[i].getPos().z > clients[client_id].getPos().z + 150) continue;


		// 충돌체크 & 후처리
		if (MissileArray[i].xoobb.Intersects(clients[client_id].xoobb))
		{
			EnterCriticalSection(&critical_section);
			// 피격모션
			clients[client_id].setLoseControl(true);
			setServerEvent(EV_TYPE_HIT, INFINITY, EV_TARGET_CLIENTS, 0, client_id, 0, NoCount);
			LeaveCriticalSection(&critical_section);
		}
	}
}

//void TrapCollision(MyVector3D vec)
//{
//
//	if (AABB.TrapOOBB.Intersects(AABB.PlayerOOBB))
//	{
//		calcRotate(vec, 0.0, 20.0, 0.0);
//	}
//}
//
//void BoosterAnimate(MyVector3D vec, float elapsedtime)
//{
//	//logic -> to kiption
//}
//
//void Animate(MyVector3D vec, float veclocity, float scarla, float elapsedtime)
//{
//	switch (iteminfo.item_value)
//	{
//	case 1:
//		MissileCollision(vec, scarla, elapsedtime);
//		break;
//	case 2:
//		TrapCollision(vec);
//		break;
//	case 3:
//		BoosterAnimate(vec, elapsedtime);
//		break;
//	default:
//		break;
//	}
//}

//==================================================

