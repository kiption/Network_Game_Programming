#pragma once
#include <array>
#include "../Server/Common/protocol.h"
#include "stdafx.h"

enum OBJECT_STATE { OBJ_ST_EMPTY, OBJ_ST_LOGOUT, OBJ_ST_RUNNING };
struct BoundingBoxInfo
{
	BoundingOrientedBox ItemboxOOBB = BoundingOrientedBox();
	BoundingOrientedBox MissileOOBB = BoundingOrientedBox();
	BoundingOrientedBox TrapOOBB = BoundingOrientedBox();
	BoundingOrientedBox PlayerOOBB = BoundingOrientedBox();

};

struct ObjINFO
{
	short m_id;
	XMFLOAT3 m_pos;
	XMFLOAT3 m_right_vec;
	XMFLOAT3 m_up_vec;
	XMFLOAT3 m_look_vec;
	BoundingBoxInfo m_xoobb;
	int m_state;

	ObjINFO() {
		m_id = -1;
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_right_vec = { 1.0f, 0.0f, 0.0f };
		m_up_vec = { 0.0f, 1.0f, 0.0f };
		m_look_vec = { 0.0f, 0.0f, 1.0f };
		m_state = OBJ_ST_EMPTY;
	}

	XMFLOAT3 GetPosition() { return XMFLOAT3(m_pos.x, m_pos.y, m_pos.z); };
};
ObjINFO objinfo;


std::array<ObjINFO, MAX_USER> players_info;