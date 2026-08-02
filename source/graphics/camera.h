#pragma once

#include <glm/glm.hpp>

enum class CameraDirection
{
	Forward,
	Backward,
	Left,
	Right
};

class Camera
{
public:
	Camera();
	
	
	Camera(
		const glm::vec3& position,
		const glm::vec3& up,
		float yaw,
		float pitch
	);
	glm::mat4 viewMatrix() const;

	void move(CameraDirection direction, float deltaTime);
	void rotate(float xOffset, float yOffset);
	void setPose(
		const glm::vec3& position,
		const glm::vec3& up,
		float yaw,
		float pitch
	);
	bool transitionToPose(
		const glm::vec3& position,
		const glm::vec3& up,
		float yaw,
		float pitch,
		float deltaTime
	);
	void followOrbitTarget(
		const glm::vec3& target,
		float distance,
		float azimuth,
		float elevation,
		float deltaTime
	);

	const glm::vec3& position() const;

private:

	void updateCameraVectors();

	glm::vec3 position_;
	glm::vec3 front_;
	glm::vec3 up_;
	glm::vec3 right_;
	glm::vec3 worldUp_;

	float yaw_;
	float pitch_;

	static constexpr float movementSpeed_ = 3.0f;

	static constexpr float mouseSensitivity_ = 0.1f;
};
