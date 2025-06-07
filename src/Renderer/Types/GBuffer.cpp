#include "GBuffer.h"


namespace vkc
{
	GBuffer::GBuffer(VkcDevice& device, VkExtent2D extent)
		: vkcDevice{ device }, swapChainExtent{ extent }
	{
	}

	GBuffer::~GBuffer() {
		cleanup();
	}

	void GBuffer::cleanup() {
		// Destroy in reverse order of creation
		auto destroyAttachment = [&](GBufferAttachment& att) {
			if (att.imageView != VK_NULL_HANDLE) {
				vkDestroyImageView(vkcDevice.device(), att.imageView, nullptr);
			}
			if (att.image != VK_NULL_HANDLE) {
				vkDestroyImage(vkcDevice.device(), att.image, nullptr);
			}
			if (att.memory != VK_NULL_HANDLE) {
				vkFreeMemory(vkcDevice.device(), att.memory, nullptr);
			}
			};

		destroyAttachment(depthAttachment);
		destroyAttachment(albedoAttachment);
		destroyAttachment(normalAttachment);
		destroyAttachment(positionAttachment);
	}

	void GBuffer::createAttachment(VkFormat format, VkImageUsageFlags usage, GBufferAttachment& outAttachment)
	{
		outAttachment.format = format;
		// Create VkImage
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format;
		imageInfo.extent = { swapChainExtent.width, swapChainExtent.height, 1 };
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		// Usage: either color‐attachment or depth‐stencil
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if (vkCreateImage(vkcDevice.device(), &imageInfo, nullptr, &outAttachment.image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create GBuffer image!");
		}

		// Allocate memory
		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(vkcDevice.device(), outAttachment.image, &memReq);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = vkcDevice.findMemoryType(
			memReq.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		if (vkAllocateMemory(vkcDevice.device(), &allocInfo, nullptr, &outAttachment.memory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate GBuffer image memory!");
		}

		vkBindImageMemory(vkcDevice.device(), outAttachment.image, outAttachment.memory, 0);

		// Create image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = outAttachment.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask =
			(format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D32_SFLOAT_S8_UINT) ?
			VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(vkcDevice.device(), &viewInfo, nullptr, &outAttachment.imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create GBuffer image view!");
		}
	}

	void GBuffer::createAttachments(VkFormat positionFmt, VkFormat normalFmt, VkFormat albedoFmt, VkFormat depthFmt)
	{
		// Position: sampled later by lighting pass → COLOR_ATTACHMENT & SAMPLED
		createAttachment(
			positionFmt,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			positionAttachment
		);

		// Normal: same as position
		createAttachment(
			normalFmt,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			normalAttachment
		);

		// Albedo: same concept
		createAttachment(
			albedoFmt,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			albedoAttachment
		);

		// Depth (no sampling in basic deferred setup, but still useful to store)
		createAttachment(
			depthFmt,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			depthAttachment
		);
	}

	std::vector<VkImageView> GBuffer::getColorAttachmentViews() const
	{
		return {
			positionAttachment.imageView,
			normalAttachment.imageView,
			albedoAttachment.imageView
		};
	}
}