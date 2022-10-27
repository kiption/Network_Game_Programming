constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int MAX_USER = 10;
constexpr int MAX_NPCS = 10;

constexpr int WORLD_X_POS = 2000;
constexpr int WORLD_Y_POS = 2000;
constexpr int WORLD_Z_POS = 2000;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_REMOVE_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;

// Packets ( CS: Client->Server, SC: Server->Client )
#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char type;
	char name[NAME_SIZE];
};

struct CS_MOVE_PACKET {
	unsigned char size;
	char type;
	char direction;				// 0: Foward, 1: Back, 2: Left, 3: Right, 4: Down, 5: Up
	float vec_x, vec_y, vec_z;	// 이동방향이 Foward/Back이면 LookVec을, Left/Right이면 RightVec, Down/Up이면 UpVec을 넣어주면 됨.
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char type;
	short id;
	short x, y, z;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char type;
	short id;
	short x, y, z;
	char name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET {
	unsigned char size;
	char type;
	short id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char type;
	short id;
	short x, y, z;
};

#pragma pack (pop)