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