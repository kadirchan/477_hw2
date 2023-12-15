#include <vector>
#include <cmath>
#include "Line.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Color.h"
#include "Camera.h"
#include "Matrix4.h"

using namespace std;


double lineEQ(Vec3 vertex1, Vec3 vertex2, double x, double y)
{
	return (vertex1.x * vertex2.y - vertex1.y * vertex2.x) + x * (vertex1.y - vertex2.y) + y * (vertex2.x - vertex1.x);
}

void rasterTriangle(vector<vector<double>>& depth, Camera *camera, vector<vector<Color>> &image, Vec3 vertex1, Vec3 vertex2, Vec3 vertex3, Color c1, Color c2, Color c3)
{
	double min_x = min(min(vertex1.x, vertex2.x), vertex3.x);
	double min_y = min(min(vertex1.y, vertex2.y), vertex3.y);
	double max_x = max(max(vertex1.x, vertex2.x), vertex3.x);
	double max_y = max(max(vertex1.y, vertex2.y), vertex3.y);

	double divider_a = lineEQ(vertex2, vertex3, vertex1.x,vertex1.y);
	double divider_b = lineEQ(vertex3, vertex1, vertex2.x,vertex2.y);
	double divider_c = lineEQ(vertex1, vertex2, vertex3.x,vertex3.y);
	
	for(int x = min_x ; x < max_x ; x++)
	{
		for(int y = min_y ; y < max_y ; y++)
		{
			double a = lineEQ(vertex2, vertex3, x,y) / divider_a;
			double b = lineEQ(vertex3, vertex1, x,y) / divider_b;
			double c = lineEQ(vertex1, vertex2, x,y) / divider_c;

			if(a >= 0 && b >= 0 && c >= 0)
			{
				Color pixelColor(c1 * a + c2 * b + c3 * c);
				if(x >= 0 && x < camera->horRes && y >= 0 && y < camera->verRes)
                {
                    double currentBuffer = depth.at(x).at(y);
					double currentZ = vertex1.z * a + vertex2.z * b + vertex3.z * c ;
					if (currentZ < currentBuffer)
					{
                    	image[x][y] = pixelColor;
						depth[x][y] = currentZ;
					}
                }
			}
		}
	}
}


void rasterLine(vector<vector<double>>& depth, Camera *camera, vector<vector<Color>> &image, Line currentLine, bool reversed, vector<Color *> colorsOfVertices)
{
	if (currentLine.isInside==false)
		return;

	vector<Vec3> res;
	float x1,y1,x2,y2,z1,z2;
	Color color1, color2;

	if (reversed)
	{
		x2 = currentLine.v1.x;
		y2 = currentLine.v1.y;
		z2 = currentLine.v1.z;
		x1 = currentLine.v2.x;
		y1 = currentLine.v2.y;
		z1 = currentLine.v2.z;
		color1 = *colorsOfVertices[currentLine.v2.colorId-1];
		color2 = *colorsOfVertices[currentLine.v1.colorId-1]; 
	}
	else 
	{
		x1 = currentLine.v1.x;
		y1 = currentLine.v1.y;
		z1 = currentLine.v1.z;
		x2 = currentLine.v2.x;
		y2 = currentLine.v2.y;
		z2 = currentLine.v2.z;
		color1 = *colorsOfVertices[currentLine.v1.colorId-1];
		color2 = *colorsOfVertices[currentLine.v2.colorId-1];
	}

    float xdiff = (x2 - x1);
	float ydiff = (y2 - y1);

	if(xdiff == 0.0f && ydiff == 0.0f) {
		if(x1 >= 0 && x1 < camera->horRes && y1 >= 0 && y1 < camera->verRes)
		{
			double currentBuffer = depth[x1][y1];
			double currentZ = currentLine.v1.z;
			if ( currentZ < currentBuffer)
			{
				depth[x1][y1] = currentZ; 
        		image[x1][y1] = color1;
			}
		}
	}

	if(fabs(xdiff) > fabs(ydiff)) {
		float xmin, xmax;
		if(x1 < x2) {
			xmin = x1;
			xmax = x2;
		} else {
			xmin = x2;
			xmax = x1;
		}

		float slope = ydiff / xdiff;
		for(float x = xmin; x <= xmax; x += 1.0f) {
			float y = y1 + ((x - x1) * slope);
			Color color = color1 + (Color(color2 - color1) * ((x - x1) / xdiff));
			if(x >= 0 && x < camera->horRes && y >= 0 && y < camera->verRes)
			{
				double currentBuffer = depth[x][y];
				double currentZ = (x-x1)*(z2-z1)/(x2-x1) + z1;
				if ( currentZ < currentBuffer)
				{
					depth[x][y] = currentZ; 
					image[x][y] = color;
				}
			}
		}
	} else {
		float ymin, ymax;
		if(y1 < y2) {
			ymin = y1;
			ymax = y2;
		} else {
			ymin = y2;
			ymax = y1;
		}
		float slope = xdiff / ydiff;
		for(float y = ymin; y <= ymax; y += 1.0f) {
			float x = x1 + ((y - y1) * slope);
			Color color = color1 + ((color2 - color1) * ((y - y1) / ydiff));
			if(x >= 0 && x < camera->horRes && y >= 0 && y < camera->verRes)
			{
				double currentBuffer = depth[x][y];
				double currentZ = (x-x1)*(z2-z1)/(x2-x1) + z1;
				if ( currentZ < currentBuffer)
				{
					depth[x][y] = currentZ; 
					image[x][y] = color;
				}
			}
		}
	}
}