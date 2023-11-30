#pragma once
#include "../ew/mesh.h"
namespace bp {
	ew::MeshData createSphere(float radius, int numSegments);
	ew::MeshData createCylinder(float height, float radius, int numSegments);
	ew::MeshData createPlane(float size, int subdivisions);
	ew::MeshData createTorus(int ringCount, int ringSubDivisions, float outerRadius, float innerRadius);
}