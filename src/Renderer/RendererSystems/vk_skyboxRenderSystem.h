#pragma once

// Project headers
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_pipeline.h"
#include "Game/vk_gameObject.h"
#include "VK_abstraction/vk_frameInfo.h"
#include "VK_abstraction/vk_obj_model.h"
#include "Game/Camera/vk_camera.h"
#include "Renderer/RendererSystems/vk_renderSystem.h"
#include "Game/vk_scene.h"

// STD
#include <memory>
#include <vector>



namespace vkc {

    class SkyboxRenderSystem : public VkcRenderSystem {
    public:
        SkyboxRenderSystem(VkcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout skyboxLayout);
        ~SkyboxRenderSystem();

        SkyboxRenderSystem(const SkyboxRenderSystem&) = delete;
        SkyboxRenderSystem& operator=(const SkyboxRenderSystem&) = delete;

        // Call this inside your scene render loop, after global descriptors are bound
        void render(FrameInfo& frameInfo) override;

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout skyboxSetLayout);
        void createPipeline(VkRenderPass renderPass);

        VkDescriptorSetLayout skyboxLayout;
        VkcDevice& vkcDevice;
        std::unique_ptr<VkcPipeline> vkcPipeline;
        VkPipelineLayout pipelineLayout;
    };

} // namespace vkc
