#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera();
    void processKeyboard(GLFWwindow* window);
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    glm::vec3 getTarget() const;
	glm::vec3 getUp() const;
    void setRotationSpeed(float rSpeed);
    void updateCam(glm::vec3 t, glm::vec3 up);
private:
    float radius;
	float yOffset;
    float rotateSpeed;
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
};

#endif
