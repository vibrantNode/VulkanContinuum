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
        _assetManager.preloadGlobalAssets();

        DescriptorConfig config{
            VkcSwapChain::MAX_FRAMES_IN_FLIGHT,
            sizeof(GlobalUbo),
            &_assetManager
        };
        _descriptorManager.Initialize(config);
      
        _game.Init(_window.getGLFWwindow());
    }
    Application::~Application() 
    {
    }
    void Application::RunApp()
    {
        _descriptorManager.createDescriptorSets();
     
        _renderSystemManager.initialize(
            _device,
            _renderer.getSwapChainRenderPass(),
            _descriptorManager.getAllLayouts(),
            _descriptorManager,
            _assetManager

        );

        _renderSystemManager.registerSystems(_game.getScene());


        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!_window.shouldClose()) 
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
           
            if (auto commandBuffer = _renderer.beginFrame()) 
            {
                int frameIndex = _renderer.getFrameIndex();

                FrameInfo frameInfo{
                    frameIndex, frameTime, commandBuffer,
                    _game.getPlayerCamera(),
                    _descriptorManager.getGlobalDescriptorSets()[frameIndex],
                    _descriptorManager.getTextureDescriptorSet(),
                    _descriptorManager.getSkyboxDescriptorSet(),
                    _game.getGameObjects()
                };

                // update
                GlobalUbo ubo{};
                auto& uboBuffer = _descriptorManager.getUboBuffers()[frameIndex];
                _game.Update(frameInfo, ubo, frameTime);
                uboBuffer->writeToBuffer(&ubo);
                uboBuffer->flush();

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


