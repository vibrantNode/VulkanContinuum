#pragma once
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_pipeline.h"
#include "VK_abstraction/vk_frameInfo.h"
#include "Renderer/RendererSystems/vk_renderSystem.h"
#include <memory>

namespace vkc
{
    class ToneMapSystem : public VkcRenderSystem{
    public:
        ToneMapSystem(VkcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
        ~ToneMapSystem();

        void render(FrameInfo& frameInfo) override;

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        VkcDevice& vkcDevice;
        VkPipelineLayout pipelineLayout;
        std::unique_ptr<VkcPipeline> toneMapPipeline;

		VkDescriptorSetLayout descriptorSetLayout;  
    };
}