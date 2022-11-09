constexpr int SERVER_PORT = 9000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_LEN = 10;

constexpr int MAX_USER = 3;

constexpr int WORLD_X_POS = 2000;
constexpr int WORLD_Y_POS = 2000;
constexpr int WORLD_Z_POS = 2000;

// Packet ID
constexpr char CS_LOGIN = 0;

// Struct
struct MyFloat3
{
	float x, y, z;
};

// Packets ( C: Client / LS: Login Server / GS: Game Server )
#pragma pack (push, 1)
// 1. Client -> Login Server
struct C2LS_SIGNUP_PACKET {
	short size;
	char type;
	char name[NAME_LEN];
};

struct C2LS_LOGIN_PACKET {
	short size;
	char type;
	char name[NAME_LEN];
};

// 2. Login Server -> Client
struct LS2C_SIGNUP_PACKET {
	short size;
	char type;
	char name[NAME_LEN];
};

struct LS2C_GAMESTART_PACKET {
	short size;
	char type;
	char start;
};

// 3. Client -> Game Server
struct C2GS_LOGIN_PACKET {
	short size;
	char type;
	char name[NAME_LEN];
};

struct C2GS_KEYVALUE_PACKET {
	short size;
	char type;
	short key;
};

// 4. Game Server -> Client
struct GS2C_LOGIN_INFO_PACKET {
	short size;
	char type;
	short id;
	MyFloat3 pos;
	MyFloat3 right_vec;
	MyFloat3 up_vec;
	MyFloat3 look_vec;
};

struct GS2C_GET_ITME_PACKET {
	short size;
	char type;
	short itemtype;
};

struct GS2C_MOVE_PACKET {
	short size;
	char type;
	short id;
	MyFloat3 pos;
};

struct GS2C_ROTATE_PACKET {
	short size;
	char type;
	short id;
	MyFloat3 right_vec;
	MyFloat3 up_vec;
	MyFloat3 look_vec;
};

struct GS2C_ADD_OBJ_PACKET {
	short size;
	char type;
	short id;
	short objtype;
	
};

#pragma pack (pop)