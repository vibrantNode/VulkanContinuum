// vk_core.cpp

// std
#include <iostream>
// std
#include <iostream>
#include <chrono>
#include <array>
#include <cassert>
#include <stdexcept>


// Vulkan
#include <vulkan/vulkan.h>


// Third party
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#define GLM_FORCE_RADIANS	
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Project source specific includes
#include "vk_core.h"
#include "VkBootstrap.h"
#include "Renderer/vk_buffer.h"
#include "VK_abstraction/vk_camera.h"
#include "VK_abstraction/vk_input.h"
#include "Renderer/Types/vk_basicRenderSystem.h"
#include "Renderer/Types/vk_pointLightSystem.h"



namespace vkc {
    Application::Application() {
        globalPool =
            VkcDescriptorPool::Builder(_device)
            .setMaxSets(VkcSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkcSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        loadGameObjects();
    }
    Application::~Application() {
    }
    void Application::RunApp() {
        std::vector<std::unique_ptr<VkcBuffer>> uboBuffers(VkcSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<VkcBuffer>(
                _device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            uboBuffers[i]->map();
        }
       auto globalSetLayout = VkcDescriptorSetLayout::Builder(_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(VkcSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VkcDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem{
            _device,
            _renderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout() };

        PointLightSystem pointLightSystem{
            _device,
            _renderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        VkcCamera camera(glm::vec3(0.0f, 2.0f, -15.0f), -90.0f, 0.0f);

        auto viewerObject = VkcGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        MNKController cameraController{};
        glfwSetInputMode(_window.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetWindowUserPointer(_window.getGLFWwindow(), &cameraController);

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!_window.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.handleMouseInput(_window.getGLFWwindow());
            cameraController.updateLook(cameraController.getXOffset(), cameraController.getYOffset(), viewerObject);
            cameraController.updateMovement(_window.getGLFWwindow(), frameTime, viewerObject);

            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = _renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
           
            if (auto commandBuffer = _renderer.beginFrame()) {
                int frameIndex = _renderer.getFrameIndex();

                FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects };

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                _renderer.beginSwapChainRenderPass(commandBuffer);

                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);

                _renderer.endSwapChainRenderPass(commandBuffer);
                _renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(_device.device());
    }
    void Application::loadGameObjects()
    {

        std::shared_ptr<VkcModel> vkcModel = VkcModel::createModelFromFile(_device, PROJECT_ROOT_DIR "/res/models/flat_vase.obj");
        auto flatVase = VkcGameObject::createGameObject();

        flatVase.model = vkcModel;
        flatVase.transform.translation = { -.5f, .5f, 0.f };
        flatVase.transform.scale = { 3.f, 1.5f, 3.f };
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        vkcModel = VkcModel::createModelFromFile(_device, PROJECT_ROOT_DIR "/res/models/quad.obj");
        auto floor = VkcGameObject::createGameObject();
        floor.model = vkcModel;
        floor.transform.translation = { 0.f, .5f, 0.f };
        floor.transform.scale = { 3.f, 1.f, 3.f };
        gameObjects.emplace(floor.getId(), std::move(floor));

        vkcModel = VkcModel::createModelFromFile(_device, PROJECT_ROOT_DIR "/res/models/smooth_vase.obj");
        auto smoothVase = VkcGameObject::createGameObject();
        smoothVase.model = vkcModel;
        smoothVase.transform.translation = { .5f, .5f, 0.f };
        smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        std::vector<glm::vec3> lightColors{
              {1.f, .1f, .1f},
              {.1f, .1f, 1.f},
              {.1f, 1.f, .1f},
              {1.f, 1.f, .1f},
              {.1f, 1.f, 1.f},
              {1.f, 1.f, 1.f}  //
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = VkcGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.f, -1.f, 0.f });
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }

    }
}


