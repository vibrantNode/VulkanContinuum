// vk_player.cpp
#include "vk_player.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkc {

    Player::Player(GLFWwindow* window)
        : _window(window)
    {
    }

    void Player::Init() {
        // Setup viewer object
        glm::vec3 startPos(0.0f, 0.0f, -5.0f);
        viewerObject = VkcGameObject::createGameObject();
        viewerObject.transform.translation = startPos;
        viewerObject.transform.rotation = glm::vec3(0.0f);

        // Compute initial yaw/pitch
        glm::vec3 target(0.0f);
        glm::vec3 dir = glm::normalize(target - startPos);
        float yaw = glm::degrees(atan2(dir.x, dir.z));
        float pitch = glm::degrees(asin(-dir.y));

        // Init camera & controller
        camera = VkcCamera(startPos, yaw, pitch, defaultFovY);
        controller = MNKController(0.1f, yaw, pitch, 9.0f);

        // Lock/hide cursor and set callback
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetWindowUserPointer(_window, &controller);
        glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double x, double y) {
            static_cast<MNKController*>(glfwGetWindowUserPointer(window))->mouseCallback(x, y);
            });
    }

    void Player::Update(float deltaTime) {
        // 1) Apply look from raw mouse data

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
        camera.setPerspectiveProjection(w / static_cast<float>(h), 0.1f, 300.f);
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