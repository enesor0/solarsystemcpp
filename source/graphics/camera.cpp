#include "graphics/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>


Camera::Camera()
	: Camera(
		glm::vec3(0.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		0.0f
	)
{
}

Camera::Camera(
	const glm::vec3& position,
	const glm::vec3& worldUp,
	float yaw,
	float pitch)
	: position_(position)
	, worldUp_(worldUp)
	, yaw_(yaw)
	, pitch_(pitch)
{
	updateCameraVectors();
}

glm::mat4 Camera::viewMatrix() const
{
	return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::move(CameraDirection direction, float deltaTime)
{
	const float velocity = movementSpeed_ * deltaTime;

	switch (direction)
	{
	case CameraDirection::Forward:
		position_ += front_ * velocity;
		break;

	case CameraDirection::Backward:
		position_ -= front_ * velocity;
		break;

	case CameraDirection::Left:
		position_ -= right_ * velocity;
		break;

	case CameraDirection::Right:
		position_ += right_ * velocity;
		break;
	}
}

void Camera::rotate(float xOffset, float yOffset)
{
	yaw_ += xOffset * mouseSensitivity_;
	pitch_ += yOffset * mouseSensitivity_;

	if (pitch_ > 89.0f)
	{
		pitch_ = 89.0f;
	}

	if (pitch_ < -89.0f)
	{
		pitch_ = -89.0f;
	}

	updateCameraVectors();
}

void Camera::setPose(
	const glm::vec3& position,
	const glm::vec3& up,
	float yaw,
	float pitch)
{
	position_ = position;
	worldUp_ = up;
	yaw_ = yaw;
	pitch_ = pitch;

	updateCameraVectors();
}

bool Camera::transitionToPose(
	const glm::vec3& position,
	const glm::vec3& up,
	float yaw,
	float pitch,
	float deltaTime)
{
	const float positionFactor = 1.0f - std::exp(-3.6f * deltaTime);
	const float directionFactor = 1.0f - std::exp(-5.2f * deltaTime);
	const float yawRadians = glm::radians(yaw);
	const float pitchRadians = glm::radians(pitch);
	const glm::vec3 desiredDirection(
		std::cos(yawRadians) * std::cos(pitchRadians),
		std::sin(pitchRadians),
		std::sin(yawRadians) * std::cos(pitchRadians)
	);

	position_ = glm::mix(position_, position, positionFactor);
	worldUp_ = glm::normalize(glm::mix(worldUp_, up, directionFactor));
	const glm::vec3 smoothedDirection = glm::normalize(
		glm::mix(front_, desiredDirection, directionFactor)
	);
	yaw_ = glm::degrees(std::atan2(
		smoothedDirection.z,
		smoothedDirection.x
	));
	pitch_ = glm::degrees(std::asin(smoothedDirection.y));
	updateCameraVectors();

	return glm::length(position_ - position) < 0.02f
		&& glm::dot(front_, desiredDirection) > 0.9995f;
}

void Camera::followOrbitTarget(
	const glm::vec3& target,
	float distance,
	float azimuth,
	float elevation,
	float deltaTime)
{
	const float azimuthRadians = glm::radians(azimuth);
	const float elevationRadians = glm::radians(elevation);
	const float orbitDistance = distance * 1.22f;
	const glm::vec3 orbitOffset(
		orbitDistance * std::cos(elevationRadians) * std::sin(azimuthRadians),
		orbitDistance * std::sin(elevationRadians),
		orbitDistance * std::cos(elevationRadians) * std::cos(azimuthRadians)
	);
	const glm::vec3 desiredPosition = target + orbitOffset;
	const float positionFactor = 1.0f - std::exp(-5.5f * deltaTime);

	position_ = glm::mix(position_, desiredPosition, positionFactor);

	const glm::vec3 targetDirection = glm::normalize(target - position_);
	const glm::vec3 smoothedDirection = glm::normalize(
		glm::mix(front_, targetDirection, positionFactor)
	);

	yaw_ = glm::degrees(std::atan2(
		smoothedDirection.z,
		smoothedDirection.x
	));
	pitch_ = glm::degrees(std::asin(smoothedDirection.y));

	updateCameraVectors();
}

const glm::vec3& Camera::position() const
{
	return position_;
}

void Camera::updateCameraVectors()
{
	glm::vec3& direction = front_;

	direction.x = std::cos(glm::radians(yaw_))
		* std::cos(glm::radians(pitch_));

	direction.y = std::sin(glm::radians(pitch_));

	direction.z = std::sin(glm::radians(yaw_))
		* std::cos(glm::radians(pitch_));

	front_ = glm::normalize(direction);
	right_ = glm::normalize(glm::cross(front_, worldUp_));
	up_ = glm::normalize(glm::cross(right_, front_));
}
