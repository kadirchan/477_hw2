#include <iostream>
#include <string>
#include <vector>
#include "Scene.h"
#include "Matrix4.h"
#include "Helpers.h"

class Line
{
public:
    bool isInside;
    Color c1, c2;
    Vec3 v1, v2;

    Line(const Line &prev);
    Line(Vec3 v1, Vec3 v2, Color c1, Color c2);
};

Line clipLine(Line line, double minX, double minY, double maxX, double maxY);