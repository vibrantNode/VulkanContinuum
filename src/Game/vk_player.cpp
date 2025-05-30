// vk_player.cpp
#include "vk_player.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkc {

    Player::Player(GLFWwindow* window)
        : _window(window)
        , controller(0.1f, 0.0f, 0.0f)
        , camera({ 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 80.f, 10.f)
    {
    }

    void Player::Init() {
        // 1) Setup viewer object
        glm::vec3 startPos(0.0f, 0.0f, -5.0f);
        viewerObject = VkcGameObject::createGameObject();
        viewerObject.transform.translation = startPos;
        viewerObject.transform.rotation = glm::vec3(0.0f);

        // 2) Compute initial yaw/pitch
        glm::vec3 target(0.0f);
        glm::vec3 dir = glm::normalize(target - startPos);
        float yaw = glm::degrees(atan2(dir.x, dir.z));
        float pitch = glm::degrees(asin(-dir.y));

        // 3) Init camera & controller
        camera = VkcCamera(startPos, yaw, pitch, 80.f, 13.f);
        controller = MNKController(0.1f, yaw, pitch);

        // 4) Lock/hide cursor and set callback
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetWindowUserPointer(_window, &controller);
        glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double x, double y) {
            static_cast<MNKController*>(glfwGetWindowUserPointer(window))->mouseCallback(x, y);
            });
    }

    void Player::Update(float deltaTime) {
        // 1) Pump OS events
        glfwPollEvents();

        // 2) Apply look from raw mouse data
        controller.applyLook(viewerObject);

        // 3) Apply movement with frame-time
        controller.applyMovement(_window, deltaTime, viewerObject);

        // 4) Sync camera to the object
        camera.setViewYXZ(
            viewerObject.transform.translation,
            viewerObject.transform.rotation
        );

        // 5) Update projection on resize
        int w, h;
        glfwGetFramebufferSize(_window, &w, &h);
        camera.setPerspectiveProjection(w / static_cast<float>(h), 0.1f, 100.f);
    }

    const glm::mat4& Player::getViewMatrix() const {
        return camera.getView();
    }

    const glm::mat4& Player::getProjectionMatrix() const {
        return camera.getProjection();
    }

    VkcGameObject& Player::getGameObject() {
        return viewerObject;
    }

    VkcCamera& Player::getCamera() {
        return camera;
    }

} // namespace vkc