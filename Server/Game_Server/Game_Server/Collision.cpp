#include "Collision.h"

random_device ItemVal;
default_random_engine ItemRanVal(ItemVal());
uniform_int_distribution<>PresentItemVal(1, 3);
BoundingBoxInfo AABB;
ITeminfo iteminfo;

void TerrainExitCollision(MyVector3D vec, float veclocity, float scarla)
{
	float Currentvelocity = veclocity / 2;
	vec.x = Currentvelocity;
	vec.z = Currentvelocity;

	if (vec.y != 10.0)
	{
		vec.y -= scarla;
	}

}

int ITemBoxCollision()
{

	if (AABB.ItemboxOOBB.Intersects(AABB.PlayerOOBB))
	{
		iteminfo.item_value = PresentItemVal(ItemVal);
		return iteminfo.item_value;
	}

}

void MissileCollision(MyVector3D vec, float scarla, float elapsedtime)
{

	if (AABB.MissileOOBB.Intersects(AABB.PlayerOOBB))
	{
		vec.y += scarla * elapsedtime;
		calcRotate(vec, 20.0, 0.0, 0.0);
		if (vec.y > 80.0)
		{
			vec.y -= scarla * elapsedtime;
			calcRotate(vec, -20.0, 0.0, 0.0);
		}
	}

}

void TrapCollision(MyVector3D vec)
{

	if (AABB.TrapOOBB.Intersects(AABB.PlayerOOBB))
	{
		calcRotate(vec, 0.0, 20.0, 0.0);
	}
}

void BoosterAnimate(MyVector3D vec, float elapsedtime)
{
	//logic -> to kiption
}

void Animate(MyVector3D vec, float veclocity, float scarla, float elapsedtime)
{
	switch (iteminfo.item_value)
	{
	case 1:
		MissileCollision(vec, scarla, elapsedtime);
		break;
	case 2:
		TrapCollision(vec);
		break;
	case 3:
		BoosterAnimate(vec, elapsedtime);
		break;
	default:
		break;
	}
}
