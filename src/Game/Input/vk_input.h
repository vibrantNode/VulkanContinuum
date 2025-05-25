// vk_input.h (MNKController)

#pragma once
#include "Game/vk_gameObject.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace vkc {

    class MNKController {
    public:
        MNKController(float sensitivity = 0.1f, float yaw = -90.0f, float pitch = 0.0f);

        void handleMouseInput(GLFWwindow* window);
        void updateLook(float xOffset, float yOffset, VkcGameObject& gameObject);
        void updateMovement(GLFWwindow* window, float dt, VkcGameObject& gameObject);
        void resetLastMousePos(double x, double y);

        float getXOffset() const { return _xOffset; }
        float getYOffset() const { return _yOffset; }

    private:
        // Mouse state
        double _lastX = 0.0, _lastY = 0.0;
        bool _firstMouse = true;
        float _xOffset = 0.0f, _yOffset = 0.0f;

        // Movement/look settings
        float _sensitivity;
        float _yaw;
        float _pitch;
        float moveSpeed = 3.0f;
    };

} // namespace vkc