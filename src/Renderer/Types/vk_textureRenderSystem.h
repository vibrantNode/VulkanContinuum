#pragma once
#include <vulkan/vulkan.h>
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_frameInfo.h"
#include "VK_abstraction/vk_pipeline.h"

namespace vkc {
    class TextureRenderSystem {
    public:
        TextureRenderSystem(VkcDevice& device, VkRenderPass renderPass,
            VkDescriptorSetLayout globalSetLayout,
            VkDescriptorSetLayout textureSetLayout);
        ~TextureRenderSystem();

        void render(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout,
            VkDescriptorSetLayout textureSetLayout);
        void createPipeline(VkRenderPass renderPass);

        VkcDevice& vkcDevice;
        std::unique_ptr<VkcPipeline> vkcPipeline;
        VkPipelineLayout pipelineLayout;
    };
}
