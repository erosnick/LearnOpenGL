#include "Camera.h"

Camera::Camera()
{

}

void Camera::setFOV(float inFov)
{
	fov = inFov;
}

void Camera::setAspectRatio(float inAspectRatio)
{
	aspectRatio = inAspectRatio;
}

void Camera::setPosition(const glm::vec3& position)
{
	cameraPosition - position;
}

void Camera::lookAt(const glm::vec3& center)
{
	cameraFront = center - cameraPosition;
}

void Camera::setFront(const glm::vec3 front)
{
	cameraFront = front;
}

void Camera::setPitch(float inPitch)
{
	pitch = inPitch;

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	cameraFront = calculateRotation(pitch, yaw);
}

void Camera::setYaw(float inYaw)
{
	yaw = inYaw;

	cameraFront = calculateRotation(pitch, yaw);
}

void Camera::forward(float delta)
{
	cameraPosition += cameraFront * cameraSpeed * delta;
}

void Camera::right(float delta)
{
	cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * delta;
}

void Camera::up(float delta)
{
	cameraPosition += cameraUp * cameraSpeed * delta;
}

glm::mat4& Camera::viewMatrix()
{
	view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
	return view;
}

glm::mat4& Camera::projectionMatrix()
{
	float tanHalfFovY = glm::tan(fov / 2);

	projection[0][0] = 1.0f / (aspectRatio * tanHalfFovY);
	projection[1][1] = 1.0f / tanHalfFovY;
	projection[2][2] = -(farZ + nearZ) / (farZ - nearZ);
	projection[2][3] = -1.0f;
	projection[3][2] = -(2 * farZ * nearZ) / (farZ - nearZ);

	return projection;
}

glm::vec3 Camera::calculateRotation(float pitch, float yaw)
{
	glm::vec3 front;

	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	return glm::normalize(front);
}
