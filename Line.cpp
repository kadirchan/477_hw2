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
    float v1_to_v2 = sqrt((lineV1.x - lineV2.x)*(lineV1.x - lineV2.x) + (lineV1.y - lineV2.y)*(lineV1.y - lineV2.y));

    Color lineC1 = line.c1;
    Color lineC2 = line.c2;

    // printf("before clip line vertex 1: %f %f %f vertex2: %f %f %f \n", lineV1.x, lineV1.y, lineV1.z, lineV2.x, lineV2.y, lineV2.z);
    // printf("after clip line vertex 1: %f %f %f vertex2: %f %f %f \n", tempV1.x, tempV1.y, tempV1.z, tempV2.x, tempV2.y, tempV2.z);

    // printf("before c1: %f %f %f\n", lineC1.r, lineC1.g, lineC1.b);
    // printf("before c2: %f %f %f\n", lineC2.r, lineC2.g, lineC2.b);
    float alpha = v1_to_v1Prime / v1_to_v2;
    // printf("alpha: %f\n", alpha);
    tempLine.c1.r = (lineC1.r * (1 - alpha) + lineC2.r * alpha) ;
    tempLine.c1.g = (lineC1.g * (1 - alpha) + lineC2.g * alpha) ;
    tempLine.c1.b = (lineC1.b * (1 - alpha) + lineC2.b * alpha) ;

    alpha = v2_to_v2Prime / v1_to_v2;
    // printf("alpha: %f\n", alpha);
    tempLine.c2.r = (lineC2.r * (1 - alpha) + lineC1.r * alpha) ;
    tempLine.c2.g = (lineC2.g * (1 - alpha) + lineC1.g * alpha) ;
    tempLine.c2.b = (lineC2.b * (1 - alpha) + lineC1.b * alpha) ;

    //printf("c1: %f %f %f\n", tempLine.c1.r, tempLine.c1.g, tempLine.c1.b);
    // printf("c2: %f %f %f\n", tempLine.c2.r, tempLine.c2.g, tempLine.c2.b);

    return tempLine;
}
