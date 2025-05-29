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
		vkMapMemory(device->device_, stagingMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device->device_, stagingMemory);
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

		vkDestroyBuffer(device->device_, stagingBuffer, nullptr);
		vkFreeMemory(device->device_, stagingMemory, nullptr);

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
		vkMapMemory(device->device_, stagingMemory, 0, totalSize, 0, &data);
		for (int i = 0; i < 6; i++) {
			memcpy((char*)data + layerSize * i, images[i], layerSize);
			stbi_image_free(images[i]);
		}
		vkUnmapMemory(device->device_, stagingMemory);

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

		vkDestroyBuffer(device->device_, stagingBuffer, nullptr);
		vkFreeMemory(device->device_, stagingMemory, nullptr);

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

		if (vkCreateImage(device->device_, &info, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}
		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(device->device_, image, &memReq);
		memory = AllocateMemory(memReq, properties);
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

}
