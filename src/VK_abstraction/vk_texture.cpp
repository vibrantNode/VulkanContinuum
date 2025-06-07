#include "vk_texture.h"
#include "vk_device.h"

#include <stb_image.h>
#include <stdexcept>
#include <cstring>

namespace vkc
{
	VkcTexture::VkcTexture(VkcDevice* device) : device(device) {}

	VkcTexture::~VkcTexture()
	{
		Destroy();
	}

	bool VkcTexture::LoadFromFile(const std::string& filename)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image: " + filename);
		}
		VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4;

		// Stage data
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		device->createBuffer(
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingMemory);

		void* data;
		vkMapMemory(device->logicalDevice, stagingMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device->logicalDevice, stagingMemory);
		stbi_image_free(pixels);

		// Create and upload to 2D image
		CreateImage(
			static_cast<uint32_t>(texWidth),
			static_cast<uint32_t>(texHeight),
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			1, // arrayLayers
			0  // flags
		);

		TransitionImageLayout(
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1 // layerCount
		);
		CopyBufferToImage(
			stagingBuffer,
			static_cast<uint32_t>(texWidth),
			static_cast<uint32_t>(texHeight),
			1 // layerCount
		);
		TransitionImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			1 // layerCount
		);

		vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);

		// Create view and sampler for 2D
		CreateImageView(
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_VIEW_TYPE_2D,
			1 // layerCount
		);
		CreateSampler();
		UpdateDescriptor();

		return true;
	}

	bool VkcTexture::LoadCubemap(const std::array<std::string, 6>& faces) {
		isCubemap = true;

		int w, h, c;
		std::vector<stbi_uc*> images(6);
		for (int i = 0; i < 6; i++) {
			images[i] = stbi_load(faces[i].c_str(), &w, &h, &c, STBI_rgb_alpha);
			if (!images[i]) {
				throw std::runtime_error("Failed to load skybox face: " + faces[i]);
			}
		}
		VkDeviceSize layerSize = w * h * 4;
		VkDeviceSize totalSize = layerSize * 6;

		// create a single staging buffer large enough for 6 faces
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		device->createBuffer(totalSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingMemory);

		// copy all 6 faces into the staging buffer, one after another
		void* data;
		vkMapMemory(device->logicalDevice, stagingMemory, 0, totalSize, 0, &data);
		for (int i = 0; i < 6; i++) {
			memcpy((char*)data + layerSize * i, images[i], layerSize);
			stbi_image_free(images[i]);
		}
		vkUnmapMemory(device->logicalDevice, stagingMemory);

		// Create the cube image: 6 layers, CUBE_COMPATIBLE flag
		CreateImage(w, h,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			6,                            // arrayLayers
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

		// Transition and copy each layer
		TransitionImageLayout(
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			/*layerCount=*/6);
		CopyBufferToImage(
			stagingBuffer, w, h,
			/*layerCount=*/6);
		TransitionImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			/*layerCount=*/6);

		vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);

		// Create a CUBE view
		CreateImageView(
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_VIEW_TYPE_CUBE,
			/*layerCount=*/6);

		CreateSampler();
		UpdateDescriptor();
		return true;
	}

	void VkcTexture::Destroy()
	{
		if (sampler) vkDestroySampler(device->logicalDevice, sampler, nullptr);
		if (view) vkDestroyImageView(device->logicalDevice, view, nullptr);
		if (image) vkDestroyImage(device->logicalDevice, image, nullptr);
		if (memory) vkFreeMemory(device->logicalDevice, memory, nullptr);
	}

	void VkcTexture::UpdateDescriptor()
	{
		descriptor.sampler = sampler;
		descriptor.imageView = view;
		descriptor.imageLayout = imageLayout;
	}

	bool VkcTexture::CreateImage(
		uint32_t width, uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		uint32_t arrayLayers,
		VkImageCreateFlags flags)
	{
		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.flags = flags;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.extent = { width, height, 1 };
		info.mipLevels = 1;
		info.arrayLayers = arrayLayers;
		info.format = format;
		info.tiling = tiling;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		info.usage = usage;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device->logicalDevice, &info, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}
		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(device->logicalDevice, image, &memReq);
		memory = AllocateMemory(memReq, properties);
		vkBindImageMemory(device->logicalDevice, image, memory, 0);
		return true;
	}

	VkDeviceMemory VkcTexture::AllocateMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties)
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);

		VkDeviceMemory memory;
		if (vkAllocateMemory(device->logicalDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory!");
		}

		return memory;
	}

	void VkcTexture::CreateImageView(
		VkFormat format,
		VkImageViewType viewType,
		uint32_t layerCount)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = viewType;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = layerCount;

		if (vkCreateImageView(device->logicalDevice, &viewInfo, nullptr, &view) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
		}
	}

	void VkcTexture::CreateSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(device->logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	void VkcTexture::TransitionImageLayout(
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		uint32_t    layerCount)
	{
		VkCommandBuffer cmd = device->beginSingleTimeCommands();
		device->transitionImageLayout(
			image, format,
			oldLayout, newLayout,
			mipLevels, layerCount);
		device->endSingleTimeCommands(cmd);
	}

	void VkcTexture::CopyBufferToImage(
		VkBuffer buffer,
		uint32_t width,
		uint32_t height,
		uint32_t layerCount)
	{
		device->copyBufferToImage(buffer,
			image,
			width, height,
			layerCount);
	}
	void VkcTexture::fromBuffer(void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight, VkcDevice* pdevice, VkQueue copyQueue, VkFilter filter, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout)
	{
		assert(buffer);

		this->device = pdevice;
		width = texWidth;
		height = texHeight;
		mipLevels = 1;

	
		VkMemoryAllocateInfo memAllocInfo = vkc::vkinit::memoryAllocateInfo();
		VkMemoryRequirements memReqs;

		// Use a separate command buffer for texture loading
		VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		// Create a host-visible staging buffer that contains the raw image data
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo = vkc::vkinit::bufferCreateInfo();
		bufferCreateInfo.size = bufferSize;
		// This buffer is used as a transfer source for the buffer copy
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		vkCreateBuffer(device->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer);

		// Get memory requirements for the staging buffer (alignment, memory type bits)
		vkGetBufferMemoryRequirements(device->logicalDevice, stagingBuffer, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;
		// Get memory type index for a host visible buffer
		memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &stagingMemory);
		vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0);

		// Copy texture data into staging buffer
		uint8_t* data;
		vkMapMemory(device->logicalDevice, stagingMemory, 0, memReqs.size, 0, (void**)&data);
		memcpy(data, buffer, bufferSize);
		vkUnmapMemory(device->logicalDevice, stagingMemory);

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = width;
		bufferCopyRegion.imageExtent.height = height;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;

		// Create optimal tiled target image
		VkImageCreateInfo imageCreateInfo = vkc::vkinit::imageCreateInfo();
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.mipLevels = mipLevels;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = { width, height, 1 };
		imageCreateInfo.usage = imageUsageFlags;
		// Ensure that the TRANSFER_DST bit is set for staging
		if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		vkCreateImage(device->logicalDevice, &imageCreateInfo, nullptr, &image);

		vkGetImageMemoryRequirements(device->logicalDevice, image, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;

		memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &memory);
		vkBindImageMemory(device->logicalDevice, image, memory, 0);

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = mipLevels;
		subresourceRange.layerCount = 1;

		// Image barrier for optimal image (target)
		// Optimal image will be used as destination for the copy
		vkc::tools::setImageLayout(
			copyCmd,
			image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		// Copy mip levels from staging buffer
		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion
		);

		// Change texture image layout to shader read after all mip levels have been copied
		this->imageLayout = imageLayout;
		vkc::tools::setImageLayout(
			copyCmd,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			imageLayout,
			subresourceRange);

		//device->flushCommandBuffer(copyCmd, copyQueue);
		

		// Clean up staging resources
		vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);

		// Create sampler
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = filter;
		samplerCreateInfo.minFilter = filter;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		vkCreateSampler(device->logicalDevice, &samplerCreateInfo, nullptr, &sampler);

		// Create image view
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.pNext = NULL;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.image = image;
		vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &view);

		// Update descriptor image info member that can be used for setting up descriptor sets
		UpdateDescriptor();
	}
}
