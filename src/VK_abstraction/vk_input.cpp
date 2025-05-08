#include "vk_input.h"
#include "vk_gameObject.h"
#include <limits>

namespace vkc {

    MNKController::MNKController(float sensitivity, float yaw, float pitch)
        : _sensitivity(sensitivity), _yaw(yaw), _pitch(pitch), _cameraDirection(glm::vec3(0.0f, 0.0f, -1.0f)), _cameraPosition(glm::vec3(0.0f, 0.0f, 3.0f)) {
    }
    void MNKController::moveInPlaneXZ(
        GLFWwindow* window, float dt, VkcGameObject& gameObject) {
        glm::vec3 rotate{ 0 };
        if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
        if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
        if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
        if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

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
} // namespace vkc