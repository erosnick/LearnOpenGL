#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Camera
{
public:

	Camera();

	void setFOV(float inFov);

	void setAspectRatio(float inAspectRatio);

	void setPosition(const glm::vec3& position);

	void lookAt(const glm::vec3& center);

	void setFront(const glm::vec3 front);

	void setPitch(float inPitch);

	void setYaw(float inYaw);

	void forward(float delta);

	void right(float delta);

	void up(float delta);

	glm::mat4& viewMatrix();
	glm::mat4& projectionMatrix();

	float pitch;
	float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.

private:

	glm::vec3 calculateRotation(float pitch, float yaw);

	float aspectRatio;
	float fov = 45.0f;
	float nearZ = 0.1f;
	float farZ = 100.0f;

	glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float cameraSpeed = 1.5f;
	float sensitivity = 0.01f;

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(0.0f);

	float lastFrame;
	float deltaTime;
};