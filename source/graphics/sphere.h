#pragma once

#include "graphics/mesh.h"

class Sphere
{
public:
	Sphere(
		float radius = 1.0f,
		unsigned int sectorCount = 64,
		unsigned int stackCount = 32
	);

	void draw() const;


private:
	static MeshData createMeshData(
		float radius,
		unsigned int sectorCount,
		unsigned int stackCount
	);


	Mesh mesh_;
};
