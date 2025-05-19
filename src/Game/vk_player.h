// vk_player.h

#pragma once
#include "vk_component.h"
#include "VK_abstraction/vk_camera.h"
#include "VK_abstraction/vk_input.h"

namespace vkc {

    class Player : public VkcGameObject {
    public:
        Player(GLFWwindow* window);

        void Update(float deltaTime);

        std::shared_ptr<InputComponent> input;
        std::shared_ptr<CameraComponent> camera;

    private:
        GLFWwindow* _window;
    };

}