#ifndef __RASTERIZATION_H__
#define __RASTERIZATION_H__

#include <vector>
#include "Matrix4.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Color.h"
#include "Camera.h"
#include "Line.h"

using namespace std;


double lineEQ(Vec3 v0, Vec3 v1, double x, double y);
void rasterTriangle(Camera *camera, vector<vector<Color>> &image, Vec3 v0, Vec3 v1, Vec3 v2, Color c0, Color c1, Color c2);

#endif
