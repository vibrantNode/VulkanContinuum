// vk_core.cpp
#include "_vkCore.h"

// STD
#include <chrono>

namespace vkc {
    Application::Application()
    {
        _assetManager.preloadGlobalAssets();
        _game.Init(_window.getGLFWwindow());

        DescriptorConfig config{
            VkcSwapChain::MAX_FRAMES_IN_FLIGHT,
            sizeof(GlobalUbo),
            &_assetManager,
			&_game.getScene()
        };

        _descriptorManager.Initialize(config);
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
        int  frameCount = 0;
        float fpsTimer = 0.0f;

        while (!_window.shouldClose()) 
        {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameCount++;
            fpsTimer += frameTime;
            if (fpsTimer >= 1.0f) {
                //std::cout << "FPS: " << frameCount << "\n";
                frameCount = 0;
                fpsTimer -= 1.0f;
            }
           
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
                _game.Update(frameInfo, ubo, frameTime);
                auto& uboBuffer = _descriptorManager.getUboBuffers()[frameIndex];
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