#include "../../Common/protocol.h"
#include "../../Common/Common.h"

#include <iostream>
#include <array>
#include <vector>
#include <queue>
#include <time.h>
#include "Costants.h"
#include "CalcMove.h"
#include "Global.h"
#include "Collision.h"

using namespace std;

DWORD WINAPI ProcessClient(LPVOID arg);				// 클라이언트 통신 스레드
DWORD WINAPI TimerThreadFunc(LPVOID arg);			// 타이머 스레드
DWORD WINAPI ServerTime_Update(LPVOID arg);			// 서버 시간 갱신 스레드
DWORD WINAPI CollideCheck_ThreadFunc(LPVOID arg);	// 충돌 검사 스레드

Coordinate basic_coordinate;				// 기본(초기) 좌표계

float START_TIME;							// 서버 프로그램이 켜진 시간
float SERVER_TIME;							// 서버 시간
constexpr int TIME_UPDATE_CYCLE = 100;		// 서버 시간 업데이트 주기 (ms단위)

void collisioncheck_Player2Map(int client_id);
void collisioncheck_Player2Water(int client_id);
void collisioncheck_Player2ItemBox(int client_id);
void collisioncheck_Player2Player(int client_id);
void collisioncheck_Player2Missile(int client_id);
void collisioncheck_Player2Bomb(int client_id);
void collisioncheck_Player2CheckPointBox(int client_id);

//==================================================
//           [ 클라이언트 객체 정보 ]
//==================================================
enum { CL_STATE_EMPTY, CL_STATE_RUNNING };
class ClientINFO {
private:
	int			m_id = 0;
	char		m_state;
	SOCKET		m_sock;

	MyVector3D	m_pos;
	Coordinate	m_coordinate;
	float		m_yaw, m_pitch, m_roll;

	float		m_accelerator;
	float		m_limit_acc;

	queue<int>  m_myitem;
	bool		m_item_cooldown;	// 아이템 사용 쿨타임
	bool		m_booster_on;		// 부스터 사용 여부

	bool		m_lose_control;		// 조작 가능 여부 (true일때에는 조작이 불가능합니다.)
	bool		m_hit_motion;		// 피격 모션 연출 중 여부

	bool		m_flooded;			// 침수 여부

	bool		m_check_section[CheckPointNum];
	int			m_lap_num;

public:
	CRITICAL_SECTION	m_cs;

	BoundingOrientedBox xoobb;

public:
	// Initialize
	ClientINFO() {
		m_id = -1;
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

		InitializeCriticalSection(&m_cs);

		xoobb = { XMFLOAT3(m_pos.x,m_pos.y,m_pos.z), XMFLOAT3(6.0f,6.0f,6.0f), XMFLOAT4(0.0f,0.0f,0.0f,1.0f) };

		m_item_cooldown = false;
		m_booster_on = false;
		m_lose_control = false;
		m_hit_motion = false;
		m_flooded = false;

		for (int i{}; i < CheckPointNum; i++) {
			m_check_section[i] = false;
			if (i == 3) {
				m_check_section[i] = true;
			}
		}
		m_lap_num = 0;
	};
	~ClientINFO() {
		DeleteCriticalSection(&m_cs);
	}

public:
	// Accessor Func
	// 1. Get
	// =============Client 기본 정보====================
	SOCKET		getSock() { return m_sock; }
	char		getState() { return m_state; }
	int			getId() { return m_id; }
	MyVector3D	getPos() { return m_pos; }
	Coordinate	getCoordinate() { return m_coordinate; }
	float		getYaw() { return m_yaw; }
	float		getPitch() { return m_pitch; }
	float		getRoll() { return m_roll; }
	
	// =============Client 아이템 정보====================
	int			getItemQueue() {
		if (m_myitem.empty()) return -1;	// 큐가 비어있으면 -1을
		return m_myitem.front();			// 아이템이 있다면 아이템의 고유번호를 반환합니다.
	}
	int			getHowManyItem() { return static_cast<int>(m_myitem.size()); }
	bool		getItemCooldown() { return m_item_cooldown; }
	
	// =============Client 부가 정보====================
	int			getLapNum() { return m_lap_num; }

	float		getAccel() { return m_accelerator; }
	float		getLimitAcc() { return m_limit_acc; }

	bool		getBoosterOn() { return m_booster_on; }
	bool		getLoseControl() { return m_lose_control; }
	bool		getHitMotion() { return m_hit_motion; }
	bool		getFlooded() { return m_flooded; }

	bool		getCheckSection(int num) { return m_check_section[num]; }

