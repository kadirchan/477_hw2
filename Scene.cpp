#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>
#include <map>

#include "tinyxml2.h"
#include "Triangle.h"
#include "Helpers.h"
#include "Scene.h"
#include "Line.h"
#include "Rasterization.h"

using namespace tinyxml2;
using namespace std;

/*
	Parses XML file
*/
Scene::Scene(const char *xmlPath)
{
	const char *str;
	XMLDocument xmlDoc;
	XMLElement *xmlElement;

	xmlDoc.LoadFile(xmlPath);

	XMLNode *rootNode = xmlDoc.FirstChild();

	// read background color
	xmlElement = rootNode->FirstChildElement("BackgroundColor");
	str = xmlElement->GetText();
	sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

	// read culling
	xmlElement = rootNode->FirstChildElement("Culling");
	if (xmlElement != NULL)
	{
		str = xmlElement->GetText();

		if (strcmp(str, "enabled") == 0)
		{
			this->cullingEnabled = true;
		}
		else
		{
			this->cullingEnabled = false;
		}
	}

	// read cameras
	xmlElement = rootNode->FirstChildElement("Cameras");
	XMLElement *camElement = xmlElement->FirstChildElement("Camera");
	XMLElement *camFieldElement;
	while (camElement != NULL)
	{
		Camera *camera = new Camera();

		camElement->QueryIntAttribute("id", &camera->cameraId);

		// read projection type
		str = camElement->Attribute("type");

		if (strcmp(str, "orthographic") == 0)
		{
			camera->projectionType = ORTOGRAPHIC_PROJECTION;
		}
		else
		{
			camera->projectionType = PERSPECTIVE_PROJECTION;
		}

		camFieldElement = camElement->FirstChildElement("Position");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->position.x, &camera->position.y, &camera->position.z);

		camFieldElement = camElement->FirstChildElement("Gaze");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->gaze.x, &camera->gaze.y, &camera->gaze.z);

		camFieldElement = camElement->FirstChildElement("Up");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->v.x, &camera->v.y, &camera->v.z);

		camera->gaze = normalizeVec3(camera->gaze);
		camera->u = crossProductVec3(camera->gaze, camera->v);
		camera->u = normalizeVec3(camera->u);

		camera->w = inverseVec3(camera->gaze);
		camera->v = crossProductVec3(camera->u, camera->gaze);
		camera->v = normalizeVec3(camera->v);

		camFieldElement = camElement->FirstChildElement("ImagePlane");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
			   &camera->left, &camera->right, &camera->bottom, &camera->top,
			   &camera->near, &camera->far, &camera->horRes, &camera->verRes);

		camFieldElement = camElement->FirstChildElement("OutputName");
		str = camFieldElement->GetText();
		camera->outputFilename = string(str);

		this->cameras.push_back(camera);

		camElement = camElement->NextSiblingElement("Camera");
	}

	// read vertices
	xmlElement = rootNode->FirstChildElement("Vertices");
	XMLElement *vertexElement = xmlElement->FirstChildElement("Vertex");
	int vertexId = 1;

	while (vertexElement != NULL)
	{
		Vec3 *vertex = new Vec3();
		Color *color = new Color();

		vertex->colorId = vertexId;

		str = vertexElement->Attribute("position");
		sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

		str = vertexElement->Attribute("color");
		sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

		this->vertices.push_back(vertex);
		this->colorsOfVertices.push_back(color);

		vertexElement = vertexElement->NextSiblingElement("Vertex");

		vertexId++;
	}

	// read translations
	xmlElement = rootNode->FirstChildElement("Translations");
	XMLElement *translationElement = xmlElement->FirstChildElement("Translation");
	while (translationElement != NULL)
	{
		Translation *translation = new Translation();

		translationElement->QueryIntAttribute("id", &translation->translationId);

		str = translationElement->Attribute("value");
		sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

		this->translations.push_back(translation);

		translationElement = translationElement->NextSiblingElement("Translation");
	}

	// read scalings
	xmlElement = rootNode->FirstChildElement("Scalings");
	XMLElement *scalingElement = xmlElement->FirstChildElement("Scaling");
	while (scalingElement != NULL)
	{
		Scaling *scaling = new Scaling();

		scalingElement->QueryIntAttribute("id", &scaling->scalingId);
		str = scalingElement->Attribute("value");
		sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

		this->scalings.push_back(scaling);

		scalingElement = scalingElement->NextSiblingElement("Scaling");
	}

	// read rotations
	xmlElement = rootNode->FirstChildElement("Rotations");
	XMLElement *rotationElement = xmlElement->FirstChildElement("Rotation");
	while (rotationElement != NULL)
	{
		Rotation *rotation = new Rotation();

		rotationElement->QueryIntAttribute("id", &rotation->rotationId);
		str = rotationElement->Attribute("value");
		sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

		this->rotations.push_back(rotation);

		rotationElement = rotationElement->NextSiblingElement("Rotation");
	}

	// read meshes
	xmlElement = rootNode->FirstChildElement("Meshes");

	XMLElement *meshElement = xmlElement->FirstChildElement("Mesh");
	while (meshElement != NULL)
	{
		Mesh *mesh = new Mesh();

		meshElement->QueryIntAttribute("id", &mesh->meshId);

		// read projection type
		str = meshElement->Attribute("type");

		if (strcmp(str, "wireframe") == 0)
		{
			mesh->type = WIREFRAME_MESH;
		}
		else
		{
			mesh->type = SOLID_MESH;
		}

		// read mesh transformations
		XMLElement *meshTransformationsElement = meshElement->FirstChildElement("Transformations");
		XMLElement *meshTransformationElement = meshTransformationsElement->FirstChildElement("Transformation");

		while (meshTransformationElement != NULL)
		{
			char transformationType;
			int transformationId;

			str = meshTransformationElement->GetText();
			sscanf(str, "%c %d", &transformationType, &transformationId);

			mesh->transformationTypes.push_back(transformationType);
			mesh->transformationIds.push_back(transformationId);

			meshTransformationElement = meshTransformationElement->NextSiblingElement("Transformation");
		}

		mesh->numberOfTransformations = mesh->transformationIds.size();

		// read mesh faces
		char *row;
		char *cloneStr;
		int v1, v2, v3;
		XMLElement *meshFacesElement = meshElement->FirstChildElement("Faces");
		str = meshFacesElement->GetText();
		cloneStr = strdup(str);

		row = strtok(cloneStr, "\n");
		while (row != NULL)
		{
			int result = sscanf(row, "%d %d %d", &v1, &v2, &v3);

			if (result != EOF)
			{
				mesh->triangles.push_back(Triangle(v1, v2, v3));
			}
			row = strtok(NULL, "\n");
		}
		mesh->numberOfTriangles = mesh->triangles.size();
		this->meshes.push_back(mesh);

		meshElement = meshElement->NextSiblingElement("Mesh");
	}
}

