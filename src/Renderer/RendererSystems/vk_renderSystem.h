#pragma once
#include <VK_abstraction/vk_device.h>
#include <VK_abstraction/vk_pipeline.h>
#include "Game/vk_gameObject.h"
#include <VK_abstraction/vk_frameInfo.h>
#include "Game/Camera/vk_camera.h"


namespace vkc {

    class VkcRenderSystem {
    public:
        virtual ~VkcRenderSystem() = default;

        virtual void init(VkcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) {
            // Default empty implementation
        }

        virtual void update(FrameInfo& frameInfo, GlobalUbo& ubo) {
            // Default empty implementation
        }

        virtual void render(FrameInfo& frameInfo) = 0;
    };
}// namespace vkc