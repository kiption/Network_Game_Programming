#include "CalcMove.h"

MyVector3D myNormalize(MyVector3D vec)
{
	float dist = sqrtf(powf(vec.x, 2) + powf(vec.y, 2) + powf(vec.z, 2));

	vec.x = vec.x / dist;
	vec.y = vec.y / dist;
	vec.z = vec.z / dist;

	return vec;
}

MyVector3D calcMove(MyVector3D vec1, MyVector3D vec2, float Accelerator)
{
	vec1.x = vec1.x + vec2.x * Accelerator;
	vec1.y = vec1.y + vec2.y * Accelerator;
	vec1.z = vec1.z + vec2.z * Accelerator;
	return vec1;
}

MyVector3D calcRotate(MyVector3D vec, float roll, float pitch, float yaw)
{
	// roll
	float x1, y1;
	x1 = vec.x * cos(roll) - vec.y * sin(roll);
	y1 = vec.x * sin(roll) + vec.y * cos(roll);

	// pitch
	float y2, z1;
	y2 = y1 * cos(pitch) - vec.z * sin(pitch);
	z1 = y1 * sin(pitch) + vec.z * cos(pitch);

	// yaw
	float x2, z2;
	z2 = z1 * cos(yaw) - x1 * sin(yaw);
	x2 = z1 * sin(yaw) + x1 * cos(yaw);

	// Update
	vec = { x2, y2, z2 };
	return myNormalize(vec);
}

MyVector3D calcBooster(MyVector3D vec1, MyVector3D vec2, float scalar)
{
	vec1.x = vec1.x + vec2.x * scalar;
	vec1.y = vec1.y + vec2.y * scalar;
	vec1.z = vec1.z + vec2.z * scalar;

	return vec1;
}
