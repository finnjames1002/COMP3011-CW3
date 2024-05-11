#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraType {
    CAMERA_ROTATE,
    CAMERA_STATIC,
    CAMERA_FREE
};

class Camera {
public:
    Camera();
    void processKeyboard(GLFWwindow* window);
    void processMouseMovement(GLFWwindow* window, float xOffset, float yOffset);
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    glm::vec3 getTarget() const;
	glm::vec3 getUp() const;
	enum CameraType cameraType;
    void updateCam(glm::vec3 t, glm::vec3 up);
private:
    float radius;
	float yOffset;
    float rotateSpeed;
    float yaw, pitch;
    glm::vec3 direction;
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
	double lastX, lastY = 0.f;
	double initialYaw, initialPitch = 0.f;
};

#endif
