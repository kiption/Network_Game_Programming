#pragma once
#include <math.h>
#include "../../Common/MyVectors.h"

MyVector3D TerrainCollision(MyVector3D vec , float veclocity, float scarla);

MyVector3D ITemBoxCollision(MyVector3D vec);
MyVector3D MissileCollision(MyVector3D vec);
MyVector3D TrapCollision(MyVector3D vec);
