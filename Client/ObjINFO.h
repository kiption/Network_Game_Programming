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
	XMFLOAT3 GetRightVector() { return XMFLOAT3(m_right_vec.x, m_right_vec.y, m_right_vec.z); };
	XMFLOAT3 GetUpVector() { return XMFLOAT3(m_up_vec.x, m_up_vec.y, m_up_vec.z); };
	XMFLOAT3 GetLookVector() { return XMFLOAT3(m_look_vec.x, m_look_vec.y, m_look_vec.z); };
	
	void returnToInitialState() {	// 초기상태로 만드는 함수
		m_id = -1;
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_right_vec = { 1.0f, 0.0f, 0.0f };
		m_up_vec = { 0.0f, 1.0f, 0.0f };
		m_look_vec = { 0.0f, 0.0f, 1.0f };
		m_state = OBJ_ST_EMPTY;
	}
};
ObjINFO objinfo;

int myID;
std::array<ObjINFO, MAX_USER> players_info;
std::array<ObjINFO, ITEMBOXNUM> itembox_arr;
std::array<ObjINFO, MissileNum> missile_arr;
std::array<ObjINFO, BombNum> bomb_arr;
int myLapNum = 0;
WCHAR myname[10];