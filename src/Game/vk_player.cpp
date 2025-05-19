// vk_player.cpp

#include "vk_player.h"

namespace vkc {

    class PlayerInputComponent : public InputComponent {
    public:
        PlayerInputComponent(GLFWwindow* window) : _window(window) {}

        void ProcessInput(GLFWwindow* window, float deltaTime) override {
            // Put your input logic here (WASD, mouse, etc.)
            // Example: capture key presses and update player's transform or send events.
        }
    private:
        GLFWwindow* _window;
    };

    class PlayerCameraComponent : public CameraComponent {
    public:
        PlayerCameraComponent(glm::vec3 pos, float yaw, float pitch, float fov, float movementSpeed)
            : camera(pos, yaw, pitch, fov, movementSpeed) {
        }

        void UpdateCamera(float deltaTime) override {
            // update the camera's view matrix or any internal states
        }

        glm::mat4 GetViewMatrix() const override {
            return camera.getView();
        }

        glm::mat4 GetProjectionMatrix() const override {
            return camera.getProjection();
        }

        VkcCamera camera;
    };

    Player::Player(GLFWwindow* window)
        : _window(window)
    {
        input = std::make_shared<PlayerInputComponent>(window);

        // Initialize camera at some default position and orientation
        camera = std::make_shared<PlayerCameraComponent>(
            glm::vec3(0.f, 0.f, -5.f),   // position
            0.f,                        // yaw
            0.f,                        // pitch
            100.f,                      // fov
            13.f                        // movement speed
        );
    }

    void Player::Update(float deltaTime) {
        input->ProcessInput(_window, deltaTime);
        camera->UpdateCamera(deltaTime);

        // Sync player transform with camera transform if needed
        //this->transform.translation = camera->camera.getPosition();
        //this->transform.rotation = camera->camera.getRotation();
       
    }

}
