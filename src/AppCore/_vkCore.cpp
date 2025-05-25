// vk_core.cpp

// Project source specific includes
#include "_vkCore.h"
#include "VkBootstrap.h"
#include "VK_abstraction/vk_buffer.h"
#include "Game/Camera/vk_camera.h"
#include "Game/Input/vk_input.h"
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

        // Does this need to be abstracted? if so where?
        globalPool =
            VkcDescriptorPool::Builder(_device)
            .setMaxSets(VkcSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkcSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
            .build();

        // ^*
        _assetManager.preloadGlobalAssets();

     
        _game.Init(_window.getGLFWwindow());
    }
    Application::~Application() 
    {
    }
    void Application::RunApp()
    {
        // Does this need to be abstracted? if so where?
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
        // ^*

        // The things below need to be abstracted
        auto allTextures = _assetManager.getAllTextures();
        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(allTextures.size());

        for (auto& tex : allTextures) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = tex->GetImageView();
            imageInfo.sampler = tex->GetSampler();
            imageInfos.push_back(imageInfo);
        }

        auto globalSetLayout = VkcDescriptorSetLayout::Builder(_device)
            .addBinding(
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS
            )
            .addBinding(    
                1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                static_cast<uint32_t>(imageInfos.size()),
                VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
           )
            .build();
      
        std::vector<VkDescriptorSet> globalDescriptorSets(VkcSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) 
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VkcDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, imageInfos.data(), static_cast<uint32_t>(imageInfos.size()))
                .build(globalDescriptorSets[i]);
        }
        // End of things that need to be abstracted*^

        _rsManager.initialize(
            _device,
            _renderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        );

        // Push systems into the games scene
        _rsManager.registerSystems(_game.getScene());


        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!_window.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
           
            if (auto commandBuffer = _renderer.beginFrame()) {
                int frameIndex = _renderer.getFrameIndex();

                FrameInfo frameInfo{
                    frameIndex, frameTime, commandBuffer,
                    _game.getPlayerCamera(), globalDescriptorSets[frameIndex],
                    _game.getGameObjects()
                };

                // update
                GlobalUbo ubo{};

                _game.Update(frameInfo, ubo, frameTime);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                _renderer.beginSwapChainRenderPass(commandBuffer);

                _game.Render(frameInfo);

                _renderer.endSwapChainRenderPass(commandBuffer);
                _renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(_device.device());
    }
   
}// namespace vkc


