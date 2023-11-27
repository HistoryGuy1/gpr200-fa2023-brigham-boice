#include "procGen.h"
#include <cmath>
#include <cstdlib>

namespace myLib {
	ew::MeshData myLib::createSphere(float radius, int numSegments)
	{
		ew::MeshData sphereData;

		float thetaStep = 2 * 3.1415f / numSegments;
		float phiStep = 3.1415f / numSegments;

		for (int row = 0; row <= numSegments; row++) 
		{
			//First and last row converge at poles
			float phi = row * phiStep;
			for (int col = 0; col <= numSegments; col++) {//Duplicate column for each row
				float theta = (float)col * thetaStep;

				ew::Vertex v;

				srand((unsigned)time(0));

				v.pos.x = ((rand() % 10) + radius) * cos(theta) * sin(phi);
				v.pos.y = ((rand() % 10) + radius) * cos(phi);
				v.pos.z = ((rand() % 10) + radius) * sin(theta) * sin(phi);

				v.normal = ew::Normalize(ew::Vec3(cos(theta) * sin(phi), cos(phi), sin(theta) * sin(phi)));

				v.uv = ew::Vec2(1- (float)col / (float)numSegments, (float)row / (float)numSegments);

				sphereData.vertices.push_back(v);
			}
		}

		int columns = numSegments + 1;

		for (int row = 0; row < numSegments; row++) 
		{
			for (int col = 0; col < numSegments; col++) 
			{
				int start = row * columns + col;

				//Triangle 1
				sphereData.indices.push_back(start);
				sphereData.indices.push_back(start + 1);
				sphereData.indices.push_back(start + columns);
				
				//Triangle 2…
				sphereData.indices.push_back(start + 1);
				sphereData.indices.push_back(start + columns + 1);
				sphereData.indices.push_back(start + columns);
			}
		}

		return sphereData;
	}

	ew::MeshData myLib::createCylinder(float height, float radius, int numSegments)
	{
        ew::MeshData cylinderData;

        // Calculate the step angle
        float thetaStep = 2 * 3.1415f / numSegments;
        float topY = height / 2;
        float bottomY = -topY;

		// Top center vertex
		ew::Vertex topCenter;
		topCenter.pos = ew::Vec3(0, topY, 0);
		topCenter.normal = ew::Vec3(0, 1, 0); // Normal points up
		topCenter.uv = ew::Vec2(0.5f, 0.5f);
		cylinderData.vertices.push_back(topCenter);

		int centerT = 0; //Index of center vertex top
		int startT = cylinderData.vertices.size(); //Index of first ring vertex

		// Top ring vertices
		for (int i = 0; i <= numSegments; i++)
		{
			float theta = i * thetaStep;
			float x = cos(theta) * radius;
			float z = sin(theta) * radius;

			// Top ring vertex
			ew::Vertex vTop;
			vTop.pos = ew::Vec3(x, topY, z);
			vTop.normal = ew::Vec3(0, 1, 0); // Normal points up for top ring

			vTop.uv = ew::Vec2((x + 1)/2, (z + 1) / 2);

			cylinderData.vertices.push_back(vTop);
		}

		int startSide = cylinderData.vertices.size(); //Index of first ring vertex side
		
		for (int i = 0; i <= numSegments; i++)
		{
			float theta = i * thetaStep;
			float x = cos(theta) * radius;
			float z = sin(theta) * radius;

			// Top ring vertex
			ew::Vertex vTop;
			vTop.pos = ew::Vec3(x, topY, z);
			vTop.normal = ew::Vec3(x / radius, 0, z / radius); // Normal points up for top ring
			vTop.uv = ew::Vec2((float)i / (float)numSegments, 1);

			cylinderData.vertices.push_back(vTop);
		}

		for (int i = 0; i <= numSegments; i++)
		{
			float theta = i * thetaStep;
			float x = cos(theta) * radius;
			float z = sin(theta) * radius;

			// Bottom ring vertex
			ew::Vertex vBottom;
			vBottom.pos = ew::Vec3(x, bottomY, z);
			vBottom.normal = ew::Vec3(x / radius, 0, z / radius); // Normal points down for bottom ring
			vBottom.uv = ew::Vec2((float)i / (float)numSegments, 0);

			cylinderData.vertices.push_back(vBottom);
		}

		int startB = cylinderData.vertices.size(); //Index of second ring vertexstartSide

		// Bottom ring vertices
		for (int i = 0; i <= numSegments; i++)
		{
			float theta = i * thetaStep;
			float x = cos(theta) * radius;
			float z = sin(theta) * radius;

			// Bottom ring vertex
			ew::Vertex vBottom;
			vBottom.pos = ew::Vec3(x, bottomY, z);
			vBottom.normal = ew::Vec3(0, -1, 0); // Normal points down for bottom ring
			vBottom.uv = ew::Vec2((x + 1) / 2, (z + 1) / 2);

			cylinderData.vertices.push_back(vBottom);
		}

		int centerB = cylinderData.vertices.size(); //Index of center vertex bottom

		// Bottom center vertex
		ew::Vertex bottomCenter;
		bottomCenter.pos = ew::Vec3(0, bottomY, 0);
		bottomCenter.normal = ew::Vec3(0, -1, 0); // Normal points down
		bottomCenter.uv = ew::Vec2(0.5f, 0.5f);

		cylinderData.vertices.push_back(bottomCenter);


		for (int i = 0; i < numSegments; i++)
		{
			cylinderData.indices.push_back(startT + i);
			cylinderData.indices.push_back(centerT);
			cylinderData.indices.push_back(startT + i + 1);
		}

		for (int i = 0; i < numSegments; i++)
		{
			cylinderData.indices.push_back(centerB);
			cylinderData.indices.push_back(startB + i);
			cylinderData.indices.push_back(startB + i + 1);
		}

		int columns = numSegments + 1;

		for (int i = 0; i < columns; i++) 
		{
			int start = startSide + i;

			//Triangle 1
			cylinderData.indices.push_back(start);
			cylinderData.indices.push_back(start + 1);
			cylinderData.indices.push_back(start + columns);

			//Triangle 2…
			cylinderData.indices.push_back(start + 1);
			cylinderData.indices.push_back(start + columns + 1);
			cylinderData.indices.push_back(start + columns);
		}

        return cylinderData;
	}

	ew::MeshData myLib::createPlane(float width, float height, int subdivisions)
	{
		ew::MeshData planeData;

		//Verticies
		for (int row = 0; row <= subdivisions; row++)
		{
			for (int col = 0; col <= subdivisions; col++)
			{
				ew::Vertex v;
				v.pos.x = width * ((float)col / (float)subdivisions);
				v.pos.z = -height * ((float)row / (float)subdivisions);

				v.normal = ew::Vec3(0.0f, 1.0f, 0.0f);
				v.uv = ew::Vec2((float)col / (float)subdivisions, (float)row / (float)subdivisions);

				planeData.vertices.push_back(v);
			}
		}

		int columns = subdivisions + 1;
		// Indices
		for (int row = 0; row < subdivisions; row++) 
		{
			for (int col = 0; col < subdivisions; col++) 
			{
				int start = row * columns + col;

				// Bottom right triangle
				planeData.indices.push_back(start);
				planeData.indices.push_back(start + 1);
				planeData.indices.push_back(start + columns + 1);

				// Top left triangle
				planeData.indices.push_back(start);
				planeData.indices.push_back(start + columns + 1);
				planeData.indices.push_back(start + columns);
			}
		}

		return planeData;
	}
}
