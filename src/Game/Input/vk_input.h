#pragma once
#include "Game/vk_gameObject.h"
#include <GLFW/glfw3.h>

namespace vkc {

    class MNKController {
    public:
        MNKController(float sensitivity = 0.1f, float yaw = -90.0f, float pitch = 0.0f, float moveSpeed = 10.f);

        // GLFW cursor callback for raw mouse movement
        void mouseCallback(double xpos, double ypos);

        // Apply look (called once per frame)
        void applyLook(VkcGameObject& gameObject);

        // Apply keyboard movement (called once per frame)
        void applyMovement(GLFWwindow* window, float deltaTime, VkcGameObject& gameObject);

    private:
        // Mouse state
        double _lastX = 0.0, _lastY = 0.0;
        bool _firstMouse = true;
        float _xOffset = 0.0f, _yOffset = 0.0f;

        // Movement/look settings
        float _sensitivity;
        float _yaw;
        float _pitch;
        float moveSpeed;
    };

} // namespace vkc
