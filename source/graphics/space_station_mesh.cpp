#include "graphics/space_station_mesh.h"

#include <array>
#include <cmath>

namespace
{
	constexpr float Pi = 3.14159265358979323846f;

	void appendQuad(
		MeshData& data,
		const std::array<glm::vec3, 4>& corners,
		const glm::vec3& normal)
	{
		const unsigned int firstIndex = static_cast<unsigned int>(
			data.vertices.size()
		);
		constexpr std::array<glm::vec2, 4> textureCoordinates{
			glm::vec2(0.0f, 0.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(0.0f, 1.0f)
		};

		for (std::size_t cornerIndex = 0;
			cornerIndex < corners.size();
			++cornerIndex)
		{
			data.vertices.push_back({
				corners[cornerIndex], normal, textureCoordinates[cornerIndex]
			});
		}

		data.indices.insert(
			data.indices.end(),
			{ firstIndex, firstIndex + 1, firstIndex + 2,
				firstIndex, firstIndex + 2, firstIndex + 3 }
		);
	}

	void appendBox(
		MeshData& data,
		const glm::vec3& halfExtents,
		const glm::vec3& center = glm::vec3(0.0f),
		const glm::vec3& axisX = glm::vec3(1.0f, 0.0f, 0.0f),
		const glm::vec3& axisY = glm::vec3(0.0f, 1.0f, 0.0f),
		const glm::vec3& axisZ = glm::vec3(0.0f, 0.0f, 1.0f))
	{
		const float x = halfExtents.x;
		const float y = halfExtents.y;
		const float z = halfExtents.z;
		const auto toWorld = [&center, &axisX, &axisY, &axisZ](
			const glm::vec3& localPosition)
		{
			return center
				+ axisX * localPosition.x
				+ axisY * localPosition.y
				+ axisZ * localPosition.z;
		};

		const auto normalToWorld = [&axisX, &axisY, &axisZ](
			const glm::vec3& localNormal)
		{
			return glm::normalize(
				axisX * localNormal.x
				+ axisY * localNormal.y
				+ axisZ * localNormal.z
			);
		};

		const std::array<std::pair<glm::vec3, std::array<glm::vec3, 4>>, 6>
			faces{
				std::make_pair(glm::vec3(1.0f, 0.0f, 0.0f),
					std::array<glm::vec3, 4>{
						glm::vec3(x, -y, -z), glm::vec3(x, -y, z),
						glm::vec3(x, y, z), glm::vec3(x, y, -z) }),
				std::make_pair(glm::vec3(-1.0f, 0.0f, 0.0f),
					std::array<glm::vec3, 4>{
						glm::vec3(-x, -y, z), glm::vec3(-x, -y, -z),
						glm::vec3(-x, y, -z), glm::vec3(-x, y, z) }),
				std::make_pair(glm::vec3(0.0f, 1.0f, 0.0f),
					std::array<glm::vec3, 4>{
						glm::vec3(-x, y, -z), glm::vec3(x, y, -z),
						glm::vec3(x, y, z), glm::vec3(-x, y, z) }),
				std::make_pair(glm::vec3(0.0f, -1.0f, 0.0f),
					std::array<glm::vec3, 4>{
						glm::vec3(-x, -y, z), glm::vec3(x, -y, z),
						glm::vec3(x, -y, -z), glm::vec3(-x, -y, -z) }),
				std::make_pair(glm::vec3(0.0f, 0.0f, 1.0f),
					std::array<glm::vec3, 4>{
						glm::vec3(x, -y, z), glm::vec3(-x, -y, z),
						glm::vec3(-x, y, z), glm::vec3(x, y, z) }),
				std::make_pair(glm::vec3(0.0f, 0.0f, -1.0f),
					std::array<glm::vec3, 4>{
						glm::vec3(-x, -y, -z), glm::vec3(x, -y, -z),
						glm::vec3(x, y, -z), glm::vec3(-x, y, -z) })
			};

		for (const auto& face : faces)
		{
			std::array<glm::vec3, 4> worldCorners;
			for (std::size_t cornerIndex = 0;
				cornerIndex < face.second.size();
				++cornerIndex)
			{
				worldCorners[cornerIndex] = toWorld(face.second[cornerIndex]);
			}

			appendQuad(data, worldCorners, normalToWorld(face.first));
		}
	}

	void appendBeam(
		MeshData& data,
		const glm::vec3& start,
		const glm::vec3& end,
		float halfWidth)
	{
		const glm::vec3 beamAxis = end - start;
		const float length = glm::length(beamAxis);
		const glm::vec3 axisX = beamAxis / length;
		const glm::vec3 referenceAxis = std::abs(axisX.y) > 0.92f
			? glm::vec3(0.0f, 0.0f, 1.0f)
			: glm::vec3(0.0f, 1.0f, 0.0f);
		const glm::vec3 axisZ = glm::normalize(glm::cross(axisX, referenceAxis));
		const glm::vec3 axisY = glm::normalize(glm::cross(axisZ, axisX));

		appendBox(
			data,
			glm::vec3(length * 0.5f, halfWidth, halfWidth),
			(start + end) * 0.5f,
			axisX,
			axisY,
			axisZ
		);
	}

	void appendCylinder(
		MeshData& data,
		const glm::vec3& center,
		const glm::vec3& axisDirection,
		float radius,
		float halfLength,
		int segmentCount = 12)
	{
		const glm::vec3 axis = glm::normalize(axisDirection);
		const glm::vec3 referenceAxis = std::abs(axis.y) > 0.92f
			? glm::vec3(0.0f, 0.0f, 1.0f)
			: glm::vec3(0.0f, 1.0f, 0.0f);
		const glm::vec3 radialX = glm::normalize(glm::cross(referenceAxis, axis));
		const glm::vec3 radialY = glm::normalize(glm::cross(axis, radialX));
		const glm::vec3 frontCenter = center + axis * halfLength;
		const glm::vec3 backCenter = center - axis * halfLength;

		for (int segmentIndex = 0;
			segmentIndex < segmentCount;
			++segmentIndex)
		{
			const float firstAngle = 2.0f * Pi
				* static_cast<float>(segmentIndex) / segmentCount;
			const float secondAngle = 2.0f * Pi
				* static_cast<float>(segmentIndex + 1) / segmentCount;
			const glm::vec3 firstRadial = radialX * std::cos(firstAngle)
				+ radialY * std::sin(firstAngle);
			const glm::vec3 secondRadial = radialX * std::cos(secondAngle)
				+ radialY * std::sin(secondAngle);

			appendQuad(data, {
				backCenter + firstRadial * radius,
				backCenter + secondRadial * radius,
				frontCenter + secondRadial * radius,
				frontCenter + firstRadial * radius
			}, glm::normalize(firstRadial + secondRadial));

			appendQuad(data, {
				frontCenter,
				frontCenter + firstRadial * radius,
				frontCenter + secondRadial * radius,
				frontCenter
			}, axis);

			appendQuad(data, {
				backCenter,
				backCenter + secondRadial * radius,
				backCenter + firstRadial * radius,
				backCenter
			}, -axis);
		}
	}
}

SpaceStationMesh::SpaceStationMesh()
	: moduleMesh_(createModuleMeshData())
	, trussMesh_(createTrussMeshData())
	, solarPanelMesh_(createSolarPanelMeshData())
	, radiatorMesh_(createRadiatorMeshData())
	, detailMesh_(createDetailMeshData())
{
}

void SpaceStationMesh::drawModules() const
{
	moduleMesh_.draw();
}

void SpaceStationMesh::drawTruss() const
{
	trussMesh_.draw();
}

void SpaceStationMesh::drawSolarPanels() const
{
	solarPanelMesh_.draw();
}

void SpaceStationMesh::drawRadiators() const
{
	radiatorMesh_.draw();
}

void SpaceStationMesh::drawDetails() const
{
	detailMesh_.draw();
}

void SpaceStationMesh::drawAll() const
{
	drawModules();
	drawTruss();
	drawSolarPanels();
	drawRadiators();
	drawDetails();
}

MeshData SpaceStationMesh::createModuleMeshData()
{
	MeshData data;

	// The long Z-axis module train is the station's recognizable pressurized core.
	appendCylinder(
		data, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		0.145f, 0.57f
	);
	appendCylinder(
		data, glm::vec3(0.0f, 0.0f, 0.72f), glm::vec3(0.0f, 0.0f, 1.0f),
		0.125f, 0.16f
	);
	appendCylinder(
		data, glm::vec3(0.0f, 0.0f, -0.72f), glm::vec3(0.0f, 0.0f, 1.0f),
		0.125f, 0.16f
	);
	appendCylinder(
		data, glm::vec3(0.0f, -0.01f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
		0.112f, 0.24f
	);
	appendBox(data, glm::vec3(0.16f, 0.08f, 0.20f), glm::vec3(0.0f, -0.17f, 0.0f));

	return data;
}

MeshData SpaceStationMesh::createTrussMeshData()
{
	MeshData data;
	constexpr float trussHeight = 0.22f;
	constexpr float trussDepth = 0.085f;
	constexpr std::array<float, 6> trussNodes{
		-1.35f, -0.81f, -0.27f, 0.27f, 0.81f, 1.35f
	};

	appendBeam(
		data, glm::vec3(-1.45f, trussHeight, -trussDepth),
		glm::vec3(1.45f, trussHeight, -trussDepth), 0.025f
	);
	appendBeam(
		data, glm::vec3(-1.45f, trussHeight, trussDepth),
		glm::vec3(1.45f, trussHeight, trussDepth), 0.025f
	);

	for (std::size_t nodeIndex = 0; nodeIndex < trussNodes.size(); ++nodeIndex)
	{
		const float node = trussNodes[nodeIndex];
		appendBeam(
			data, glm::vec3(node, trussHeight, -trussDepth),
			glm::vec3(node, trussHeight, trussDepth), 0.022f
		);

		if (nodeIndex + 1 < trussNodes.size())
		{
			const float nextNode = trussNodes[nodeIndex + 1];
			const bool risesForward = nodeIndex % 2 == 0;
			appendBeam(
				data,
				glm::vec3(node, trussHeight, risesForward ? -trussDepth : trussDepth),
				glm::vec3(nextNode, trussHeight, risesForward ? trussDepth : -trussDepth),
				0.018f
			);
		}
	}

	appendBeam(data, glm::vec3(0.0f, 0.12f, 0.0f), glm::vec3(0.0f, trussHeight, 0.0f), 0.040f);
	return data;
}

MeshData SpaceStationMesh::createSolarPanelMeshData()
{
	MeshData data;
	constexpr std::array<float, 8> panelCenters{
		-1.47f, -1.18f, -0.89f, -0.60f,
		0.60f, 0.89f, 1.18f, 1.47f
	};

	for (const float panelCenter : panelCenters)
	{
		appendBox(
			data,
			glm::vec3(0.132f, 0.009f, 0.265f),
			glm::vec3(panelCenter, 0.275f, 0.0f)
		);
	}

	return data;
}

MeshData SpaceStationMesh::createRadiatorMeshData()
{
	MeshData data;
	appendBox(data, glm::vec3(0.36f, 0.010f, 0.095f), glm::vec3(-0.34f, -0.22f, 0.48f));
	appendBox(data, glm::vec3(0.36f, 0.010f, 0.095f), glm::vec3(0.34f, -0.22f, -0.48f));

	return data;
}

MeshData SpaceStationMesh::createDetailMeshData()
{
	MeshData data;
	constexpr std::array<float, 8> panelCenters{
		-1.47f, -1.18f, -0.89f, -0.60f,
		0.60f, 0.89f, 1.18f, 1.47f
	};

	// Thin seams turn the panels into an array rather than a single blue slab.
	for (const float panelCenter : panelCenters)
	{
		appendBox(
			data,
			glm::vec3(0.006f, 0.004f, 0.272f),
			glm::vec3(panelCenter, 0.288f, 0.0f)
		);
		appendBox(
			data,
			glm::vec3(0.135f, 0.004f, 0.006f),
			glm::vec3(panelCenter, 0.288f, 0.0f)
		);
	}

	appendCylinder(
		data, glm::vec3(0.0f, 0.0f, 0.92f), glm::vec3(0.0f, 0.0f, 1.0f),
		0.092f, 0.028f
	);
	appendCylinder(
		data, glm::vec3(0.0f, 0.0f, -0.92f), glm::vec3(0.0f, 0.0f, 1.0f),
		0.092f, 0.028f
	);
	appendBeam(data, glm::vec3(0.10f, 0.13f, -0.22f), glm::vec3(0.22f, 0.43f, -0.32f), 0.012f);
	appendBox(data, glm::vec3(0.035f, 0.014f, 0.035f), glm::vec3(0.22f, 0.43f, -0.32f));

	return data;
}
