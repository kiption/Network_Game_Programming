#pragma once
#include <array>
#include "../Server/Common/protocol.h"
#include "../Server/Common/MyVectors.h"

enum OBJECT_STATE { OBJ_ST_EMPTY, OBJ_ST_LOGOUT, OBJ_ST_RUNNING };
struct ObjINFO
{
	short m_id;
	float m_x, m_y, m_z;
	MyVector3D m_right_vec;
	MyVector3D m_up_vec;
	MyVector3D m_look_vec;
	int m_state;

	ObjINFO() {
		m_id = -1;
		m_x = 0, m_y = 0, m_z = 0;
		m_right_vec = { 1.0f, 0.0f, 0.0f };
		m_up_vec = { 0.0f, 1.0f, 0.0f };
		m_look_vec = { 0.0f, 0.0f, 1.0f };
		m_state = OBJ_ST_EMPTY;
	}
};

std::array<ObjINFO, MAX_USER> players_info;