#pragma once
#include "VK_abstraction/vk_device.h"
#include "Renderer/RendererSystems/vk_basicRenderSystem.h"
#include "Renderer/RendererSystems/vk_pointLightSystem.h"
#include "Game/vk_scene.h"

namespace vkc {

    class RenderSystemManager {
    public:
        // Call once, after swapchain + descriptor‐layout exist
        void initialize(VkcDevice& device,
            VkRenderPass renderPass,
            VkDescriptorSetLayout globalLayout);

        // Register all systems into a scene
        void registerSystems(Scene& scene);

    private:
        std::vector<std::unique_ptr<VkcRenderSystem>> systems;
    };

}