void Scene::assignColorToPixel(int i, int j, Color c)
{
	this->image[i][j].r = c.r;
	this->image[i][j].g = c.g;
	this->image[i][j].b = c.b;
}

/*
	Initializes image with background color
*/
void Scene::initializeImage(Camera *camera)
{
	if (this->image.empty())
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			vector<Color> rowOfColors;
			vector<double> rowOfDepths;

			for (int j = 0; j < camera->verRes; j++)
			{
				rowOfColors.push_back(this->backgroundColor);
				rowOfDepths.push_back(1.01);
			}

			this->image.push_back(rowOfColors);
			this->depth.push_back(rowOfDepths);
		}
	}
	else
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			for (int j = 0; j < camera->verRes; j++)
			{
				assignColorToPixel(i, j, this->backgroundColor);
				this->depth[i][j] = 1.01;
				this->depth[i][j] = 1.01;
				this->depth[i][j] = 1.01;
			}
		}
	}
}

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
int Scene::makeBetweenZeroAnd255(double value)
{
	if (value >= 255.0)
		return 255;
	if (value <= 0.0)
		return 0;
	return (int)(value);
}

/*
	Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera)
{
	ofstream fout;

	fout.open(camera->outputFilename.c_str());

	fout << "P3" << endl;
	fout << "# " << camera->outputFilename << endl;
	fout << camera->horRes << " " << camera->verRes << endl;
	fout << "255" << endl;

	for (int j = camera->verRes - 1; j >= 0; j--)
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
		}
		fout << endl;
	}
	fout.close();
}

/*
	Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
*/
void Scene::convertPPMToPNG(string ppmFileName)
{
	string command;

	// TODO: Change implementation if necessary.
	command = "./magick convert " + ppmFileName + " " + ppmFileName + ".png";
	system(command.c_str());
}

