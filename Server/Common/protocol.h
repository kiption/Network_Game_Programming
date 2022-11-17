constexpr int SERVER_PORT = 9000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_LEN = 10;

constexpr int MAX_USER = 3;

constexpr int WORLD_X_POS = 2000;
constexpr int WORLD_Y_POS = 2000;
constexpr int WORLD_Z_POS = 2000;

// Packet ID
constexpr char C2LS_LOGIN = 0;
constexpr char C2LS_REGISTER = 1;
constexpr char LS2C_REGISTER = 2;
constexpr char LS2C_GAMESTART = 3;

// Process Key
constexpr char C2GS_INPUT_KEYBOARD = 6;

// Packets ( C: Client / LS: Login Server / GS: Game Server )
#pragma pack (push, 1)
struct PACKET_INFO {
	short size;
	char type;
};
// 1. Client -> Login Server
struct C2LS_REGISTER_PACKET {
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
struct LS2C_REGISTER_PACKET {
	short size;
	char type;
	bool result;
};

enum{START_DENY, START_APPROVAL};
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
	float pos_x, pos_y, pos_z;
	float right_vec_x, right_vec_y, right_vec_z;
	float up_vec_x, up_vec_y, up_vec_z;
	float look_vec_x, look_vec_y, look_vec_z;
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
	float pos_x, pos_y, pos_z;
};

struct GS2C_ROTATE_PACKET {
	short size;
	char type;
	short id;
	float right_vec_x, right_vec_y, right_vec_z;
	float up_vec_x, up_vec_y, up_vec_z;
	float look_vec_x, look_vec_y, look_vec_z;
};

struct GS2C_ADD_OBJ_PACKET {
	short size;
	char type;
	short id;
	short objtype;
	
};

#pragma pack (pop)