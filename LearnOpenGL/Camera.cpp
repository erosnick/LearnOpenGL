#include "Camera.h"
#include <iostream>

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
	cameraPosition = position;
}

void Camera::lookAt(const glm::vec3& center)
{
	cameraFront = glm::normalize(center - cameraPosition);

	float dot = glm::dot(cameraFront, glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)));
	float angle = glm::degrees(glm::acos(dot));

	// 判断向量B在向量A的左边还是右边。
	// 向量A为基准向量, -z轴。
	// B X A的结果中，如果y分量>0, 则A位于B左边；如果y分量<0,则A位于B右边。
	glm::vec3 cross = glm::cross(cameraFront, glm::vec3(0.0f, 0.0f, -1.0f));
	std::cout << "angle:" << angle << std::endl;
	std::cout << "z:" << cross.y << std::endl;

	if (cross.y > 0)
	{
		yaw += angle;
	}
	else
	{
		yaw -= angle;
	}

	std::cout << "cameraFront(" << cameraFront.x << "," << cameraFront.y << "," << cameraFront.z << ")" << std::endl;
	std::cout << "cameraUp(" << cameraUp.x << "," << cameraUp.y << "," << cameraUp.z << ")" << std::endl;
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

	pitch *= sensitivity;

	glm::mat3 pitchRotation = glm::mat3(glm::rotate(glm::radians(pitch), cameraRight));

	cameraFront = glm::normalize(pitchRotation * cameraFront);
	cameraUp = glm::normalize(pitchRotation * cameraUp);

	cameraRight = glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f));

	//std::cout << "cameraFront(" << cameraFront.x << "," << cameraFront.y << "," << cameraFront.z << ")" << std::endl;
	//std::cout << "cameraUp(" << cameraUp.x << "," << cameraUp.y << "," << cameraUp.z << ")" << std::endl;
}

void Camera::setYaw(float inYaw)
{
	yaw = inYaw;

	yaw *= sensitivity;

	glm::mat3 yawRotation = glm::mat3(glm::rotate(glm::radians(yaw), cameraUp));

	cameraFront = glm::normalize(yawRotation * cameraFront);
	cameraRight = glm::normalize(yawRotation * cameraRight);

	cameraUp = glm::cross(cameraRight, cameraFront);

	//std::cout << "cameraFront(" << cameraFront.x << "," << cameraFront.y << "," << cameraFront.z << ")" << std::endl;
	//std::cout << "cameraUp(" << cameraUp.x << "," << cameraUp.y << "," << cameraUp.z << ")" << std::endl;
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
