// vk_component.h

#pragma once
#include <memory>

namespace vkc {

    struct Transform {
        glm::vec3 translation{};
        glm::vec3 rotation{};
        glm::vec3 scale{ 1.f, 1.f, 1.f };
    };

    class Component {
    public:
        virtual ~Component() = default;
        virtual void Update(float deltaTime) {}
    };

    class InputComponent : public Component {
    public:
        virtual void ProcessInput(GLFWwindow* window, float deltaTime) = 0;
    };

    class CameraComponent : public Component {
    public:
        virtual void UpdateCamera(float deltaTime) = 0;
        virtual glm::mat4 GetViewMatrix() const = 0;
        virtual glm::mat4 GetProjectionMatrix() const = 0;
    };

}