/*
	Transformations, clipping, culling, rasterization are done here.
*/
void Scene::forwardRenderingPipeline(Camera *camera)
{
	// for each mesh, create and map (indexOfVertices,Vec3)
	vector<map<int,Vec3>> deepCopyVerticesForMeshes;
	deepCopyVerticesForMeshes.resize(vertices.size());


	// create transformation matrix
	for(int i=0; i<meshes.size();i++)
	{
		Matrix4 transformationMatrix = getIdentityMatrix();
		// create transformation matrix
		for(int j=0; j<meshes[i]->numberOfTransformations;j++)
		{
			char type = meshes[i]->transformationTypes[j];
			int index = meshes[i]->transformationIds[j]-1;
			Matrix4 temp = getIdentityMatrix();

			if (type=='s') 
			{
				Scaling *scaling = scalings[index];
				temp.values[0][0] = scaling->sx;
				temp.values[1][1] = scaling->sy;
				temp.values[2][2] = scaling->sz;
			} 
			else if (type=='t') 
			{
				Translation* translation = translations[index];
				temp.values[0][3] = translation->tx;
				temp.values[1][3] = translation->ty;
				temp.values[2][3] = translation->tz;
			} 
			else if (type=='r') 
			{
				Rotation* rotation = rotations[index];
				double c = cos(rotation->angle * M_PI / 180.0);
				double s = sin(rotation->angle * M_PI / 180.0);

				double ux = rotation -> ux;
				double uy = rotation -> uy;
				double uz = rotation -> uz;

				temp.values[0][0] = ux * ux * (1 - c) + c;
				temp.values[0][1] = ux * uy * (1 - c) - uz * s;
				temp.values[0][2] = ux * uz * (1 - c) + uy * s;

				temp.values[1][0] = uy * ux * (1 - c) + uz * s;
				temp.values[1][1] = uy * uy * (1 - c) + c;
				temp.values[1][2] = uy * uz * (1 - c) - ux * s;

				temp.values[2][0] = uz * ux * (1 - c) - uy * s;
				temp.values[2][1] = uz * uy * (1 - c) + ux * s;
    			temp.values[2][2] = uz * uz * (1 - c) + c;
			}

			transformationMatrix = multiplyMatrixWithMatrix(temp, transformationMatrix);
		}

		for(int j=0; j<meshes[i]->numberOfTriangles;j++)
		{
			// CALCULATE MATRICES
			Matrix4 cameraTransformationMatrix = getIdentityMatrix();
			Vec3 u = camera -> u;
			Vec3 w = camera -> w;
			Vec3 v = camera -> v;
			Vec3 position = camera -> position;
			// for later use
			double left = camera -> left; 
			double right = camera -> right; 
			double bottom = camera -> bottom; 
			double top = camera -> top; 
			double near = camera -> near; 
			double far = camera -> far;

			cameraTransformationMatrix.values[0][0] = u.x;
			cameraTransformationMatrix.values[0][1] = u.y;
			cameraTransformationMatrix.values[0][2] = u.z;
			cameraTransformationMatrix.values[0][3] = -1 * (u.x * position.x + u.y * position.y + u.z * position.z);

			cameraTransformationMatrix.values[1][0] = v.x;
			cameraTransformationMatrix.values[1][1] = v.y;
			cameraTransformationMatrix.values[1][2] = v.z;
			cameraTransformationMatrix.values[1][3] = -1 * (v.x * position.x + v.y * position.y + v.z * position.z);

			cameraTransformationMatrix.values[2][0] = w.x;
			cameraTransformationMatrix.values[2][1] = w.y;
			cameraTransformationMatrix.values[2][2] = w.z;
			cameraTransformationMatrix.values[2][3] = -1 * (w.x * position.x + w.y * position.y + w.z * position.z);

			// projectionType=0 for orthographic, projectionType=1 for perspective
			Matrix4 projectionMatrix = getIdentityMatrix();
			if (camera->projectionType)
			{
				projectionMatrix.values[0][0] = (2*near) / (right-left);
				projectionMatrix.values[0][2] = (right+left)/(right-left);
				projectionMatrix.values[1][1] = (2*near)/(top-bottom);
				projectionMatrix.values[1][2] = (top+bottom)/(top-bottom);
				projectionMatrix.values[2][2] = (-1) * ( (far+near) / (far-near) );
				projectionMatrix.values[2][3] = (-2) * ( (far*near) / (far-near) );
				projectionMatrix.values[3][2] = -1;
				projectionMatrix.values[3][3] = 0;
			}
			else 
			{
				projectionMatrix.values[0][0] = 2 / (right - left);
				projectionMatrix.values[1][1] = 2 / (top - bottom);
				projectionMatrix.values[2][2] = -2 / (far - near);
				projectionMatrix.values[0][3] = -1 * (right + left) / (right - left);
				projectionMatrix.values[1][3] = -1 * (top + bottom) / (top - bottom);
				projectionMatrix.values[2][3] = -1 * (far + near) / (far - near);
			}

			projectionMatrix = multiplyMatrixWithMatrix(projectionMatrix,cameraTransformationMatrix);
			projectionMatrix = multiplyMatrixWithMatrix(projectionMatrix,transformationMatrix);


			// CALCULATE TRIANGLE
			Triangle triangle = meshes[i]->triangles[j];
			int v1_id = triangle.vertexIds[0]-1;
			int v2_id = triangle.vertexIds[1]-1;
			int v3_id = triangle.vertexIds[2]-1;

			Vec3 vertex1 =  deepCopyVerticesForMeshes[i][v1_id];
			Vec3 vertex2 =  deepCopyVerticesForMeshes[i][v2_id];
			Vec3 vertex3 =  deepCopyVerticesForMeshes[i][v3_id];

			Vec4 vertex1_4 = Vec4(vertex1.x, vertex1.y, vertex1.z, 1, vertex1.colorId);
			Vec4 vertex2_4 = Vec4(vertex2.x, vertex2.y, vertex2.z, 1, vertex2.colorId);
			Vec4 vertex3_4 = Vec4(vertex3.x, vertex3.y, vertex3.z, 1, vertex3.colorId);


			vertex1_4 = multiplyMatrixWithVec4(projectionMatrix, vertex1_4);
			vertex2_4 = multiplyMatrixWithVec4(projectionMatrix, vertex2_4);
			vertex3_4 = multiplyMatrixWithVec4(projectionMatrix, vertex3_4);


			vertex1 = Vec3(vertex1_4.x/vertex1_4.t, vertex1_4.y/vertex1_4.t, vertex1_4.z/vertex1_4.t, vertex1.colorId);
			vertex2 = Vec3(vertex2_4.x/vertex2_4.t, vertex2_4.y/vertex2_4.t, vertex2_4.z/vertex2_4.t, vertex2.colorId);
			vertex3 = Vec3(vertex3_4.x/vertex3_4.t, vertex3_4.y/vertex3_4.t, vertex3_4.z/vertex3_4.t, vertex3.colorId);

			deepCopyVerticesForMeshes[i][v1_id] = vertex1;
			deepCopyVerticesForMeshes[i][v2_id] = vertex2;
			deepCopyVerticesForMeshes[i][v3_id] = vertex3;
		}
	}

	for(int i=0; i<meshes.size();i++)
	{
		map<int,Vec3> meshMap = deepCopyVerticesForMeshes[i];
		for(int j=0; j<meshes[i]->numberOfTriangles;j++)
		{
			Triangle triangle = meshes[i]->triangles[j];
			int v1_id = triangle.vertexIds[0]-1;
			int v2_id = triangle.vertexIds[1]-1;
			int v3_id = triangle.vertexIds[2]-1;

			Vec3 vertex1 =  meshMap[v1_id];
			Vec3 vertex2 =  meshMap[v2_id];
			Vec3 vertex3 =  meshMap[v3_id];

			// check for fuck face culling ( ͡° ͜ʖ ͡°)
			if (cullingEnabled) 
			{
				Vec3 normal;
				Vec3 a = vertex1 - vertex2;
				Vec3 b = vertex3 - vertex1;
				normal.z = a.x * b.y - b.x * a.y;
				normal.x = a.y * b.z - b.y * a.z;
				normal.y = b.x * a.z - a.x * b.z;

				double dotProduct = normal.x * vertex1.x + normal.y * vertex1.y + normal.z * vertex1.z;
				if (dotProduct > 0)
					continue;
			}

			// viewportMatrix is used in both scenario
			Matrix4 viewportMatrix  = getIdentityMatrix();
			int horRes = camera -> horRes;
			int verRes = camera -> verRes;

			viewportMatrix.values[0][0] = horRes * 0.5;
			viewportMatrix.values[1][1] = verRes * 0.5;
			viewportMatrix.values[2][2] = 0.5;
			viewportMatrix.values[2][3] = 0.5;
			viewportMatrix.values[0][3] = (horRes-1) * 0.5;
			viewportMatrix.values[1][3] = (verRes-1) * 0.5;

			// 0 for wireframe, 1 for solid
			if(!(meshes[i]->type))
			{
				Line tempLine1(vertex1,vertex2,*colorsOfVertices[vertex1.colorId-1],*colorsOfVertices[vertex2.colorId-1]);
				Line line1 = clipLine(tempLine1);
				Line tempLine2(vertex2,vertex3,*colorsOfVertices[vertex2.colorId-1],*colorsOfVertices[vertex3.colorId-1]);
				Line line2 = clipLine(tempLine2);
				Line tempLine3(vertex3,vertex1,*colorsOfVertices[vertex3.colorId-1],*colorsOfVertices[vertex1.colorId-1]);
				Line line3 = clipLine(tempLine3);

				// FOR LINE 1
				Vec3 l1p1 = line1.v1;
				// HOMO ( ͡° ͜ʖ ͡°)
				Vec4 homo = Vec4(l1p1.x, l1p1.y, l1p1.z, 1, l1p1.colorId);
				homo = multiplyMatrixWithVec4(viewportMatrix, homo);
				line1.v1 = Vec3(homo.x, homo.y, homo.z, homo.colorId);

				Vec3 l1p2 = line1.v2;
				homo = Vec4(l1p2.x, l1p2.y, l1p2.z, 1, l1p2.colorId);
				homo = multiplyMatrixWithVec4(viewportMatrix, homo);
				line1.v2 = Vec3(homo.x, homo.y, homo.z, homo.colorId);


				// FOR LINE 2
				Vec3 l2p1 = line2.v1;
				homo = Vec4(l2p1.x, l2p1.y, l2p1.z, 1, l2p1.colorId);
				homo = multiplyMatrixWithVec4(viewportMatrix, homo);
				line2.v1 = Vec3(homo.x, homo.y, homo.z, homo.colorId);

				Vec3 l2p2 = line2.v2;
				homo = Vec4(l2p2.x, l2p2.y, l2p2.z, 1, l2p2.colorId);
				homo = multiplyMatrixWithVec4(viewportMatrix, homo);
				line2.v2 = Vec3(homo.x, homo.y, homo.z, homo.colorId);

				// FOR LINE 3
				Vec3 l3p1 = line3.v1;
				homo = Vec4(l3p1.x, l3p1.y, l3p1.z, 1, l3p1.colorId);
				homo = multiplyMatrixWithVec4(viewportMatrix, homo);
				line3.v1 = Vec3(homo.x, homo.y, homo.z, homo.colorId);

				Vec3 l3p2 = line3.v2;
				homo = Vec4(l3p2.x, l3p2.y, l3p2.z, 1, l3p2.colorId);
				homo = multiplyMatrixWithVec4(viewportMatrix, homo);
				line3.v2 = Vec3(homo.x, homo.y, homo.z, homo.colorId);

				// bool value to reverse point in line2
				rasterLine(depth, camera, image, line1, false);
				rasterLine(depth, camera, image, line2, true);
				rasterLine(depth, camera, image, line3, false);


			}
			else 
			{
				// vertex1
				Vec4 homo = Vec4(vertex1.x, vertex1.y, vertex1.z, 1, vertex1.colorId);
				homo = multiplyMatrixWithVec4(viewportMatrix, homo);
				vertex1 = Vec3(homo.x, homo.y, homo.z, homo.colorId);

				// vertex2
				homo = Vec4(vertex2.x, vertex2.y, vertex2.z, 1, vertex2.colorId);
				homo = multiplyMatrixWithVec4(viewportMatrix, homo);
				vertex2 = Vec3(homo.x, homo.y, homo.z, homo.colorId);

				// vertex3
				homo = Vec4(vertex3.x, vertex3.y, vertex3.z, 1, vertex3.colorId);
				homo = multiplyMatrixWithVec4(viewportMatrix, homo);
				vertex3 = Vec3(homo.x, homo.y, homo.z, homo.colorId);

				rasterTriangle(depth, camera, image, vertex1, vertex2, vertex3, *colorsOfVertices[vertex1.colorId-1], *colorsOfVertices[vertex2.colorId-1], *colorsOfVertices[vertex3.colorId-1]);
			}
		}

	}
}
