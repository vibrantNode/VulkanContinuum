// vk_player.cpp

#include "vk_player.h"

namespace vkc {

   
    Player::Player(GLFWwindow* window)
        : _window(window),
        controller(0.1f, 0.0f, 0.0f),
        camera({ 0.0f,0.0f,0.0f }, 0.0f, 0.0f, 80.f, 10.f)
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

        // 4) Lock/hide cursor
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetWindowUserPointer(_window, &controller);

        // 5) Seed the controller's last mouse position to avoid jump
        int w, h;
        glfwGetFramebufferSize(_window, &w, &h);
        double cx = w * 0.5, cy = h * 0.5;
        glfwSetCursorPos(_window, cx, cy);
        controller.resetLastMousePos(cx, cy);
    }

    void Player::Update(float deltaTime) {
        // a) Mouse look
        controller.handleMouseInput(_window);
        controller.updateLook(controller.getXOffset(),
            controller.getYOffset(),
            viewerObject);

        // b) Recenter cursor & sync controller
        int w, h;
        glfwGetFramebufferSize(_window, &w, &h);
        double cx = w * 0.5, cy = h * 0.5;
        glfwSetCursorPos(_window, cx, cy);
        controller.resetLastMousePos(cx, cy);

        // c) Keyboard movement
        controller.updateMovement(_window, deltaTime, viewerObject);

        // d) Sync camera to the object
        camera.setViewYXZ(viewerObject.transform.translation,
            viewerObject.transform.rotation);

        // e) Update projection for correct aspect
        glfwGetFramebufferSize(_window, &w, &h);
        float aspect = w / float(h);
        camera.setPerspectiveProjection(aspect, 0.1f, 100.f);
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

    VkcCamera& Player::GetCamera() {
        return camera;
    }

}
