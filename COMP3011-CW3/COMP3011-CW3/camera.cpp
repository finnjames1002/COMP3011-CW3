#include "camera.h"

Camera::Camera() : yaw(0.0f), pitch(0.0f), direction(0.0f, 0.0f, -1.0f), radius(15.0f), position(0.0f), target(0.0f), up(0.0f, 1.0f, 0.0f), yOffset(1.f){
    cameraType = CAMERA_ROTATE;
}



void Camera::processKeyboard(GLFWwindow* window) {
	
    float zoomSpeed = 0.05f; // The speed of zooming
	float cameraSpeed; // The speed of moving the camera
    glm::vec3 forward = direction;
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) // Speed camera up
    {
        if (cameraType == CAMERA_FREE) {
            cameraSpeed = 0.2f;
        }
	}
	else {
		cameraSpeed = 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // Zoom in / move forward
    {
        if (cameraType == CAMERA_ROTATE || cameraType == CAMERA_STATIC) {
            radius -= zoomSpeed;
        }
        else {
            position += cameraSpeed * forward;
        }
    }
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // Zoom out / move backward
    {
        if (cameraType == CAMERA_ROTATE || cameraType == CAMERA_STATIC) {
            radius += zoomSpeed;
        }
        else {
            position -= cameraSpeed * forward;
        }
    }
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // Move left / rotate left faster
    {
        if (cameraType == CAMERA_FREE) {
            glm::vec3 right = glm::normalize(glm::cross(up, target - position));
            position += cameraSpeed * right;
        }
        else {
			rotateSpeed += 0.00003;
        }
		
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // Move right / rotate right faster
	{
        if (cameraType == CAMERA_FREE) {
            glm::vec3 right = glm::normalize(glm::cross(up, target - position));
            position -= cameraSpeed * right;
        }
        else {
            rotateSpeed -= 0.00003;
        }
	}
    
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) // Camera 1
    {
		cameraType = CAMERA_ROTATE;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) // Camera 2
	{
        cameraType = CAMERA_STATIC;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) // Camera 3
	{
		cameraType = CAMERA_FREE;
        direction = glm::normalize(target - position);

        // Calculate yaw and pitch based on the current direction vector
        yaw = glm::degrees(atan2(direction.z, direction.x));
        pitch = glm::degrees(asin(direction.y));

		// Had to do this weird setup i'm not sure why
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        lastX = static_cast<float>(mouseX);
        lastY = static_cast<float>(mouseY);
	}
    
}

void Camera::processMouseMovement(GLFWwindow* window, float xPos, float yPos) {
    if (cameraType != CAMERA_FREE) {
        return;
    }

	float sensitivity = 0.1f; // Sensitivity of camera movement responding to mouse input

    float xOffset = xPos - lastX;
    float yOffset = yPos - lastY;

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch -= yOffset;

    // Make sure that when pitch is out of bounds, the screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Update direction, right and up vectors
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction = glm::normalize(direction);

    // Update lastX and lastY to the current mouse position
    lastX = xPos;
    lastY = yPos;
}

void Camera::updateCam(glm::vec3 t, glm::vec3 up) {
    if (cameraType == CAMERA_FREE) {
        target = position + direction;
    }
    else {
		if (cameraType == CAMERA_ROTATE) { // Rotate the camera around the target
            rotatePos += rotateSpeed;
        }
		// If not, the camera is static
        target = t;
        this->up = up;
        position = glm::vec3(
            t.x + radius * cos(rotatePos), 
            t.y + yOffset,
            t.z + radius * sin(rotatePos)
        );
    }
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