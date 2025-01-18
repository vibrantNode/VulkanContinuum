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

namespace vkc {

    Application::Application() {
        globalPool =
            VkcDescriptorPool::Builder(_device)
            .setMaxSets(VkcSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkcSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
     
    }

    Application::~Application() {}

    void Application::RunApp() {



        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!_window.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
           
           
            if (auto commandBuffer = _renderer.beginFrame()) {


                //int frameIndex = _renderer.getFrameIndex();
         

                // render
                _renderer.beginSwapChainRenderPass(commandBuffer);
              
                _renderer.endSwapChainRenderPass(commandBuffer);
                _renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(_device.device());
    }

}


