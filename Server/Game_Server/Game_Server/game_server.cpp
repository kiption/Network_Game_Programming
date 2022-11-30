#include "../../Common/protocol.h"
#include "../../Common/Common.h"

#include <iostream>
#include <array>
#include <vector>
#include <queue>
#include "CalcMove.h"
#include "Global.h"

using namespace std;

DWORD WINAPI ProcessClient(LPVOID arg);		// Ŭ���̾�Ʈ ��� ������
DWORD WINAPI TimerThreadFunc(LPVOID arg);	// Ÿ�̸� ������
DWORD WINAPI ServerTime_Update(LPVOID arg);	// ���� �ð� ���� ������

Coordinate basic_coordinate;	// �⺻(�ʱ�) ��ǥ��

float START_TIME;						// ���� ���α׷��� ���� �ð�
float SERVER_TIME;						// ���� �ð�
constexpr int TIME_UPDATE_CYCLE = 100;	// ���� �ð� ������Ʈ �ֱ� (ms����)

//==================================================
//           [ Ŭ���̾�Ʈ ��ü ���� ]
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
		if (m_myitem.empty()) return -1;	// ť�� ��������� -1��
		return m_myitem.front();			// �������� �ִٸ� �������� ������ȣ�� ��ȯ�մϴ�.
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
	void		setItemQueue(int type) { m_myitem.push(type); } // ������ ���� �ÿ� ���
	void		setItemRelease() { m_myitem.pop(); } // ����� ������ ����

public:
	void		sendLoginInfoPacket(GS2C_LOGIN_INFO_PACKET packet);
	void		sendAddObjPacket(GS2C_ADD_OBJ_PACKET packet);
	void		sendUpdatePacket(GS2C_UPDATE_PACKET packet);
};
array<ClientINFO, MAX_USER> clients;
//==================================================

//==================================================
//             [ ������ ��ü ���� ]
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
vector<ItemObject> ObjectManager; // �̻��� ���� ������ ������ ��Ƶδ� ��
//==================================================