	// 2. Set
	// =============Client 기본 정보====================
	void		setSock(SOCKET sock) { m_sock = sock; }
	void		setState(char state) { m_state = state; }
	void		setID(int id) { m_id = id; }	
	void		setCoordinate(Coordinate co) { m_coordinate = co; }
	void		setCoordinate(MyVector3D x, MyVector3D y, MyVector3D z) { m_coordinate.x_coordinate = x; m_coordinate.y_coordinate = y; m_coordinate.z_coordinate = z; }
	void		setPos(MyVector3D pos) {
					m_pos = pos;
					xoobb = BoundingOrientedBox(XMFLOAT3(m_pos.x, m_pos.y, m_pos.z), XMFLOAT3(6.0f, 6.0f, 6.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
				}
	void		setYaw(float f) { m_yaw = f; }
	void		setPitch(float f) { m_pitch = f; }
	void		setRoll(float f) { m_roll = f; }

	// =============Client 아이템 정보====================
	void		setItemQueue(int type) { m_myitem.push(type); } // 아이템 먹을 시에 사용
	void		setItemRelease() { m_myitem.pop(); }			// 사용한 아이템 방출
	void		setItemCooldown(bool b) { m_item_cooldown = b; }
	
	// =============Client 부가 정보====================
	void		setLapNum(int num) { m_lap_num = num; }
	
	void		setAccel(float accel) { m_accelerator = accel; }
	void		setLimitAcc(float acc) { m_limit_acc = acc; }
	
	void		setBoosterOn(bool b) { m_booster_on = b; }
	void		setLoseControl(bool b) { m_lose_control = b; }
	void		setHitMotion(bool b) { m_hit_motion = b; }
	void		setFlooded(bool b) { m_flooded = b; }

	void		setCheckSection(int num, bool b) { m_check_section[num] = b; }

public:
	void		resetInfo();

public:
	// Networking Func
	void		sendLoginInfoPacket(GS2C_LOGIN_INFO_PACKET packet);
	void		sendAddObjPacket(GS2C_ADD_OBJ_PACKET packet);
	void		sendUpdatePacket(GS2C_UPDATE_PACKET packet);
	void		sendRemoveObjPacket(GS2C_REMOVE_OBJ_PACKET packet);
	void		sendLapInfoPacket(GS2C_UPDATE_LAP_PACKET packet);
	void		sendBoosterPacket(GS2C_UPDATE_BOOSTER_PACKET packet);
	void		sendEndTimePacket(GS2C_SERVER_TIME_PACKET packet);

	void		disconnectClient();
};
void ClientINFO::resetInfo() {
	m_id = -1;
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

	m_item_cooldown = false;
	m_booster_on = false;
	m_lose_control = false;
	m_hit_motion = false;
	m_flooded = false;

	for (int i{}; i < CheckPointNum; i++) {
		m_check_section[i] = false;
		if (i == 3) {
			m_check_section[i] = true;
		}
	}
	m_lap_num = 0;
}

array<ClientINFO, MAX_USER> clients;
//==================================================

//==================================================
//             [ 아이템 객체 정보 ]
//==================================================
enum { ITEM_Booster, ITEM_Missile, ITEM_Bomb };
class ItemObject {
	int			m_id;
	int			m_objtype;
	int			m_objOwner;
	float		m_yaw, m_pitch, m_roll;
	MyVector3D	m_pos;
	Coordinate	m_coordinate;
	bool		m_running;

public:
	CRITICAL_SECTION	m_cs;

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
		InitializeCriticalSection(&m_cs);
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
	void		setPos(MyVector3D pos) {
					m_pos = pos;
					xoobb = BoundingOrientedBox(XMFLOAT3(m_pos.x, m_pos.y, m_pos.z), XMFLOAT3(6.0f, 6.0f, 6.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
				}
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
	MyVector3D	m_pos;
	float		m_yaw, m_pitch, m_roll; // 회전 각도
	Coordinate	m_coordinate;			// 회전 행렬
	bool		m_visible;
	CRITICAL_SECTION	m_cs;
	BoundingOrientedBox xoobb;

	ItemBox() {
		m_pos = { 0, 0, 0 };
		m_yaw = m_pitch = m_roll = 0.f;
		MyVector3D tmp_rightvec = { 1.f, 0.f, 0.f };
		MyVector3D tmp_upvec = { 0.f, 1.f, 0.f };
		MyVector3D tmp_lookvec = { 0.f, 0.f, 1.f };
		m_coordinate.x_coordinate = tmp_rightvec;
		m_coordinate.y_coordinate = tmp_upvec;
		m_coordinate.z_coordinate = tmp_lookvec;
		m_visible = true;
		InitializeCriticalSection(&m_cs);
	}
};
array<ItemBox, ITEMBOXNUM> ItemBoxArray;
//==================================================

//==================================================
//           [ 체크 포인트 객체 정보 ]
//==================================================
struct CheckPoint {
	MyVector3D	m_pos = { 0, 0, 0 };
	BoundingOrientedBox xoobb;
};
array<CheckPoint, CheckPointNum> CheckPointBoxArray;
//==================================================

//==================================================
//            [ 서버 이벤트 관련 ]
//==================================================
constexpr int EV_TYPE_QUEUE_ERROR = 99; // type error
enum { EV_TYPE_REFRESH, EV_TYPE_MOVE, EV_TYPE_ROTATE, EV_TYPE_REMOVE, EV_TYPE_HIT, EV_TYPE_HIT_BOMB };											// 이벤트 타입
enum { EV_TARGET_CLIENTS, EV_TARGET_MISSILE, EV_TARGET_BOMB, EV_TARGET_ITEMBOX };												// 이벤트 적용 대상
enum { EV_DTARGET_NONE, EV_DTARGET_ITEMCOOLDOWN, EV_DTARGET_BOOSTER, EV_DTARGET_CONTROL, EV_DTARGET_BOOSTEND };	// Extra Info
constexpr int EV_DTARGET_ALL = 999;	// Extra Info
struct ServerEvent {	// 타이머스레드에서 처리할 이벤트
	// setServerEvent함수 인자에 입력해야하는 정보
	char	ev_type;														// 이벤트 종류
	float	ev_duration;													// 이벤트 지속시간 (이벤트 종료조건을 수동으로 주고싶다면 0을 넣고, flag에 NoCount를 넣어주세요.)
	char	ev_target;														// 이벤트 적용 대상
	char	ev_target_detail;												// 세부 적용 대상
	int		target_num;														// 적용 대상이 배열, 벡터 등에서 몇번째 칸에 있는 지
	float	extra_info;														// 추가적인 정보가 필요한 경우 입력하세요.

	// setServerEvent함수를 통해 자동으로 입력되는 정보
	float	ev_start_time;
};
queue<ServerEvent> ServerEventQueue;		// 서버 이벤트 큐
CRITICAL_SECTION cs_timer_event;

enum { NoFlag, SetStartTimeToExInfo };
void setServerEvent(char type, float sec, char target, char target_detail, int t_num, float ex_info, char flag) {
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
void ClientINFO::sendLapInfoPacket(GS2C_UPDATE_LAP_PACKET packet) {
	int retval = send(m_sock, (char*)&packet, sizeof(GS2C_UPDATE_LAP_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		//err_display("send()");
	}
}
void ClientINFO::sendBoosterPacket(GS2C_UPDATE_BOOSTER_PACKET packet) {
	int retval = send(m_sock, (char*)&packet, sizeof(GS2C_UPDATE_BOOSTER_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		//err_display("send()");
	}
}
void ClientINFO::sendEndTimePacket(GS2C_SERVER_TIME_PACKET packet) {
	int retval = send(m_sock, (char*)&packet, sizeof(GS2C_SERVER_TIME_PACKET), 0);
	if (retval == SOCKET_ERROR) {
		//err_display("send()");
	}
}

void ClientINFO::disconnectClient() {
	cout << "클라이언트[" << m_id << "]의 연결 종료를 감지하였습니다." << endl;
	// c_id번째 클라이언트의 연결해제 사실을 접속해있는 모든 클라이언트에게 전달합니다.
	GS2C_REMOVE_OBJ_PACKET disconnect_pack;
	disconnect_pack.type = GS2C_REMOVE_OBJ;
	disconnect_pack.id = m_id;
	disconnect_pack.objtype = OBJ_TYPE_PLAYER;

	for (int i = 0; i < MAX_USER; i++) {
		if (clients[i].getState() == CL_STATE_EMPTY) continue;

		clients[i].sendRemoveObjPacket(disconnect_pack);
	}

	// c_id번째 클라이언트의 정보를 초기화시킵니다.
	resetInfo();
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
	// 서버 시간 초기화
	START_TIME = 0.0f;
	SERVER_TIME = 0.0f;

	// 서버시간을 업데이트시켜주는 스레드 생성
	HANDLE hTimeUpdateThread = CreateThread(NULL, 0, ServerTime_Update, 0, 0, NULL);

	// 주기적인 작업을 수행하는 타이머 스레드 생성
	HANDLE hTimerThreadThread = CreateThread(NULL, 0, TimerThreadFunc, 0, 0, NULL);
	// 임계영역 초기화
	InitializeCriticalSection(&cs_timer_event);

	// 충돌 검사를 수행하는 스레드 생성
	HANDLE hCollideThreadThread = CreateThread(NULL, 0, CollideCheck_ThreadFunc, 0, 0, NULL);


	// 아이템 박스의 위치값을 설정합니다.
	for (int i{}; i < 3; ++i) {
		ItemBoxArray[i].m_pos.x = 350.f + i * 40.f;
		ItemBoxArray[i].m_pos.y = 20.f;
		ItemBoxArray[i].m_pos.z = MiddleZ;

		ItemBoxArray[3 + i].m_pos.x = MiddleX;
		ItemBoxArray[3 + i].m_pos.y = 20.f;
		ItemBoxArray[3 + i].m_pos.z = 2220.f - i * 40.f;

		ItemBoxArray[6 + i].m_pos.x = 2240.f - i * 40.f;
		ItemBoxArray[6 + i].m_pos.y = 20.f;
		ItemBoxArray[6 + i].m_pos.z = MiddleZ;

		ItemBoxArray[9 + i].m_pos.x = MiddleX;
		ItemBoxArray[9 + i].m_pos.y = 20.f;
		ItemBoxArray[9 + i].m_pos.z = 400.f + i * 40.f;
	}
	// 아이템 박스의 bb를 설정합니다.
	for (int i = 0; i < ITEMBOXNUM; i++) {
		ItemBoxArray[i].xoobb = BoundingOrientedBox(XMFLOAT3(ItemBoxArray[i].m_pos.x, ItemBoxArray[i].m_pos.y, ItemBoxArray[i].m_pos.z),
			XMFLOAT3(6.0f, 6.0f, 6.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		cout << "Create itembox's oobb - " << i << endl;
	}

	// CheckPoint Box의 중심 위치, bb를 설정해줍니다.
	for (int i{}; i < 4; i++) {
		CheckPointBoxArray[i].m_pos = { ItemBoxArray[3 * i + 1].m_pos.x, 0.0f, ItemBoxArray[3 * i + 1].m_pos.z };
		CheckPointBoxArray[i].xoobb = BoundingOrientedBox(XMFLOAT3(CheckPointBoxArray[i].m_pos.x, CheckPointBoxArray[i].m_pos.y, CheckPointBoxArray[i].m_pos.z),
			XMFLOAT3(200.0f, 10.0f, 200.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	// 통신 관련 초기작업들
	int retval;

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
	DeleteCriticalSection(&cs_timer_event);

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

			EnterCriticalSection(&clients[client_id].m_cs);
			clients[client_id].setID(client_id);
			clients[client_id].setSock(client_sock);
			clients[client_id].setState(CL_STATE_RUNNING);
			LeaveCriticalSection(&clients[client_id].m_cs);

			break;
		}
	}

	// 새로 접속한 클라이언트의 정보를 초기화합니다.
	EnterCriticalSection(&clients[client_id].m_cs);
	clients[client_id].setID(client_id);
	MyVector3D Pos = { 400.f + 50.f * client_id, 14.0f, 1150.f };
	clients[client_id].setPos(Pos);
	LeaveCriticalSection(&clients[client_id].m_cs);

	// 새로 접속한 클라이언트에게 자신의 초기 정보를 전달합니다.
	GS2C_LOGIN_INFO_PACKET login_packet;
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
	EnterCriticalSection(&cs_timer_event);
	setServerEvent(EV_TYPE_ROTATE, INFINITY, EV_TARGET_ITEMBOX, EV_DTARGET_ALL, 0, 0, 0);
	LeaveCriticalSection(&cs_timer_event);

	//==================================================
	// Loop - Recv & Process Packets
	//==================================================
	while (1) {

		PACKET_INFO recv_info;
		retval = recv(client_sock, (char*)&recv_info, sizeof(PACKET_INFO), MSG_PEEK);	// MSG_PEEK을 사용하여 수신버퍼를 읽지만 가져오지는 않도록
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAECONNRESET) {
				// 연결 해제 후처리
				clients[client_id].disconnectClient();

				// 소켓 닫기
				closesocket(client_sock);
				cout << "[TCP 서버] 클라이언트 종료: IP 주소= " << addr << ", 포트 번호 = " << ntohs(clientaddr.sin_port) << endl;
				return 0;
			}
			else {
				err_display("recv()");
			}
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
						EnterCriticalSection(&clients[client_id].m_cs);
						clients[client_id].setYaw(temp_yaw);
						LeaveCriticalSection(&clients[client_id].m_cs);

						// right, up, look 벡터 회전계산
						MyVector3D rotate_result_x = calcRotate(basic_coordinate.x_coordinate
							, clients[client_id].getRoll(), clients[client_id].getPitch(), clients[client_id].getYaw());
						MyVector3D rotate_result_y = calcRotate(basic_coordinate.y_coordinate
							, clients[client_id].getRoll(), clients[client_id].getPitch(), clients[client_id].getYaw());
						MyVector3D rotate_result_z = calcRotate(basic_coordinate.z_coordinate
							, clients[client_id].getRoll(), clients[client_id].getPitch(), clients[client_id].getYaw());

						// right, up, look 벡터 회전결과 적용
						EnterCriticalSection(&clients[client_id].m_cs);
						clients[client_id].setCoordinate(rotate_result_x, rotate_result_y, rotate_result_z);
						LeaveCriticalSection(&clients[client_id].m_cs);

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

						float getacc = clients[client_id].getAccel();
						if (getacc <= 0.0f) {
							getacc = 0.015f;

							EnterCriticalSection(&clients[client_id].m_cs);
							clients[client_id].setAccel(getacc);
							LeaveCriticalSection(&clients[client_id].m_cs);
						}
						else if (getacc >= clients[client_id].getLimitAcc()) {
							getacc = clients[client_id].getLimitAcc();

							EnterCriticalSection(&clients[client_id].m_cs);
							clients[client_id].setAccel(getacc);
							LeaveCriticalSection(&clients[client_id].m_cs);
						}
						else {
							if (clients[client_id].getBoosterOn()) {
								getacc += 0.05f;
							}
							else {
								getacc += 0.018f;
							}

							EnterCriticalSection(&clients[client_id].m_cs);
							clients[client_id].setAccel(getacc);
							LeaveCriticalSection(&clients[client_id].m_cs);
						}

						MyVector3D move_dir{ 0, 0, 0 };
						move_dir = { clients[client_id].getCoordinate().z_coordinate.x * plus_minus,
									 clients[client_id].getCoordinate().z_coordinate.y * plus_minus,
									 clients[client_id].getCoordinate().z_coordinate.z * plus_minus };

						MyVector3D Move_Vertical_Result{ 0, 0, 0 };
						Move_Vertical_Result = calcMove(clients[client_id].getPos(), move_dir, clients[client_id].getAccel(), clients[client_id].getFlooded());

						EnterCriticalSection(&clients[client_id].m_cs);
						// 좌표 업데이트
						clients[client_id].setPos(Move_Vertical_Result);
						LeaveCriticalSection(&clients[client_id].m_cs);

						// 클라이언트에게 전달
						sendPlayerUpdatePacket_toAllClient(client_id);

						break;
					}
					case KEY_SPACE:
					{
						// 제어권을 잃은 상태에선 조작이 불가능 합니다.
						if (clients[client_id].getLoseControl())
							break;
						// 아이템 사용 쿨타임 상태라면 아무일도 일어나지 않는다.
						if (clients[client_id].getItemCooldown())
							break;

						int used_item = clients[client_id].getItemQueue();
						if (used_item == -1)
							break;										// 플레이어가 현재 가진 아이템이 없다면 아무일도 일어나지 않는다.

						EnterCriticalSection(&clients[client_id].m_cs);
						clients[client_id].setItemCooldown(true);
						LeaveCriticalSection(&clients[client_id].m_cs);

						EnterCriticalSection(&cs_timer_event);
						setServerEvent(EV_TYPE_REFRESH, ITEM_COOLDOWN_DURATION, EV_TARGET_CLIENTS, EV_DTARGET_ITEMCOOLDOWN, client_id, 0, 0);	// 아이템 사용 쿨타임
						LeaveCriticalSection(&cs_timer_event);

						switch (used_item) {
						case ITEM_Booster:
						{
							if (clients[client_id].getBoosterOn())	// 이미 부스터가 켜져있으면 사용할 수 없습니다.
								break;

							cout << "Use Item[Booster, " << used_item << "]." << endl;

							EnterCriticalSection(&clients[client_id].m_cs);
							clients[client_id].setItemRelease();
							clients[client_id].setBoosterOn(true);
							if (clients[client_id].getFlooded()) {
								clients[client_id].setLimitAcc(FLOODED_BOOSTER_ACCELERATOR);
							}
							else {
								clients[client_id].setLimitAcc(BOOSTER_ACCELERATOR);
							}
							LeaveCriticalSection(&clients[client_id].m_cs);

							// 부스터 연출을 위해 부스터를 사용한 사실을 모든 클라이언트에게 전송합니다.
							GS2C_UPDATE_BOOSTER_PACKET boost_pack;
							boost_pack.type = GS2C_UPDATE_BOOSTER;
							boost_pack.id = client_id;
							boost_pack.boost_on = true;
							for (int i = 0; i < MAX_USER; i++) {
								clients[i].sendBoosterPacket(boost_pack);
							}

							EnterCriticalSection(&cs_timer_event);
							setServerEvent(EV_TYPE_REFRESH, BOOSTER_DURATION, EV_TARGET_CLIENTS, EV_DTARGET_BOOSTER, client_id, 0, 0);	// 부스터 지속시간
							LeaveCriticalSection(&cs_timer_event);

							break;
						}
						case ITEM_Missile:
						{
							// id 할당
							int missile_id = -1;
							for (int i = 0; i < MissileNum; i++) {
								if (!MissileArray[i].getRunning()) {	// 빈칸을 찾았으면 id를 설정하고, for루프를 빠져나옵니다.
									missile_id = i;
									break;
								}
							}
							if (missile_id == -1) {
								cout << "Failed to Shoot Missile - Too Many Missiles in Game!" << endl;
								break;
							}

							// 아이템 사용
							cout << "Use Item[Missile, " << used_item << "]." << endl;
							EnterCriticalSection(&clients[client_id].m_cs);
							clients[client_id].setItemRelease();
							LeaveCriticalSection(&clients[client_id].m_cs);

							// 새롭게 추가되는 미사일의 정보 저장
							EnterCriticalSection(&MissileArray[missile_id].m_cs);
							MissileArray[missile_id].setID(missile_id);
							MissileArray[missile_id].setObjType(used_item);
							MissileArray[missile_id].setObjOwner(client_id);
							MyVector3D missile_first_pos = clients[client_id].getPos();
							MyVector3D missile_lookvec = clients[client_id].getCoordinate().z_coordinate;
							MyVector3D missile_final_pos = calcMove(missile_first_pos, missile_lookvec, 50.f, false);
							MissileArray[missile_id].setPos(missile_final_pos);
							MissileArray[missile_id].setYaw(clients[client_id].getYaw());
							MissileArray[missile_id].setRoll(clients[client_id].getRoll());
							MissileArray[missile_id].setPitch(clients[client_id].getPitch());
							MissileArray[missile_id].setCoordinate(clients[client_id].getCoordinate());
							MissileArray[missile_id].setRunning(true);
							LeaveCriticalSection(&MissileArray[missile_id].m_cs);

							// 새롭게 추가되는 미사일 객체 정보를 모든 클라이언트에게 전달
							GS2C_ADD_OBJ_PACKET add_missile_packet;

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
							EnterCriticalSection(&cs_timer_event);
							setServerEvent(EV_TYPE_MOVE, MISSILE_DURATION, EV_TARGET_MISSILE, 0, missile_id, 0, 0); // 미사일 이동
							LeaveCriticalSection(&cs_timer_event);

							break;
						}
						case ITEM_Bomb:
						{
							// id 할당
							int bomb_id = -1;
							for (int i = 0; i < BombNum; i++) {
								if (!BombArray[i].getRunning()) {	// 빈칸을 찾았으면 id를 설정하고, for루프를 빠져나옵니다.
									bomb_id = i;
									break;
								}
							}
							if (bomb_id == -1) {
								cout << "Failed to Install Bomb - Too Many Bomb in Game!" << endl;
								break;
							}

							// 아이템 사용
							EnterCriticalSection(&clients[client_id].m_cs);
							clients[client_id].setItemRelease();
							cout << "Use Item[Bomb, " << used_item << "]." << endl;
							LeaveCriticalSection(&clients[client_id].m_cs);

							// 새롭게 추가되는 지뢰의 정보 저장
							EnterCriticalSection(&BombArray[bomb_id].m_cs);
							BombArray[bomb_id].setID(bomb_id);
							BombArray[bomb_id].setObjType(used_item);
							BombArray[bomb_id].setObjOwner(client_id);
							MyVector3D bomb_first_pos = clients[client_id].getPos();
							MyVector3D bomb_lookvec = clients[client_id].getCoordinate().z_coordinate;
							MyVector3D bomb_final_pos = calcMove(bomb_first_pos, bomb_lookvec, -20.f, false);
							BombArray[bomb_id].setPos(bomb_final_pos);
							BombArray[bomb_id].setYaw(clients[client_id].getYaw());
							BombArray[bomb_id].setRoll(clients[client_id].getRoll());
							BombArray[bomb_id].setPitch(clients[client_id].getPitch());
							BombArray[bomb_id].setCoordinate(clients[client_id].getCoordinate());
							BombArray[bomb_id].setRunning(true);
							LeaveCriticalSection(&BombArray[bomb_id].m_cs);

							// 새롭게 추가되는 미사일 객체 정보를 모든 클라이언트에게 전달
							GS2C_ADD_OBJ_PACKET add_bomb_packet;

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
							EnterCriticalSection(&cs_timer_event);
							setServerEvent(EV_TYPE_REMOVE, BOMB_DURATION, EV_TARGET_BOMB, 0, bomb_id, 0, 0);
							LeaveCriticalSection(&cs_timer_event);
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
			break;
		case C2GS_KEYUPVALUE:
			C2GS_KEYUPVALUE_PACKET ClientUpKey;
			retval = recv(client_sock, reinterpret_cast<char*>(&ClientUpKey), sizeof(C2GS_KEYUPVALUE_PACKET), 0);
			if (retval == SOCKET_ERROR) {
				//err_display("recv()");
			}

			// 키를 떼면 가속도를 0으로 설정합니다.
			EnterCriticalSection(&clients[client_id].m_cs);
			clients[client_id].setAccel(0.0f);
			LeaveCriticalSection(&clients[client_id].m_cs);

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
		ServerEvent new_event;
		EnterCriticalSection(&cs_timer_event);
		if (ServerEventQueue.empty()) {
			LeaveCriticalSection(&cs_timer_event);
			continue;
		}
		else {
			new_event = ServerEventQueue.front();
			ServerEventQueue.pop();
			LeaveCriticalSection(&cs_timer_event);
		}

		int target = new_event.target_num;
		switch (new_event.ev_type) {
		case EV_TYPE_REFRESH:
		{
			if (SERVER_TIME >= new_event.ev_start_time + new_event.ev_duration) {
				// 이벤트 시간이 끝났다면 타겟 타입에 맞는 후처리 작업을 해줍니다.
				switch (new_event.ev_target) {
				case EV_TARGET_CLIENTS:
					if (new_event.ev_target_detail == EV_DTARGET_CONTROL) {
						EnterCriticalSection(&clients[target].m_cs);
						clients[target].setLoseControl(false);
						LeaveCriticalSection(&clients[target].m_cs);

						cout << "Client[ " << target << "] Gets Control Back" << endl;
					}
					else if (new_event.ev_target_detail == EV_DTARGET_ITEMCOOLDOWN) {
						EnterCriticalSection(&clients[target].m_cs);
						clients[target].setItemCooldown(false);
						LeaveCriticalSection(&clients[target].m_cs);

						cout << "Client[ " << target << "]'s Item Cooldown is End." << endl;
					}
					else if (new_event.ev_target_detail == EV_DTARGET_BOOSTER) {
						EnterCriticalSection(&cs_timer_event);
						setServerEvent(EV_TYPE_REFRESH, 0.1f, EV_TARGET_CLIENTS, EV_DTARGET_BOOSTEND, target, 0, 0);
						LeaveCriticalSection(&cs_timer_event);
						cout << "Client[ " << target << "]'s Booster is End." << endl;
					}
					else if (new_event.ev_target_detail == EV_DTARGET_BOOSTEND) {
						float new_acc = clients[target].getAccel() - 0.6f;
						float limit_acc = 0.f;
						if (clients[target].getFlooded()) {
							limit_acc = FLOODED_ACCELERATOR;
						}
						else {
							limit_acc = LIMIT_ACCELERATOR;
						}

						if (new_acc <= limit_acc) {			// 원래의 가속도 제한수치까지 낮췄을 경우
							EnterCriticalSection(&clients[target].m_cs);
							clients[target].setAccel(limit_acc);
							clients[target].setLimitAcc(limit_acc);
							clients[target].setBoosterOn(false);	// 부스터를 꺼줍니다.
							LeaveCriticalSection(&clients[target].m_cs);

							// 부스터가 종료된 사실을 모든 클라이언트에게 알려줍니다.
							GS2C_UPDATE_BOOSTER_PACKET boost_end_pack;
							boost_end_pack.type = GS2C_UPDATE_BOOSTER;
							boost_end_pack.id = target;
							boost_end_pack.boost_on = false;
							for (int i = 0; i < MAX_USER; i++) {
								if (clients[i].getState() == CL_STATE_EMPTY)
									continue;

								clients[i].sendBoosterPacket(boost_end_pack);
							}
						}
						else {
							EnterCriticalSection(&clients[target].m_cs);
							clients[target].setAccel(new_acc);
							clients[target].setLimitAcc(new_acc);
							LeaveCriticalSection(&clients[target].m_cs);

							// 아직 원래의 가속도로 돌아오지 않았으므로 동일한 이벤트를 다시 불러줍니다.
							EnterCriticalSection(&cs_timer_event);
							setServerEvent(EV_TYPE_REFRESH, 0.1f, EV_TARGET_CLIENTS, EV_DTARGET_BOOSTEND, target, 0, 0);
							LeaveCriticalSection(&cs_timer_event);
						}
					}
					else {
						cout << "[Event Error] Unknown Event's Extra Info." << endl;
					}
					break;
				case EV_TARGET_ITEMBOX:
					cout << "ItemBox[" << target << "] is Refreshed." << endl;

					EnterCriticalSection(&ItemBoxArray[target].m_cs);
					ItemBoxArray[target].m_pos.y = 20.0f;
					ItemBoxArray[target].m_visible = true;
					LeaveCriticalSection(&ItemBoxArray[target].m_cs);

					// 아이템 박스의 변경사항을 모든 클라이언트에게 전달합니다.
					sendItemBoxUpdatePacket_toAllClient(target);
					break;
				}
			}
			else {
				// 아직 이벤트 시간이 끝나지 않았다면 정보를 그대로 유지한채 이벤트 큐에 다시 넣어줍니다.
				EnterCriticalSection(&cs_timer_event);
				setServerEvent(new_event.ev_type, new_event.ev_duration, new_event.ev_target, new_event.ev_target_detail, new_event.target_num
					, new_event.ev_start_time, SetStartTimeToExInfo);
				LeaveCriticalSection(&cs_timer_event);
			}
			break;
		}
		case EV_TYPE_MOVE:
		{
			switch (new_event.ev_target) {
			case EV_TARGET_MISSILE:
				if (!MissileArray[target].getRunning()) break;
				if (SERVER_TIME < new_event.ev_start_time + new_event.ev_duration) {	// 지속시간이 끝날 때까지 지속적으로 움직입니다.
					// 미사일을 앞으로 움직입니다.
					MyVector3D move_dir{ 0, 0, 0 };
					move_dir = { MissileArray[target].getCoordinate().z_coordinate.x,
								 MissileArray[target].getCoordinate().z_coordinate.y,
								 MissileArray[target].getCoordinate().z_coordinate.z };

					MyVector3D Missile_Move_Result = calcMove(MissileArray[target].getPos(), move_dir, MISSILE_MOVE_SCALAR, false);

					EnterCriticalSection(&MissileArray[target].m_cs);
					// 변경된 좌표로 업데이트합니다.
					MissileArray[target].setPos(Missile_Move_Result);
					LeaveCriticalSection(&MissileArray[target].m_cs);

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

					// 이동을 마치면 다시 큐에 넣습니다.
					EnterCriticalSection(&cs_timer_event);
					setServerEvent(new_event.ev_type, new_event.ev_duration, new_event.ev_target, new_event.ev_target_detail, new_event.target_num
						, new_event.ev_start_time, SetStartTimeToExInfo);
					LeaveCriticalSection(&cs_timer_event);
				}
				else {
					// 지속시간이 끝났으면 미사일을 제거합니다.
					EnterCriticalSection(&MissileArray[target].m_cs);
					MissileArray[target].setPos(MyVector3D{ 0.f, 0.f, 0.f });
					MissileArray[target].setRunning(false);
					LeaveCriticalSection(&MissileArray[target].m_cs);

					// 제거 패킷을 보냅니다.
					GS2C_REMOVE_OBJ_PACKET missile_remove_pack;

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
			if (SERVER_TIME < new_event.ev_start_time + new_event.ev_duration) {
				switch (new_event.ev_target) {
				case EV_TARGET_ITEMBOX:
					// 아이템박스를 회전시킵니다.
					// yaw 설정
					for (int i = 0; i < ITEMBOXNUM; i++) {
						float temp_yaw = ItemBoxArray[i].m_yaw + ITEMBOX_ROTATE_SCALAR * PI / 360.0f;
						if (temp_yaw >= 360.0f)
							temp_yaw -= 360.0f;
						EnterCriticalSection(&ItemBoxArray[i].m_cs);
						ItemBoxArray[i].m_yaw = temp_yaw;
						LeaveCriticalSection(&ItemBoxArray[i].m_cs);

						// right, up, look 벡터 회전계산
						MyVector3D rotate_result_x = calcRotate(basic_coordinate.x_coordinate
							, ItemBoxArray[i].m_roll, ItemBoxArray[i].m_pitch, ItemBoxArray[i].m_yaw);
						MyVector3D rotate_result_y = calcRotate(basic_coordinate.y_coordinate
							, ItemBoxArray[i].m_roll, ItemBoxArray[i].m_pitch, ItemBoxArray[i].m_yaw);
						MyVector3D rotate_result_z = calcRotate(basic_coordinate.z_coordinate
							, ItemBoxArray[i].m_roll, ItemBoxArray[i].m_pitch, ItemBoxArray[i].m_yaw);

						// right, up, look 벡터 회전결과 적용
						EnterCriticalSection(&ItemBoxArray[i].m_cs);
						ItemBoxArray[i].m_coordinate.x_coordinate = rotate_result_x;
						ItemBoxArray[i].m_coordinate.y_coordinate = rotate_result_y;
						ItemBoxArray[i].m_coordinate.z_coordinate = rotate_result_z;
						LeaveCriticalSection(&ItemBoxArray[i].m_cs);

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
					EnterCriticalSection(&cs_timer_event);
					setServerEvent(new_event.ev_type, new_event.ev_duration, new_event.ev_target, new_event.ev_target_detail, new_event.target_num,
						new_event.ev_start_time, SetStartTimeToExInfo);
					LeaveCriticalSection(&cs_timer_event);

					break;
				}
			}
			break;
		}
		case EV_TYPE_REMOVE:
		{
			if (SERVER_TIME >= new_event.ev_start_time + new_event.ev_duration) {
				// 이벤트 시간이 끝났다면 타겟 타입에 맞는 후처리 작업을 해줍니다.
				switch (new_event.ev_target) {
				case EV_TARGET_BOMB:
					if (!BombArray[target].getRunning())
						break;

					cout << "Bomb[" << target << "] is Removed." << endl;//test

					EnterCriticalSection(&BombArray[target].m_cs);
					BombArray[target].returnToInitialState();
					LeaveCriticalSection(&BombArray[target].m_cs);

					// 제거 패킷을 모든 클라이언트에게 전달합니다.
					GS2C_REMOVE_OBJ_PACKET rm_bomb_packet;
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
				EnterCriticalSection(&cs_timer_event);
				setServerEvent(new_event.ev_type, new_event.ev_duration, new_event.ev_target, new_event.ev_target_detail, new_event.target_num,
					new_event.ev_start_time, SetStartTimeToExInfo);
				LeaveCriticalSection(&cs_timer_event);
			}
			break;
		}
		case EV_TYPE_HIT:
		{
			if (new_event.ev_target == EV_TARGET_CLIENTS) {
				if (!clients[target].getLoseControl())
					break;
				if (!clients[target].getHitMotion())
					break;

				if (SERVER_TIME >= new_event.ev_start_time + new_event.ev_duration) {
					// 이벤트 시간이 끝났다면
					EnterCriticalSection(&clients[target].m_cs);
					clients[target].setLoseControl(false);
					clients[target].setHitMotion(false);
					LeaveCriticalSection(&clients[target].m_cs);
				}
				else { // 플레이어 객체를 제자리에서 y축기준으로 회전시킵니다.
					// 처음엔 점점 빨라졌다 중간부터 서서히 느려지면서 멈추는 연출
					float rot_scalar = 0.0f;
					float cur_duration = SERVER_TIME - new_event.ev_start_time;
					if (cur_duration < new_event.ev_duration / 2.f) {	// 지속시간의 절반 이전
						rot_scalar = 0.01f + cur_duration / 4.5f;
					}
					else {	// 지속시간의 절반 이후
						rot_scalar = 0.01f + (new_event.ev_start_time + new_event.ev_duration - SERVER_TIME) / 4.5f;
					}

					// yaw 설정
					float theta_rad = rot_scalar * PI / 180.0f;
					float temp_yaw = clients[target].getYaw() + theta_rad;

					EnterCriticalSection(&clients[target].m_cs);
					clients[target].setYaw(temp_yaw);
					LeaveCriticalSection(&clients[target].m_cs);

					// right, up, look 벡터 회전계산
					MyVector3D rotate_result_x = calcRotate(basic_coordinate.x_coordinate
						, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());
					MyVector3D rotate_result_y = calcRotate(basic_coordinate.y_coordinate
						, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());
					MyVector3D rotate_result_z = calcRotate(basic_coordinate.z_coordinate
						, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());

					// right, up, look 벡터 회전결과 적용
					EnterCriticalSection(&clients[target].m_cs);
					clients[target].setCoordinate(rotate_result_x, rotate_result_y, rotate_result_z);
					LeaveCriticalSection(&clients[target].m_cs);

					sendPlayerUpdatePacket_toAllClient(target);

					EnterCriticalSection(&cs_timer_event);
					setServerEvent(new_event.ev_type, new_event.ev_duration, new_event.ev_target, new_event.ev_target_detail, new_event.target_num,
						new_event.ev_start_time, SetStartTimeToExInfo);
					LeaveCriticalSection(&cs_timer_event);
				}
			}
			break;
		}
		case EV_TYPE_HIT_BOMB:
		{
			if (new_event.ev_target == EV_TARGET_CLIENTS) {
				if (!clients[target].getLoseControl())
					break;
				if (!clients[target].getHitMotion())
					break;

				if (SERVER_TIME >= new_event.ev_start_time + new_event.ev_duration) {
					// 이벤트 시간이 끝나면...
					// 우선 플레이어 객체를 다시 지상으로 돌려놓습니다.
					MyVector3D landing_pos = clients[target].getPos();
					landing_pos.y = 14.f;

					// pitch를 원상태로 복구시킵니다.
					EnterCriticalSection(&clients[target].m_cs);
					clients[target].setPitch(0.f);
					LeaveCriticalSection(&clients[target].m_cs);

					// right, up, look 벡터 회전계산
					MyVector3D rotate_result_x = calcRotate(basic_coordinate.x_coordinate
						, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());
					MyVector3D rotate_result_y = calcRotate(basic_coordinate.y_coordinate
						, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());
					MyVector3D rotate_result_z = calcRotate(basic_coordinate.z_coordinate
						, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());

					// 원 상태로 복구합니다.
					EnterCriticalSection(&clients[target].m_cs);
					clients[target].setPos(landing_pos);
					clients[target].setCoordinate(rotate_result_x, rotate_result_y, rotate_result_z);
					clients[target].setLoseControl(false);
					clients[target].setHitMotion(false);
					LeaveCriticalSection(&clients[target].m_cs);

					sendPlayerUpdatePacket_toAllClient(target);
				}
				else {
					// 우선 플레이어 객체를 공중으로 띄웁니다.
					MyVector3D burst_pos = clients[target].getPos();

					//burst_pos.y = 50.f;
					if (SERVER_TIME - new_event.ev_start_time < new_event.ev_duration / 2) {
						burst_pos.y += 0.006f;
					}
					else {
						if (burst_pos.y < 14.f) burst_pos.y = 14.f;
						else					burst_pos.y -= 0.0065f;
					}
					EnterCriticalSection(&clients[target].m_cs);
					clients[target].setPos(burst_pos);
					LeaveCriticalSection(&clients[target].m_cs);

					// pitch 설정
					float theta_rad = HIT_BOMB_ROTATE_SCALAR * PI / 180.0f;
					float temp_pitch = clients[target].getPitch() + theta_rad;

					EnterCriticalSection(&clients[target].m_cs);
					clients[target].setPitch(temp_pitch);
					LeaveCriticalSection(&clients[target].m_cs);

					// right, up, look 벡터 회전계산
					MyVector3D rotate_result_x = calcRotate(basic_coordinate.x_coordinate
						, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());
					MyVector3D rotate_result_y = calcRotate(basic_coordinate.y_coordinate
						, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());
					MyVector3D rotate_result_z = calcRotate(basic_coordinate.z_coordinate
						, clients[target].getRoll(), clients[target].getPitch(), clients[target].getYaw());

					// right, up, look 벡터 회전결과 적용
					EnterCriticalSection(&clients[target].m_cs);
					clients[target].setCoordinate(rotate_result_x, rotate_result_y, rotate_result_z);
					LeaveCriticalSection(&clients[target].m_cs);

					sendPlayerUpdatePacket_toAllClient(target);

					EnterCriticalSection(&cs_timer_event);
					setServerEvent(new_event.ev_type, new_event.ev_duration, new_event.ev_target, new_event.ev_target_detail, new_event.target_num,
						new_event.ev_start_time, SetStartTimeToExInfo);
					LeaveCriticalSection(&cs_timer_event);
				}
			}
			break;
		}
		//case end
		}
		//switch end
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
			collisioncheck_Player2Map(i);
			collisioncheck_Player2Water(i);

			// Player - Player 충돌
			collisioncheck_Player2Player(i);

			// Player - Missile 충돌
			collisioncheck_Player2Missile(i);

			// Player - Bomb 충돌
			collisioncheck_Player2Bomb(i);

			// Player - ItemBox 충돌
			collisioncheck_Player2ItemBox(i);

			// Player - CheckPoint 충돌
			collisioncheck_Player2CheckPointBox(i);
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

// 1. Player - Map 충돌체크
// 1) Map의 끝
void collisioncheck_Player2Map(int client_id) {
	MyVector3D cur_pos = clients[client_id].getPos();
	// 맵의 끝으로부터 너무 멀리떨어져 있다면 충돌체크 및 후처리를 하지 않습니다.
	if (cur_pos.x > MAP_X_MIN + MAP_COLLISIONCHECK_RANGE && cur_pos.x < MAP_X_MAX - MAP_COLLISIONCHECK_RANGE &&
		cur_pos.z > MAP_Z_MIN + MAP_COLLISIONCHECK_RANGE && cur_pos.z < MAP_Z_MAX - MAP_COLLISIONCHECK_RANGE)
		return;

	if (cur_pos.x < MAP_X_MIN) {
		cur_pos.x = MAP_X_MIN;
		// Update
		EnterCriticalSection(&clients[client_id].m_cs);
		clients[client_id].setPos(cur_pos);
		LeaveCriticalSection(&clients[client_id].m_cs);
	}
	else if (cur_pos.x > MAP_X_MAX) {
		cur_pos.x = MAP_X_MAX;
		// Update
		EnterCriticalSection(&clients[client_id].m_cs);
		clients[client_id].setPos(cur_pos);
		LeaveCriticalSection(&clients[client_id].m_cs);
	}

	if (cur_pos.z < MAP_Z_MIN) {
		cur_pos.z = MAP_Z_MIN;
		// Update
		EnterCriticalSection(&clients[client_id].m_cs);
		clients[client_id].setPos(cur_pos);
		LeaveCriticalSection(&clients[client_id].m_cs);
	}
	else if (cur_pos.z > MAP_Z_MAX) {
		cur_pos.z = MAP_Z_MAX;
		// Update
		EnterCriticalSection(&clients[client_id].m_cs);
		clients[client_id].setPos(cur_pos);
		LeaveCriticalSection(&clients[client_id].m_cs);
	}
}
// 2) 물
void collisioncheck_Player2Water(int client_id) {
	MyVector3D cur_pos = clients[client_id].getPos();

	if (clients[client_id].getFlooded()) {	// 이미 침수해있는 차
		// 물 밖으로 빠져나와있는지 검사
		if (cur_pos.x < WATER_X_MIN || cur_pos.x > WATER_X_MAX ||
			cur_pos.z < WATER_Z_MIN || cur_pos.z > WATER_Z_MAX) {
			EnterCriticalSection(&clients[client_id].m_cs);
			clients[client_id].setFlooded(false);
			if (clients[client_id].getBoosterOn()) {
				clients[client_id].setLimitAcc(BOOSTER_ACCELERATOR);
			}
			else {
				clients[client_id].setLimitAcc(LIMIT_ACCELERATOR);
			}
			LeaveCriticalSection(&clients[client_id].m_cs);
		}
	}
	else {	// 육지에 있던 차
		// 물 속으로 들어갔는지 검사
		if (WATER_X_MIN < cur_pos.x && cur_pos.x < WATER_X_MAX &&
			WATER_Z_MIN < cur_pos.z && cur_pos.z < WATER_Z_MAX) {
			EnterCriticalSection(&clients[client_id].m_cs);
			clients[client_id].setFlooded(true);
			if (clients[client_id].getBoosterOn()) {
				clients[client_id].setLimitAcc(FLOODED_BOOSTER_ACCELERATOR);
			}
			else {
				clients[client_id].setLimitAcc(FLOODED_ACCELERATOR);
			}
			LeaveCriticalSection(&clients[client_id].m_cs);
		}
	}
}

// 2. Player - ItemBox 충돌체크
void collisioncheck_Player2ItemBox(int client_id)
{
	for (int i = 0; i < ITEMBOXNUM; i++)
	{
		// 이미 누군가가 최근에 충돌한 적있는 아이템박스는 충돌체크 대상에서 제외합니다.
		if (!ItemBoxArray[i].m_visible) continue;

		// 플레이어에게서 너무 멀리 떨어져있는 아이템박스는 충돌체크 대상에서 제외합니다.
		if (ItemBoxArray[i].m_pos.x < clients[client_id].getPos().x - 150
			|| ItemBoxArray[i].m_pos.x > clients[client_id].getPos().x + 150) continue;
		if (ItemBoxArray[i].m_pos.y < clients[client_id].getPos().y - 100
			|| ItemBoxArray[i].m_pos.y > clients[client_id].getPos().y + 100) continue;
		if (ItemBoxArray[i].m_pos.z < clients[client_id].getPos().z - 150
			|| ItemBoxArray[i].m_pos.z > clients[client_id].getPos().z + 150) continue;

		// 충돌체크 & 후처리
		if (ItemBoxArray[i].xoobb.Intersects(clients[client_id].xoobb))
		{
			// 아이템 박스를 안보이게 위치를 조정하고, 충돌체크 대상에서 제외되도록 설정합니다.
			EnterCriticalSection(&ItemBoxArray[i].m_cs);
			ItemBoxArray[i].m_pos.y = ItemBoxArray[i].m_pos.y - 500;
			ItemBoxArray[i].m_visible = false;
			LeaveCriticalSection(&ItemBoxArray[i].m_cs);

			// 아이템 박스의 변경사항을 모든 클라이언트에게 전달합니다.
			sendItemBoxUpdatePacket_toAllClient(i);

			// 충돌한 아이템박스는 5.0초 후에 초기상태로 돌아옵니다.
			EnterCriticalSection(&cs_timer_event);
			setServerEvent(EV_TYPE_REFRESH, 5.0f, EV_TARGET_ITEMBOX, 0, i, 0, 0);
			LeaveCriticalSection(&cs_timer_event);

			// 충돌한 플레이어는 갖고 있는 아이템이 2개 미만일 때에만 새로운 아이템을 얻을 수 있습니다.
			if (clients[client_id].getHowManyItem() < 2) {
				srand(static_cast<unsigned int>(SERVER_TIME) * i);
				int new_item = rand() % 3;
				//int new_item = 0;

				EnterCriticalSection(&clients[client_id].m_cs);
				clients[client_id].setItemQueue(new_item);
				LeaveCriticalSection(&clients[client_id].m_cs);

				cout << "Collide ItemBox[" << i << "], and... ";
				cout << "Get New Item(type: " << new_item << ")." << endl;
			}
			else {
				cout << "Failed to Get New Item - You Have Too Many Items..." << endl;
			}
		}
	}
}

// 3. Player - Player 충돌체크
void collisioncheck_Player2Player(int client_id)
{
	for (int i{}; i < MAX_USER; i++)
	{
		// 자기 자신은 충돌체크 대상에서 제외합니다.
		if (i == client_id) continue;

		// 이미 충돌해서 제어불능 상태인 객체는 충돌체크 대상에서 제외합니다.
		if (clients[client_id].getLoseControl() && clients[i].getLoseControl())
			continue;

		// 플레이어에게서 너무 멀리 떨어져있는 다른 플레이어의 객체는 충돌체크 대상에서 제외합니다.
		if (clients[i].getPos().x < clients[client_id].getPos().x - 150
			|| clients[i].getPos().x > clients[client_id].getPos().x + 150) continue;
		if (clients[i].getPos().y < clients[client_id].getPos().y - 100
			|| clients[i].getPos().y > clients[client_id].getPos().y + 100) continue;
		if (clients[i].getPos().z < clients[client_id].getPos().z - 150
			|| clients[i].getPos().z > clients[client_id].getPos().z + 150) continue;

		// 충돌 체크 & 후처리
		if (clients[client_id].xoobb.Intersects(clients[i].xoobb))
		{
			cout << "Collide Player : " << client_id << "&" << i << endl;

			// 객체의 제어권을 잠시 없앱니다.
			EnterCriticalSection(&clients[client_id].m_cs);
			clients[client_id].setLoseControl(true);
			LeaveCriticalSection(&clients[client_id].m_cs);

			EnterCriticalSection(&cs_timer_event);
			setServerEvent(EV_TYPE_REFRESH, 0.7f, EV_TARGET_CLIENTS, EV_DTARGET_CONTROL, client_id, 0, 0);
			LeaveCriticalSection(&cs_timer_event);

			// 충돌 연출
			MyVector3D reflect_dir{ 0, 0, 0 };
			reflect_dir = { clients[client_id].getCoordinate().z_coordinate.x * (-1.0f),
							clients[client_id].getCoordinate().z_coordinate.y * (-1.0f),
							clients[client_id].getCoordinate().z_coordinate.z * (-1.0f) };

			MyVector3D reflect_Result{ 0,0,0 };
			reflect_Result = calcMove(clients[client_id].getPos(), reflect_dir, REFLECT_SCALAR, false);

			EnterCriticalSection(&clients[client_id].m_cs);
			clients[client_id].setPos(reflect_Result);
			LeaveCriticalSection(&clients[client_id].m_cs);

			// 플레이어의 변경사항을 모든 클라이언트에게 전달합니다.
			sendPlayerUpdatePacket_toAllClient(client_id);
		}
	}
}

// 4. Player - Missile 충돌체크
void collisioncheck_Player2Missile(int client_id)
{
	for (int i = 0; i < MissileNum; i++) {
		// 작동하지 않는 미사일은 충돌체크 대상에서 제외합니다.
		if (!MissileArray[i].getRunning()) continue;

		// 자신을 생성한 플레이어는 공격하지 않습니다.
		if (MissileArray[i].getObjOwner() == client_id) continue;

		// 피격 모션 중인 플레이어는 공격하지 않습니다.
		if (clients[client_id].getHitMotion()) continue;

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
			EnterCriticalSection(&clients[client_id].m_cs);
			// 피격모션
			clients[client_id].setLoseControl(true);
			clients[client_id].setHitMotion(true);
			// 미사일 삭제
			MissileArray[i].returnToInitialState();
			LeaveCriticalSection(&clients[client_id].m_cs);

			// 미사일 제거 패킷을 모든 클라이언트에게 전달합니다.
			GS2C_REMOVE_OBJ_PACKET rm_missile_packet;
			rm_missile_packet.type = GS2C_REMOVE_OBJ;
			rm_missile_packet.id = i;
			rm_missile_packet.objtype = OBJ_TYPE_MISSILE;
			for (int j = 0; j < MAX_USER; j++) {
				if (clients[j].getState() == CL_STATE_EMPTY) continue;
				clients[j].sendRemoveObjPacket(rm_missile_packet);
			}

			EnterCriticalSection(&cs_timer_event);
			setServerEvent(EV_TYPE_HIT, HIT_MISSILE_DURATION, EV_TARGET_CLIENTS, 0, client_id, 0, 0);
			LeaveCriticalSection(&cs_timer_event);
		}
	}
}

// 5. Player - Bomb 충돌체크
void collisioncheck_Player2Bomb(int client_id)
{
	for (int i = 0; i < BombNum; i++) {
		// 작동하지 않는 지뢰는 충돌체크 대상에서 제외합니다.
		if (!BombArray[i].getRunning()) continue;

		// 피격 모션 중인 플레이어는 공격하지 않습니다.
		if (clients[client_id].getHitMotion()) continue;

		// 플레이어에게서 너무 멀리 떨어져있는 지뢰는 충돌체크 대상에서 제외합니다.
		if (BombArray[i].getPos().x < clients[client_id].getPos().x - 150
			|| BombArray[i].getPos().x > clients[client_id].getPos().x + 150) continue;
		if (BombArray[i].getPos().y < clients[client_id].getPos().y - 100
			|| BombArray[i].getPos().y > clients[client_id].getPos().y + 100) continue;
		if (BombArray[i].getPos().z < clients[client_id].getPos().z - 150
			|| BombArray[i].getPos().z > clients[client_id].getPos().z + 150) continue;


		// 충돌체크 & 후처리
		if (BombArray[i].xoobb.Intersects(clients[client_id].xoobb))
		{
			EnterCriticalSection(&clients[client_id].m_cs);
			// 피격모션
			clients[client_id].setLoseControl(true);
			clients[client_id].setHitMotion(true);
			// 지뢰 삭제
			BombArray[i].returnToInitialState();
			LeaveCriticalSection(&clients[client_id].m_cs);

			// 지뢰 제거 패킷을 모든 클라이언트에게 전달합니다.
			GS2C_REMOVE_OBJ_PACKET rm_bomb_packet;
			rm_bomb_packet.type = GS2C_REMOVE_OBJ;
			rm_bomb_packet.id = i;
			rm_bomb_packet.objtype = OBJ_TYPE_BOMB;
			for (int j = 0; j < MAX_USER; j++) {
				if (clients[j].getState() == CL_STATE_EMPTY) continue;
				clients[j].sendRemoveObjPacket(rm_bomb_packet);
			}

			EnterCriticalSection(&cs_timer_event);
			setServerEvent(EV_TYPE_HIT_BOMB, HIT_BOMB_DURATION, EV_TARGET_CLIENTS, 0, client_id, 0, 0);
			LeaveCriticalSection(&cs_timer_event);
		}
	}
}

// 6. Player - CheckPoint 충돌체크
void collisioncheck_Player2CheckPointBox(int client_id)
{
	for (int i{}; i < CheckPointNum; i++) {
		// 플레이어에게서 너무 멀리 떨어져있는 미사일은 충돌체크 대상에서 제외합니다.
		if (CheckPointBoxArray[i].m_pos.x < clients[client_id].getPos().x - 150
			|| CheckPointBoxArray[i].m_pos.x > clients[client_id].getPos().x + 150) continue;
		if (CheckPointBoxArray[i].m_pos.y < clients[client_id].getPos().y - 100
			|| CheckPointBoxArray[i].m_pos.y > clients[client_id].getPos().y + 100) continue;
		if (CheckPointBoxArray[i].m_pos.z < clients[client_id].getPos().z - 150
			|| CheckPointBoxArray[i].m_pos.z > clients[client_id].getPos().z + 150) continue;

		if (CheckPointBoxArray[i].xoobb.Intersects(clients[client_id].xoobb)) {			
			// 순서를 위해 이전 구역이 true인지 판단. 0->1, 1->2, 2->3, 3->0 순서
			if (i == 0) {	// 0구역
				if (clients[client_id].getCheckSection(3)) {
					EnterCriticalSection(&clients[client_id].m_cs);
					clients[client_id].setCheckSection(i, true);
					clients[client_id].setCheckSection(3, false);
					clients[client_id].setLapNum(clients[client_id].getLapNum() + 1);
					LeaveCriticalSection(&clients[client_id].m_cs);
					
					GS2C_UPDATE_LAP_PACKET add_lap_packet;
					add_lap_packet.type = GS2C_UPDATE_LAP;
					add_lap_packet.lap = clients[client_id].getLapNum();

					clients[client_id].sendLapInfoPacket(add_lap_packet);
					break;
				}
			
				//// 게임시간 UI에 쓰일 서버로그
				if ((clients[client_id].getLapNum() >= 3) && !(clients[client_id].getLoseControl()))
				{
					clients[client_id].setLoseControl(true);
					GS2C_SERVER_TIME_PACKET add_endtime_packet;
					add_endtime_packet.type = GS2C_SERVER_TIME;
					add_endtime_packet.time = (int)SERVER_TIME;
					char sendMSG[50]{ "Finished!" };
					strcpy(add_endtime_packet.msg, sendMSG);
					
					clients[client_id].sendEndTimePacket(add_endtime_packet);
				}
			}
			else { // 1구역 부터 3구역까지만 해당 구간
				if (clients[client_id].getCheckSection(i - 1)) {
					EnterCriticalSection(&clients[client_id].m_cs);
					clients[client_id].setCheckSection(i, true);
					clients[client_id].setCheckSection(i - 1, false);
					LeaveCriticalSection(&clients[client_id].m_cs);
					break;
				}
			}
		}
	}
}
//==================================================

