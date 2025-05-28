#include "vk_texture.h"
#include "vk_device.h"

#define STB_IMAGE_IMPLEMENTATION
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
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		device->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device->device_, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device->device_, stagingBufferMemory);

		stbi_image_free(pixels);

		CreateImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(stagingBuffer, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
		TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device->device_, stagingBuffer, nullptr);
		vkFreeMemory(device->device_, stagingBufferMemory, nullptr);

		CreateImageView(VK_FORMAT_R8G8B8A8_SRGB);
		CreateSampler();
		UpdateDescriptor();

		return true;
	}

	void VkcTexture::Destroy()
	{
		if (sampler) vkDestroySampler(device->device_, sampler, nullptr);
		if (view) vkDestroyImageView(device->device_, view, nullptr);
		if (image) vkDestroyImage(device->device_, image, nullptr);
		if (memory) vkFreeMemory(device->device_, memory, nullptr);
	}

	void VkcTexture::UpdateDescriptor()
	{
		descriptor.sampler = sampler;
		descriptor.imageView = view;
		descriptor.imageLayout = imageLayout;
	}

	bool VkcTexture::CreateImage(uint32_t width, uint32_t height, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;	
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device->device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device->device_, image, &memRequirements);

		memory = AllocateMemory(memRequirements, properties);
		vkBindImageMemory(device->device_, image, memory, 0);
		return true;
	}

	VkDeviceMemory VkcTexture::AllocateMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties)
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);

		VkDeviceMemory memory;
		if (vkAllocateMemory(device->device_, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory!");
		}

		return memory;
	}

	void VkcTexture::CreateImageView(VkFormat format)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device->device_, &viewInfo, nullptr, &view) != VK_SUCCESS) {
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

		if (vkCreateSampler(device->device_, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	void VkcTexture::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer cmd = device->beginSingleTimeCommands();
		device->transitionImageLayout(image, format, oldLayout, newLayout, mipLevels, layerCount);
		device->endSingleTimeCommands(cmd);
	}

	void VkcTexture::CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height, uint32_t layerCount)
	{
		device->copyBufferToImage(buffer, image, width, height, layerCount);
	}











	// Cubemap
	VkcCubemapTexture::VkcCubemapTexture(VkcDevice* device) : _device(device)
	{
	}
	bool VkcCubemapTexture::LoadFromFiles(const std::array<std::string, 6>& filepaths)
	{
		return false;
	}
	VkImageView VkcCubemapTexture::GetImageView() const
	{
		return VkImageView();
	}
	VkSampler VkcCubemapTexture::GetSampler() const
	{
		return VkSampler();
	}
	void VkcCubemapTexture::createCubemap(const std::array<std::string, 6>& filepaths)
	{
	}
}
