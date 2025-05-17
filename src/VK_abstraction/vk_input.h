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

        void updateMovement(GLFWwindow* window, float dt, VkcGameObject& gameObject);
        void updateLook(float xOffset, float yOffset, VkcGameObject& gameObject);
        void processMouseMovement(float xOffset, float yOffset);
        void processKeyboardInput(GLFWwindow* window, float deltaTime);

        glm::vec3 getCameraDirection() const;
        glm::vec3 getCameraPosition() const;

    public:
       
        void handleMouseInput(GLFWwindow* window);

        float getXOffset() const { return _xOffset; }
        float getYOffset() const { return _yOffset; }

    private:
        double _lastX = 0.0, _lastY = 0.0;
        bool _firstMouse = true;
        float _xOffset = 0.f, _yOffset = 0.f;



        KeyMappings keys{};
        float moveSpeed{ 3.f };
        float lookSpeed{ 1.5f };


    private:
        double lastX = 400.0;
        double lastY = 300.0;
        bool firstMouse = true;

        float _sensitivity;  
        float _yaw;         
        float _pitch;       
        glm::vec3 _cameraDirection; 
        glm::vec3 _cameraPosition;
    };
}  // namespace vkc