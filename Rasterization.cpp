#include <vector>
#include "Matrix4.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Color.h"
#include "Camera.h"
#include "Line.h"

using namespace std;


double lineEQ(Vec3 v0, Vec3 v1, double x, double y)
{
	return x*(v0.y-v1.y)+y*(v1.x-v0.x)+v0.x*v1.y-v0.y*v1.x;
}

void rasterTriangle(Camera *camera, vector<vector<Color>> &image, Vec3 vertex1, Vec3 vertex2, Vec3 vertex3, Color c1, Color c2, Color c3)
{
    // vector<vector<double>>& depth;
	double xmin, xmax, ymin, ymax;
	xmin = min(min(vertex2.x, vertex3.x), vertex1.x);
	xmax = max(max(vertex2.x, vertex3.x), vertex1.x);
	ymin = min(min(vertex2.y, vertex3.y), vertex1.y);
	ymax = max(max(vertex2.y, vertex3.y), vertex1.y);
	for(int x = xmin ; x<xmax ; x++)
	{
		for(int y=ymin ;y<ymax; y++)
		{
			double a = lineEQ(vertex2, vertex3, x,y)/lineEQ(vertex2, vertex3, vertex1.x,vertex1.y);
			double b = lineEQ(vertex3, vertex1, x,y)/lineEQ(vertex3, vertex1, vertex2.x,vertex2.y);
			double c = lineEQ(vertex1, vertex2, x,y)/lineEQ(vertex1, vertex2, vertex3.x,vertex3.y);
			if(a>=0 && b>=0 && c>=0)
			{
				Color clr(c1*a + c2*b + c3*c);
				// if check buffer value!!!!
				if(x >= 0 && x < camera->horRes && y >= 0 && y < camera->verRes)
                {
                    // TODO: check for validity
                    //double currentBuffer = depth.at(x).at(y);
                    image[x][y] = clr;
                }
			}

		}
	}
}