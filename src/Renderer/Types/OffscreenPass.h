// OffscreenPass.h
#pragma once
#include "VK_abstraction/vk_device.h"

namespace vkc
{
	struct OffscreenPass {
		VkcDevice& device;
		VkExtent2D extent;
		VkImage        hdrImage;
		VkDeviceMemory hdrImageMemory;
		VkImageView    hdrImageView;
		VkRenderPass   renderPass;
		VkFramebuffer  framebuffer;
		VkSampler	   hdrSampler;
		OffscreenPass(VkcDevice&, VkExtent2D);
		~OffscreenPass();
		void resize(VkExtent2D newExtent);
		void begin(VkCommandBuffer);
		void end(VkCommandBuffer);
		void createResources();

		VkImageView getColorImageView() const { return hdrImageView; }
		VkSampler getSampler() const { return hdrSampler; }
	};
}

