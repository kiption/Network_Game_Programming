constexpr int LOGIN_SERVER_PORT = 9000;
constexpr int GAME_SERVER_PORT = 10000;

constexpr int BUF_SIZE = 200;
constexpr int NAME_LEN = 10;

constexpr int MAX_USER = 3;
constexpr int ITEMBOXNUM = 12;
constexpr int MissileNum = 100;
constexpr int BombNum = 100;
constexpr int CheckPointNum = 4;

constexpr int WORLD_X_POS = 2000;
constexpr int WORLD_Y_POS = 2000;
constexpr int WORLD_Z_POS = 2000;

//===================================
// Packet ID
//===================================
// C -> LS
constexpr char C2LS_LOGIN = 0;
constexpr char C2LS_REGISTER = 1;
// LS -> C
constexpr char LS2C_REGISTER = 2;
constexpr char LS2C_GAMESTART = 3;
// C -> GS
constexpr char C2GS_LOGIN = 4;
constexpr char C2GS_KEYVALUE = 5;
constexpr char C2GS_KEYUPVALUE = 6;

// GS -> C
constexpr char GS2C_LOGIN_INFO = 7;
constexpr char GS2C_ADD_OBJ = 8;
constexpr char GS2C_REMOVE_OBJ = 9;
constexpr char GS2C_UPDATE = 10;
constexpr char GS2C_GET_ITME = 11;
constexpr char GS2C_UPDATE_LAP = 12;
constexpr char GS2C_UPDATE_BOOSTER = 13;
constexpr char GS2C_SERVER_TIME = 14;


//===================================
// Packets
//   C: Client
//   LS: Login Server
//   GS: Game Server )
//===================================
#pragma pack (push, 1)
struct PACKET_INFO {
	char type;
};
// 1. Client -> Login Server
struct C2LS_REGISTER_PACKET {
	char type;
	char name[NAME_LEN];
};

struct C2LS_LOGIN_PACKET {
	char type;
	char name[NAME_LEN];
};

// 2. Login Server -> Client
struct LS2C_REGISTER_PACKET {
	char type;
	bool result;
};

enum { START_DENY_UNKNOWNNAME, START_DENY_FULL, START_APPROVAL, START_DENY_ALREADYUSED };
struct LS2C_GAMESTART_PACKET {
	char type;
	char start;
};

// 3. Client -> Game Server
struct C2GS_LOGIN_PACKET {
	char type;
	char name[NAME_LEN];
};

struct C2GS_KEYVALUE_PACKET {
	char type;
	short key;
};

struct C2GS_KEYUPVALUE_PACKET {
	char type;
	short key;
};

// 4. Game Server -> Client
struct GS2C_LOGIN_INFO_PACKET {
	char type;
	short id;
	float pos_x, pos_y, pos_z;
	float right_vec_x, right_vec_y, right_vec_z;
	float up_vec_x, up_vec_y, up_vec_z;
	float look_vec_x, look_vec_y, look_vec_z;
};

enum { OBJ_TYPE_PLAYER, OBJ_TYPE_MISSILE, OBJ_TYPE_BOMB, OBJ_TYPE_ITEMBOX, OBJ_TYPE_LAP };
struct GS2C_ADD_OBJ_PACKET {
	char type;
	short id;
	short objtype;
	float pos_x, pos_y, pos_z;
	float right_vec_x, right_vec_y, right_vec_z;
	float up_vec_x, up_vec_y, up_vec_z;
	float look_vec_x, look_vec_y, look_vec_z;
};

struct GS2C_REMOVE_OBJ_PACKET {
	char type;
	short id;
	short objtype;
};

struct GS2C_UPDATE_PACKET {
	char type;
	short id;
	short objtype;
	float pos_x, pos_y, pos_z;
	float right_vec_x, right_vec_y, right_vec_z;
	float up_vec_x, up_vec_y, up_vec_z;
	float look_vec_x, look_vec_y, look_vec_z;
};

struct GS2C_GET_ITME_PACKET {
	char type;
	short itemtype;
};

struct GS2C_UPDATE_LAP_PACKET {
	char type;
	int lap;
	short objtype;
};

struct GS2C_UPDATE_BOOSTER_PACKET {
	char type;
	short id;
	bool boost_on;
};

struct GS2C_SERVER_TIME_PACKET {
	char type;
	int time;
	char msg[50];
};

#pragma pack (pop)