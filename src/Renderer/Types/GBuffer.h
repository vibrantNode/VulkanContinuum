#pragma once
#include "VK_abstraction/vk_device.h"
#include <vector>



namespace vkc
{
	struct GBufferAttachment {
		VkImage        image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView    imageView = VK_NULL_HANDLE;
		VkFormat       format = VK_FORMAT_UNDEFINED;
	};

	class GBuffer {
	public:
		GBuffer(VkcDevice& device, VkExtent2D extent);
		~GBuffer();

		// Call this after swapchain and depth format are known:
		void createAttachments(VkFormat positionFmt, VkFormat normalFmt, VkFormat albedoFmt, VkFormat depthFmt);

		// Returns all color‐attachment views for the MRT
		std::vector<VkImageView> getColorAttachmentViews() const;

		// Returns the depth image view
		VkImageView getDepthAttachmentView() const { return depthAttachment.imageView; }

		// Cleanup
		void cleanup();

	private:
		void createAttachment(VkFormat format, VkImageUsageFlags usage, GBufferAttachment& outAttachment);

		VkcDevice& vkcDevice;
		VkExtent2D           swapChainExtent;

		// MRT attachments
		GBufferAttachment    positionAttachment;
		GBufferAttachment    normalAttachment;
		GBufferAttachment    albedoAttachment;

		// Depth
		GBufferAttachment    depthAttachment;
	};

}