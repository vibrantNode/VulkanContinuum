#pragma once 
// MNK_Controller.h
#include "VK_Abstraction/vk_gameObject.h"
#include "AppCore/vk_window.h"

namespace vkc {
    class MNKController {
    public:

        MNKController(float sensitivity = 0.1f, float yaw = -90.0f, float pitch = 0.0f);

        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
            int Exit = GLFW_KEY_ESCAPE;
        };

        void moveInPlaneXZ(GLFWwindow* window, float dt, VkcGameObject& gameObject);

        void processMouseMovement(float xOffset, float yOffset);
        void processKeyboardInput(GLFWwindow* window, float deltaTime);

        glm::vec3 getCameraDirection() const;
        glm::vec3 getCameraPosition() const;

        KeyMappings keys{};
        float moveSpeed{ 3.f };
        float lookSpeed{ 1.5f };


    private:
        double lastX = 400.0;
        double lastY = 300.0;
        bool firstMouse = true;

        float _sensitivity;  // Sensitivity factor for mouse movement
        float _yaw;          // Camera yaw (horizontal rotation)
        float _pitch;        // Camera pitch (vertical rotation)
        glm::vec3 _cameraDirection; // Camera direction vector
        glm::vec3 _cameraPosition;
    };
}  // namespace vkc