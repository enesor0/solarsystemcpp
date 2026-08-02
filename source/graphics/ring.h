#pragma once

#include "graphics/mesh.h"

class Ring
{
public:
	Ring(
		float innerRadius,
		float outerRadius,
		unsigned int sectorCount = 96
	);

	void draw() const;

private:
	static MeshData createMeshData(
		float innerRadius,
		float outerRadius,
		unsigned int sectorCount
	);

	Mesh mesh_;
};
