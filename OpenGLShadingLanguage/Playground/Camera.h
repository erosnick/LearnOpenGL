#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(const glm::vec3& inEye, const glm::vec3& inCenter)
    : eye(inEye), center(inCenter) {
        forward = glm::normalize(center - eye);
        right = glm::cross(forward, up);
        up = glm::cross(right, forward);

        isDirty = true;

        updateViewMatrix();
    }

    void walk(float delta) {
        eye += forward * cameraSpeed * delta;
        center += forward * cameraSpeed * delta;
        isDirty = true;
    }

    void strafe(float delta) {
        eye += right * cameraSpeed * delta;
        center += right * cameraSpeed * delta;
        isDirty = true;
    }

    void raise(float delta) {
        eye += up * cameraSpeed * delta;
        center += up * cameraSpeed * delta;
        isDirty = true;
    }

    void yaw(float delta) {
        // Should rotate around up vector
        glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(delta), up);
        forward = glm::normalize(rotation * forward);
        right = glm::normalize(rotation * right);

        //up = glm::cross(right, forward);

        center = eye + forward;

        isDirty = true;
    }

    void pitch(float delta) {
        // Should rotate around right vector
        glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(delta), right);
        forward = glm::normalize(rotation * forward);
        //up = glm::normalize(rotation * up);

        center = eye + forward;

        isDirty = true;
    }

    void updateViewMatrix() {
        if (isDirty) {
            viewMatrix = glm::lookAt(eye, center, up);
            right = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
            up = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };
            forward = { viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2] };
            forward = -forward;
            isDirty = false;
        }
    }

    void perspective(float inFOV, float inAspect, float inNear, float inFar) {
        fov = inFOV;
        aspect = inAspect;
        near = inNear;
        far = inFar;
        projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
    }

    void orthographic(float left, float right, float bottom, float top) {
        projectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    }

    const glm::mat4& getViewMatrix() const {
        return viewMatrix;
    }

    const glm::mat4& getProjectionMatrix() const {
        return projectionMatrix;
    }

    const glm::vec3 getRight() const {
        return right;
    }

    const glm::vec3 getUp() const {
        return up;
    }

    const glm::vec3 getForward() const {
        return forward;
    }

    const glm::vec3 getEye() const {
        return eye;
    }
private:
    float fov = 60.0f;
    float near = 0.1f;
    float far = 100.0f;
    float aspect = 16.0f / 9.0f;
    glm::vec3 eye = { 0.0f, 1.0f, 4.5f };
    glm::vec3 center = { 0.0f, 1.0f, 0.0f };
    float cameraSpeed = 6.0f;
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 forward;
    glm::vec3 right;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    bool isDirty = false;
};

