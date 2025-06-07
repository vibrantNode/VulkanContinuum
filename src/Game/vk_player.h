// vk_player.h

#pragma once
#include "Game/Camera/vk_camera.h"
#include "Game/Input/vk_input.h"
#include "Game/vk_gameObject.h"
#include <glm/glm.hpp>



namespace vkc {

    class Player {
    public:
        Player(GLFWwindow* window);

        void Init();

        void Update(float deltaTime);

        //void handleInput();

        const glm::mat4& getViewMatrix() const;
        const glm::mat4& getProjectionMatrix() const;
        VkcGameObject& getGameObject();
        VkcCamera& getCamera();
    private:
        GLFWwindow* _window;
        VkcCamera camera;
        VkcGameObject viewerObject;
        MNKController controller;

        float defaultFovY = 80.0f;
    };

}