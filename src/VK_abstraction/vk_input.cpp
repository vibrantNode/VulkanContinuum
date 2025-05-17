#include "vk_input.h"
#include "vk_gameObject.h"
#include <limits>

namespace vkc {

    MNKController::MNKController(float sensitivity, float yaw, float pitch)
        : _sensitivity(sensitivity), _yaw(yaw), _pitch(pitch), _cameraPosition(glm::vec3(0.0f, 0.0f, 3.0f)) {
    }
    void MNKController::updateLook(float xOffset, float yOffset, VkcGameObject& gameObject) {

        if (!_hasMouseMoved) {
            return;
        }
        xOffset *= _sensitivity;
        yOffset *= _sensitivity;

        _yaw += xOffset;
        _pitch += yOffset;

        // Clamp pitch to avoid flipping
        _pitch = glm::clamp(_pitch, -89.0f, 89.0f);

        // Optional: wrap yaw
        if (_yaw > 360.0f) _yaw -= 360.0f;
        if (_yaw < 0.0f) _yaw += 360.0f;

        gameObject.transform.rotation.x = glm::radians(_pitch);
        gameObject.transform.rotation.y = glm::radians(_yaw);
    }

    void MNKController::updateMovement(GLFWwindow* window, float dt, VkcGameObject& gameObject) {
        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
        const glm::vec3 upDir{ 0.f, -1.f, 0.f };

        glm::vec3 moveDir{ 0.f };
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
        if (glfwGetKey(window, keys.Exit) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    
    }



    void MNKController::processMouseMovement(float xOffset, float yOffset)
    {
        // Apply sensitivity factor
        xOffset *= _sensitivity;
        yOffset *= _sensitivity;

        // Update yaw and pitch based on offsets
        _yaw += xOffset;
        _pitch += yOffset;

        // Constrain the pitch if necessary
        if (_pitch > 89.0f)
            _pitch = 89.0f;
        if (_pitch < -89.0f)
            _pitch = -89.0f;

        // Update camera direction based on yaw and pitch
        glm::vec3 direction;
        direction.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        direction.y = sin(glm::radians(_pitch));
        direction.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _cameraDirection = glm::normalize(direction);
    }
    void MNKController::processKeyboardInput(GLFWwindow* window, float deltaTime) {
        float cameraSpeed = 2.5f * deltaTime; // adjust speed as necessary

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            _cameraPosition += _cameraDirection * cameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            _cameraPosition -= _cameraDirection * cameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            _cameraPosition -= glm::normalize(glm::cross(_cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f))) * cameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            _cameraPosition += glm::normalize(glm::cross(_cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f))) * cameraSpeed;
        }
    }
    glm::vec3 MNKController::getCameraDirection() const { return _cameraDirection; }

    glm::vec3 MNKController::getCameraPosition() const { return _cameraPosition; }


    void MNKController::handleMouseInput(GLFWwindow* window)
    {
        if (_xOffset != 0.0f || _yOffset != 0.0f) {
            _hasMouseMoved = true;
        }
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (_firstMouse) {
            _lastX = xpos;
            _lastY = ypos;
            _firstMouse = false;
        }

        _xOffset = static_cast<float>(xpos - _lastX);
        _yOffset = static_cast<float>(_lastY - ypos);  // reversed Y

        _lastX = xpos;
        _lastY = ypos;
    }
} // namespace vkc