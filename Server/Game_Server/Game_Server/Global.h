#pragma once

#define SERVERPORT 9000
#define BUFSIZE    512

#define PI 3.141592654f

#define MOVE_SCALAR 6.0f
#define BOOSTER_SCALAR 10.0f
#define MISSILE_MOVE_SCALAR 10.0f
#define ROTATE_SCALAR 1.5f

#define MiddleX 1295
#define MiddleZ 1310

#define ITEM_COOLDOWN_DURATION 1.0f
#define MISSILE_DURATION 10.0f
#define BOMB_DURATION 60.0f
#define BOOSTER_DURATION 5.0f

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