// vk_core.cpp
#include "_vkCore.h"

// STD
#include <chrono>

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
            //glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
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
                    _game.getGameObjects(),
                    &_game.getScene()
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