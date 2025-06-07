
// vk_input.cpp
#include "vk_input.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkc {

    MNKController::MNKController(float sensitivity, float yaw, float pitch, float moveSpeed)
        : _sensitivity(sensitivity)
        , _yaw(yaw)
        , _pitch(pitch)
        , moveSpeed(moveSpeed)
    {
    }
    void MNKController::mouseCallback(double xpos, double ypos) {
        if (_firstMouse) {
            _lastX = xpos;
            _lastY = ypos;
            _firstMouse = false;
        }
        _xOffset = static_cast<float>(xpos - _lastX);
        _yOffset = static_cast<float>(_lastY - ypos);
        _lastX = xpos;
        _lastY = ypos;
    }

    void MNKController::applyLook(VkcGameObject& gameObject) {
        float dx = _xOffset * _sensitivity;
        float dy = _yOffset * _sensitivity;

        _yaw += dx;
        _pitch += dy;
        _pitch = glm::clamp(_pitch, -89.0f, 89.0f);

        gameObject.transform.rotation.y = glm::radians(_yaw);
        gameObject.transform.rotation.x = glm::radians(_pitch);

        // Reset offsets
        _xOffset = 0.0f;
        _yOffset = 0.0f;
    }

    void MNKController::applyMovement(GLFWwindow* window, float dt, VkcGameObject& gameObject) {
        float yawRad = gameObject.transform.rotation.y;
        glm::vec3 forward{ sin(yawRad), 0.0f, cos(yawRad) };
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 dir(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dir += forward;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dir -= forward;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dir += right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir -= right;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) dir += up;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) dir -= up;

        if (glm::length(dir) > 0.0f) {
            gameObject.transform.translation += glm::normalize(dir) * (moveSpeed * dt);
        }
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

} // namespace vkc