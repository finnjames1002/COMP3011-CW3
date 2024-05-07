#include "camera.h"

Camera::Camera() : radius(15.0f), position(0.0f), target(0.0f), up(0.0f, 1.0f, 0.0f), yOffset(1.f) {
    // Initialize your camera here
}

void Camera::processKeyboard(GLFWwindow* window) {
    float zoomSpeed = 0.005f; // The speed of zooming
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) // Zoom in
    {
        radius -= zoomSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // Zoom out
    {
        radius += zoomSpeed;
    }
}

void Camera::setRotationSpeed(float rSpeed) {
    rotateSpeed = rSpeed;
}

void Camera::updateCam(glm::vec3 t, glm::vec3 up) {
    target = t;
    this->up = up;
    position = glm::vec3(
        t.x + radius * cos(rotateSpeed),
        t.y + yOffset,
        t.z + radius * sin(rotateSpeed)
    );;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, target, up);
}

glm::vec3 Camera::getPosition() const {
    return position;
}

glm::vec3 Camera::getTarget() const {
	return target;
}

glm::vec3 Camera::getUp() const {
	return up;
}