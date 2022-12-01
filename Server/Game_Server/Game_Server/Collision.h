#pragma once
#include "../../Common/MyVectors.h"
#include "CalcMove.h"
#include "Global.h"
MyVector3D vec3;

struct ITeminfo
{
	int  item_value;
	bool Missile;
	bool Trap;
	bool Booster;
};

XMFLOAT3 GetPosition() { return XMFLOAT3(vec3.x, vec3.y, vec3.z); };
struct BoundingBoxInfo
{
	BoundingOrientedBox ItemboxOOBB = BoundingOrientedBox(GetPosition(), XMFLOAT3(4.0, 4.0, 4.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
	BoundingOrientedBox MissileOOBB = BoundingOrientedBox(GetPosition(), XMFLOAT3(4.0, 4.0, 2.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
	BoundingOrientedBox TrapOOBB = BoundingOrientedBox(GetPosition(), XMFLOAT3(6.0, 6.0, 6.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
	BoundingOrientedBox PlayerOOBB = BoundingOrientedBox(GetPosition(), XMFLOAT3(6.0, 6.0, 6.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));

};

//
//void TerrainExitCollision(MyVector3D vec , float veclocity, float scarla);
//int ITemBoxCollision();
//void MissileCollision(MyVector3D vec, float scarla, float elapsedtime);
//void TrapCollision(MyVector3D vec);
//void BoosterAnimate(MyVector3D vec, float elapsedtime);
//void Animate(MyVector3D vec, float veclocity, float scarla, float elapsedtime);
