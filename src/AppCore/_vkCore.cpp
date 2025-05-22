// vk_core.cpp

// Project source specific includes
#include "_vkCore.h"
#include "VkBootstrap.h"
#include "VK_abstraction/vk_buffer.h"
#include "VK_abstraction/vk_camera.h"
#include "VK_abstraction/vk_input.h"
#include "Renderer/RendererSystems/vk_basicRenderSystem.h"
#include "Renderer/RendererSystems/vk_pointLightSystem.h"

// Vulkan
#include <vulkan/vulkan.h>

// External
#define GLM_FORCE_RADIANS	
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// STD
#include <iostream>
#include <chrono>
#include <array>
#include <cassert>
#include <stdexcept>

namespace vkc {
    Application::Application() 
    {
        globalPool =
            VkcDescriptorPool::Builder(_device)
            .setMaxSets(VkcSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkcSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
            .build();
        _assetManager.preloadGlobalAssets();
        _scene.loadSceneData("DefaultScene");
    }
    Application::~Application() 
    {
    }
    void Application::RunApp() 
    {
       

        auto allTextures = _assetManager.getAllTextures();
        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(allTextures.size());

        std::vector<std::unique_ptr<VkcBuffer>> uboBuffers(VkcSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<VkcBuffer>
                (
                    _device,
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                );

            uboBuffers[i]->map();
        }

        for (auto& tex : allTextures) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = tex->GetImageView();
            imageInfo.sampler = tex->GetSampler();
            imageInfos.push_back(imageInfo);
        }

       auto globalSetLayout = VkcDescriptorSetLayout::Builder(_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
           .addBinding(
               2,
               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
               VK_SHADER_STAGE_FRAGMENT_BIT,
               static_cast<uint32_t>(imageInfos.size()),
               VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
           )

            .build();

     
  


        std::vector<VkDescriptorSet> globalDescriptorSets(VkcSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) 
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VkcDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfos[0], 1)
                .writeImage(2, imageInfos.data(), static_cast<uint32_t>(imageInfos.size()))
                .build(globalDescriptorSets[i]);
        }

        _scene.addRenderSystem(std::make_unique<SimpleRenderSystem>(_device, _renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()));
        _scene.addRenderSystem(std::make_unique<PointLightSystem>(_device, _renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()));

        // Camera initialization logic
        glm::vec3 cameraPos = glm::vec3(0.0f, .0f, -5.0f);
        glm::vec3 target = glm::vec3(0.0f); // Look at origin
        glm::vec3 direction = glm::normalize(target - cameraPos);

        float initialYaw = glm::degrees(atan2(direction.x, direction.z));
        float initialPitch = glm::degrees(asin(-direction.y));

        // Adjust the yaw and pitch
        initialYaw += .5f;
        initialPitch += .0f;
        
        // fov and movement speed
        float fov = 80.f;
        float movementSpeed = 13.f;


        VkcCamera camera(
            cameraPos,
            initialYaw,
            initialPitch,
            fov,
            movementSpeed   
        );

        auto viewerObject = VkcGameObject::createGameObject();
        viewerObject.transform.translation = cameraPos;
        viewerObject.transform.rotation = glm::vec3(
            glm::radians(initialPitch), // pitch
            glm::radians(initialYaw),   // yaw
            0.0f
        );


        MNKController cameraController(0.1f, initialYaw, initialPitch);

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
            camera.setPerspectiveProjection(aspect, 0.1f, 100.f);
           
            if (auto commandBuffer = _renderer.beginFrame()) {
                int frameIndex = _renderer.getFrameIndex();

                FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], _scene.getGameObjects()};

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
          
                _scene.update(frameInfo, ubo, frameTime);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                _renderer.beginSwapChainRenderPass(commandBuffer);

                _scene.render(frameInfo);

                _renderer.endSwapChainRenderPass(commandBuffer);
                _renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(_device.device());
    }
   
}// namespace vkc