//==================================================
//           [ ������ �ڽ� ��ü ���� ]
//==================================================
struct ItemBox {
	float		m_yaw, m_pitch, m_roll; // ȸ�� ����
	MyVector3D	m_pos;
	Coordinate	m_coordinate; // ȸ�� ���
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
void sendUpdatePacket_toAllClient(int c_id) {	// ��� Ŭ���̾�Ʈ���� ������ �Լ�
	GS2C_UPDATE_PACKET update_packet;
	// client_id��° Ŭ���̾�Ʈ ��ü�� ��������� ���� ��Ŷ�� ����ϴ�.
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

	// client_id��° Ŭ���̾�Ʈ ��ü�� ����� ������ ��� Ŭ���̾�Ʈ�鿡�� �����մϴ�.
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
	// ���� �ð� �ʱ�ȭ
	START_TIME = 0.0f;
	SERVER_TIME = 0.0f;
	// �����ð��� ������Ʈ�����ִ� ������ ����
	HANDLE hTimeUpdateThread = CreateThread(NULL, 0, ServerTime_Update, 0, 0, NULL);

	// �ֱ����� �۾��� �����ϴ� ������ ����
	HANDLE hTimerThreadThread = CreateThread(NULL, 0, TimerThreadFunc, 0, 0, NULL);

	// ������ �ڽ��� ��ġ���� �����մϴ�.
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

	// ��� ���� �ʱ��۾���
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
			//err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		cout << "\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=" << addr << ", ��Ʈ ��ȣ = " << ntohs(clientaddr.sin_port) << endl;

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
//==================================================

//==================================================
//             [ ��� ������ �Լ� ]
//==================================================
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	// Ŭ���̾�ƮID �Ҵ�
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

	// ���� ������ Ŭ���̾�Ʈ�� ������ �ʱ�ȭ�մϴ�.
	clients[client_id].setID(client_id);
	MyVector3D Pos = { 400 + 50 * client_id, 14.0, 400 + 50 * client_id };
	clients[client_id].setPos(Pos);

	// ���� ������ Ŭ���̾�Ʈ���� �ڽ��� �ʱ� ������ �����մϴ�.
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


	// ���� ������ Ŭ���̾�Ʈ���� ���� ������ �ִ� ��� Ŭ���̾�Ʈ���� ��ü ������ �����մϴ�.
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

	// ���� ������ �ִ� ��� Ŭ���̾�Ʈ�鿡�� ���� ������ Ŭ���̾�Ʈ�� ��ü ������ �����մϴ�.
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

	// ���� ������ �ִ� ��� Ŭ���̾�Ʈ�鿡�� ������ �ڽ� ������ �����մϴ�.
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
		// �̵� �Լ�
		C2GS_KEYVALUE_PACKET ClientPushKey;
		retval = recv(client_sock, reinterpret_cast<char*>(&ClientPushKey), sizeof(C2GS_KEYVALUE_PACKET), 0);
		if (retval == SOCKET_ERROR) {
			//err_display("recv()");
			closesocket(client_sock);
		}

		enum { KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP, KEY_SPACE };
		for (int i = KEY_LEFT; i <= KEY_SPACE; ++i) {
			if ((ClientPushKey.key >> i) & 1) {

				int plus_minus = 1;	// ��� ����

				switch (i) {
				case KEY_LEFT:
					plus_minus = -1;
					[[fallthrough]];
				case KEY_RIGHT:
				{
					// yaw ����
					clients[client_id].setYaw(clients[client_id].getYaw() + ROTATE_SCALAR * plus_minus * PI / 360.0f);

					// right, up, look ���� ȸ�����
					MyVector3D rotate_result_x = calcRotate(basic_coordinate.x_coordinate
						, clients[client_id].getRoll(), clients[client_id].getPitch(), clients[client_id].getYaw());
					MyVector3D rotate_result_y = calcRotate(basic_coordinate.y_coordinate
						, clients[client_id].getRoll(), clients[client_id].getPitch(), clients[client_id].getYaw());
					MyVector3D rotate_result_z = calcRotate(basic_coordinate.z_coordinate
						, clients[client_id].getRoll(), clients[client_id].getPitch(), clients[client_id].getYaw());

					// right, up, look ���� ȸ����� ����
					clients[client_id].setCoordinate(rotate_result_x, rotate_result_y, rotate_result_z);

					// client_id��° Ŭ���̾�Ʈ ��ü�� ��������� ���� ��Ŷ�� ����ϴ�.
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

					// client_id��° Ŭ���̾�Ʈ ��ü�� ��������� ���� ��Ŷ�� ����ϴ�.
					sendUpdatePacket_toAllClient(client_id);

					break;
				}
				case KEY_SPACE:
				{
					if (clients[client_id].getItemQueue() == -1) // �÷��̾ ���� ���� �������� ���ٸ� �ƹ��ϵ� �Ͼ�� �ʴ´�.
						break;

					switch (clients[client_id].getItemQueue()) {
					case ITEM_Missile:
					case ITEM_Bomb:
					{
						// �������� �����ϴ� �����۰�ü(�̻���, ����) �����̳ʿ� ���Ӱ� �߰��Ǵ� ��ü ���� ����
						ItemObject temp;

						temp.setObjType(clients[client_id].getItemQueue());
						temp.setObjOwner(client_id);
						temp.setPos(clients[client_id].getPos());
						temp.setYaw(clients[client_id].getYaw());
						temp.setRoll(clients[client_id].getRoll());
						temp.setPitch(clients[client_id].getPitch());
						temp.setCoordinate(clients[client_id].getCoordinate());

						ObjectManager.push_back(temp);

						// ���Ӱ� �߰��Ǵ� ��ü ������ ��� Ŭ���̾�Ʈ���� ����
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
						// �ν��ʹ� ���߿�...
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

	// ���� �ݱ�
	closesocket(client_sock);
	cout << "[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�= " << addr << ", ��Ʈ ��ȣ = " << ntohs(clientaddr.sin_port) << endl;
	return 0;
}
//==================================================

//==================================================
//            [ Ÿ�̸� ������ �Լ� ]
//      ���� �ֱ⸶�� �����ؾ��ϴ� �۾�����
//             �̰����� ó���մϴ�.
//==================================================
DWORD WINAPI TimerThreadFunc(LPVOID arg)
{
	while (1) {
		Sleep(100);
	}
}
//==================================================

//==================================================
//   [ ���� �ð��� �帣�� ���ִ� ������ �Լ� ]
// 		���� �ð��� �帣�� �ϴ� �ڵ��Դϴ�.
//		Ư���� �� ������ �ǵ��� �����ּ���.
//==================================================
DWORD WINAPI ServerTime_Update(LPVOID arg)
{
	START_TIME = (float)clock() / CLOCKS_PER_SEC;
	while (1) {
		SERVER_TIME = (float)clock() / CLOCKS_PER_SEC - START_TIME;	// ���� �ð� ������Ʈ
		Sleep(TIME_UPDATE_CYCLE);									// 0.2�� ��� (���� �ð� ������Ʈ �ֱ�)
	}
}
//==================================================

