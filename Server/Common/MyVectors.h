#pragma once
struct MyVector2D {
	float x, y;
};

struct MyVector3D {
	float x, y, z;
};

struct Coordinate {
	MyVector3D x_coordinate;
	MyVector3D y_coordinate;
	MyVector3D z_coordinate;

	Coordinate() {
		x_coordinate = { 1.0f, 0.0f, 0.0f };
		y_coordinate = { 0.0f, 1.0f, 0.0f };
		z_coordinate = { 0.0f, 0.0f, 1.0f };
	}
};
