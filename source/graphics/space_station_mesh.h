#pragma once

#include "graphics/mesh.h"

class SpaceStationMesh
{
public:
	SpaceStationMesh();

	void drawModules() const;
	void drawTruss() const;
	void drawSolarPanels() const;
	void drawRadiators() const;
	void drawDetails() const;
	void drawAll() const;

private:
	static MeshData createModuleMeshData();
	static MeshData createTrussMeshData();
	static MeshData createSolarPanelMeshData();
	static MeshData createRadiatorMeshData();
	static MeshData createDetailMeshData();

	Mesh moduleMesh_;
	Mesh trussMesh_;
	Mesh solarPanelMesh_;
	Mesh radiatorMesh_;
	Mesh detailMesh_;
};
