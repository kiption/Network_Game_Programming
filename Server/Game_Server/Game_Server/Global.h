#pragma once

#define SERVERPORT 9000
#define BUFSIZE    512

#define MAP_X_MIN 270.f
#define MAP_X_MAX 2380.f
#define MAP_Z_MIN 230.f
#define MAP_Z_MAX 2430.f

#define WATER_X_MIN 670.f
#define WATER_X_MAX 1980.f
#define WATER_Z_MIN 620.f
#define WATER_Z_MAX 2020.f

#define MAP_COLLISIONCHECK_RANGE 100.f

#define PI 3.141592654f

#define ITEM_VARIETY 3;					// 아이템 종류 수

#define REFLECT_SCALAR 20.0f
#define MISSILE_MOVE_SCALAR 0.06f

#define ROTATE_SCALAR 1.5f
#define HIT_BOMB_ROTATE_SCALAR 0.05f
#define ITEMBOX_ROTATE_SCALAR 0.01f

#define MiddleX 1295
#define MiddleZ 1310

#define ITEM_COOLDOWN_DURATION 1.0f
#define MISSILE_DURATION 7.0f
#define BOMB_DURATION 50.0f
#define BOOSTER_DURATION 0.8f
#define HIT_MISSILE_DURATION 3.0f
#define HIT_BOMB_DURATION 2.0f

#define	LIMIT_ACCELERATOR 4.0f
#define BOOSTER_ACCELERATOR 9.0f
#define FLOODED_ACCELERATOR 1.5f
#define FLOODED_BOOSTER_ACCELERATOR 3.5f

#include <random>
#include <math.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#include <DirectXCollision.inl>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;