#include "OffscreenPass.h"

namespace vkc
{

	OffscreenPass::OffscreenPass(VkcDevice& deviceRef, VkExtent2D initialExtent)
		: device(deviceRef), extent(initialExtent)
	{
		// (You still need to implement createResources() here to fill
		//  hdrImage, hdrImageMemory, hdrImageView, renderPass, framebuffer)
		createResources();
	}

	OffscreenPass::~OffscreenPass()
	{
		// clean up in reverse order of creation:
		vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
		vkDestroyRenderPass(device.device(), renderPass, nullptr);
		vkDestroyImageView(device.device(), hdrImageView, nullptr);
        vkDestroySampler(device.device(), hdrSampler, nullptr);
		vkDestroyImage(device.device(), hdrImage, nullptr);
		vkFreeMemory(device.device(), hdrImageMemory, nullptr);
	}
	void OffscreenPass::resize(VkExtent2D newExtent)
	{
		if (newExtent.width == extent.width && newExtent.height == extent.height)
			return;
		// destroy old
		vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
		vkDestroyImageView(device.device(), hdrImageView, nullptr);
		vkDestroyImage(device.device(), hdrImage, nullptr);
		vkFreeMemory(device.device(), hdrImageMemory, nullptr);
		vkDestroyRenderPass(device.device(), renderPass, nullptr);

		// update extent and recreate
		extent = newExtent;
		// (re-call your createResources() logic here)
	}

	void OffscreenPass::begin(VkCommandBuffer cmd)
	{
		// 1) Clear values: color = black, depth = 1.0
		std::array<VkClearValue, 2> clears{};
		clears[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clears[1].depthStencil = { 1.0f, 0 };

		// 2) RenderPassBeginInfo
		VkRenderPassBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = renderPass;
		info.framebuffer = framebuffer;
		info.renderArea.offset = { 0, 0 };
		info.renderArea.extent = extent;
		info.clearValueCount = static_cast<uint32_t>(clears.size());
		info.pClearValues = clears.data();

		// 3) Begin
		vkCmdBeginRenderPass(cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void OffscreenPass::end(VkCommandBuffer cmd) {
		vkCmdEndRenderPass(cmd);
	}
	void OffscreenPass::createResources()
	{
        // Create HDR color image
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT; // HDR format
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device.device(), &imageInfo, nullptr, &hdrImage) != VK_SUCCESS)
            throw std::runtime_error("failed to create HDR image");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device.device(), hdrImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device.findMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device.device(), &allocInfo, nullptr, &hdrImageMemory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate HDR image memory");

        vkBindImageMemory(device.device(), hdrImage, hdrImageMemory, 0);

        // Create image view
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = hdrImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = imageInfo.format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &hdrImageView) != VK_SUCCESS)
            throw std::runtime_error("failed to create HDR image view");

        // Create Sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &hdrSampler) != VK_SUCCESS)
            throw std::runtime_error("failed to create HDR sampler");

        // Create render pass
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = imageInfo.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("failed to create offscreen render pass");

        // Create framebuffer
        VkImageView attachments[] = { hdrImageView };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to create offscreen framebuffer");
	}
}