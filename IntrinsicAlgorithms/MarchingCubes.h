#pragma once

#include <vector>

struct Vec3
{
	float x, y, z;
};

struct Triangle
{
	Vec3 pos;
	Vec3 normal;
	Vec3 color;
};

//void vDrawScene();
void vSetTime(float fTime);
void vMarchingCubes(std::vector<Triangle>& triangles);
void vMarchCube(float fX, float fY, float fZ, float fScale, std::vector<Triangle>& triangles);