#include "Collision.h"


MyVector3D TerrainCollision(MyVector3D vec, float veclocity, float scarla)
{
	float Currentvelocity = veclocity / 2;
	vec.x = Currentvelocity;
	vec.z = Currentvelocity;
	
	if (vec.y != 10.0)
	{
		vec.y -= scarla;
	}

	return vec;
}
