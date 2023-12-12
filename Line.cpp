#include "Line.h"
#include "cmath"

Line::Line(const Line &prev)
{
    this->c2 = prev.c2;
    this->c1 = prev.c1;
    this->isInside = prev.isInside;
    this->v1 = prev.v1;
    this->v2 = prev.v2;
}


Line::Line(Vec3 v1, Vec3 v2, Color c1, Color c2)
{
    this->c2 = c2;
    this->c1 = c1;
    this->isInside = false;
    this->v1 = v1;
    this->v2 = v2;
}

bool visible(double x, double y, double &tE, double &tL)
{
    double t;
    if(x < 0)
    {
        t = y / x;
        if(t < tE)
            return false;
        else if(t < tL)
            tL = t;
    } else if(x > 0)
    {
        t = y / x;
        if(t > tL)
            return false;
        else if(t > tE)
            tE = t;
    } else if(y > 0)
    {
        return false;
    }
    return true;
}

Line clipLine(Line line)
{
    Line tempLine(line);
    tempLine.isInside = false;
    double tE = 0;
    double tL = 1;
    double dx = tempLine.v2.x - tempLine.v1.x;
    double dy = tempLine.v2.y - tempLine.v1.y;

    if( visible(dx, -1 - tempLine.v1.x, tE, tL) &&
        visible(-dx, tempLine.v1.x - 1, tE, tL) &&
        visible(dy, -1 - tempLine.v1.y, tE, tL) &&
        visible(-dy, tempLine.v1.y - 1, tE, tL) )
    {
        if(tL < 1)
        {
            tempLine.v2.x = tempLine.v1.x + tL * dx;
            tempLine.v2.y = tempLine.v1.y + tL * dy;
        }
        if(tE > 0)
        {
            tempLine.v1.x = tempLine.v1.x + tE * dx;
            tempLine.v1.y = tempLine.v1.y + tE * dy;
        }
        tempLine.isInside = true;
    }

    Vec3 lineV1 = line.v1;
    Vec3 lineV2 = line.v2;
    Vec3 tempV1 = tempLine.v1;
    Vec3 tempV2 = tempLine.v2;

    float v1_to_v1Prime = sqrt((lineV1.x - tempV1.x)*(lineV1.x - tempV1.x) + (lineV1.y - tempV1.y)*(lineV1.y - tempV1.y));
    float v1_to_v2Prime = sqrt((lineV1.x - tempV2.x)*(lineV1.x - tempV2.x) + (lineV1.y - tempV2.y)*(lineV1.y - tempV2.y));
    float v2_to_v1Prime = sqrt((lineV2.x - tempV1.x)*(lineV2.x - tempV1.x) + (lineV2.y - tempV1.y)*(lineV2.y - tempV1.y));
    float v2_to_v2Prime = sqrt((lineV2.x - tempV2.x)*(lineV2.x - tempV2.x) + (lineV2.y - tempV2.y)*(lineV2.y - tempV2.y));

    Color lineC1 = line.c1;
    Color lineC2 = line.c2;

    float divider = (v1_to_v1Prime + v2_to_v1Prime);
    tempLine.c1.r = (lineC1.r * v2_to_v1Prime + lineC2.r * v1_to_v1Prime) / divider;
    tempLine.c1.g = (lineC1.g * v2_to_v1Prime + lineC2.g * v1_to_v1Prime) / divider;
    tempLine.c1.b = (lineC1.b * v2_to_v1Prime + lineC2.b * v1_to_v1Prime) / divider;

    divider = (v1_to_v2Prime + v2_to_v2Prime); 
    tempLine.c2.r = (lineC1.r * v2_to_v2Prime + lineC2.r * v1_to_v2Prime) / divider;
    tempLine.c2.g = (lineC1.g * v2_to_v2Prime + lineC2.g * v1_to_v2Prime) / divider;
    tempLine.c2.b = (lineC1.b * v2_to_v2Prime + lineC2.b * v1_to_v2Prime) / divider;

    return tempLine;
}
