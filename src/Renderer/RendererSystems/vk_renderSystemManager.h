#pragma once
#include "VK_abstraction/vk_device.h"
#include "Renderer/RendererSystems/vk_basicRenderSystem.h"
#include "Renderer/RendererSystems/vk_glTFRenderSystem.h"
#include "Renderer/RendererSystems/vk_pointLightSystem.h"
#include "Renderer/RendererSystems/vk_skyboxRenderSystem.h"
#include "Game/vk_scene.h"
#include "Renderer/vk_descriptorManager.h"
#include "AppCore/vk_assetManager.h"

namespace vkc {

    class RenderSystemManager {
    public:
        // Call once, after swapchain + descriptor‐layout exist
        void initialize(VkcDevice& device,
            VkRenderPass renderPass,
            const DescriptorLayouts& layouts,
            DescriptorManager& descriptorManager,
            AssetManager& assetManager);

        // Register all systems into a scene
        void registerSystems(Scene& scene);

    private:
        std::vector<std::unique_ptr<VkcRenderSystem>> systems;
        DescriptorManager* _descriptorManager = nullptr;
    };

}