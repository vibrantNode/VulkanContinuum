#pragma once
#include <vulkan/vulkan.h>
#include "VK_abstraction/vk_device.h"

#include "ktx.h"
#include "ktxvulkan.h"


namespace vkc {
	class VulkanTexture {
	public:
	public:
		vkc::VkcDevice* device;
		VkImage               image;
		VkImageLayout         imageLayout;
		VkDeviceMemory        deviceMemory;
		VkImageView           view;
		uint32_t              width, height;
		uint32_t              mipLevels;
		uint32_t              layerCount;
		VkDescriptorImageInfo descriptor;
		VkSampler             sampler;

		void updateDescriptor();
		void destroy();
		ktxResult loadKTXFile(std::string filename, ktxTexture** target);
	};

	class Texture2D : public VulkanTexture
	{
	public:
		void loadFromFile(
			std::string        filename,
			VkFormat           format,
			vkc::VkcDevice& device,
			VkQueue            copyQueue,
			VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			bool               forceLinear = false);
		void fromBuffer(
			void* buffer,
			VkDeviceSize       bufferSize,
			VkFormat           format,
			uint32_t           texWidth,
			uint32_t           texHeight,
			vkc::VkcDevice* device,
			VkQueue            copyQueue,
			VkFilter           filter = VK_FILTER_LINEAR,
			VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	};

	class Texture2DArray : public VulkanTexture
	{
	public:
		void loadFromFile(
			std::string        filename,
			VkFormat           format,
			vkc::VkcDevice&    device,
			VkQueue            copyQueue,
			VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	};

	class TextureCubeMap : public VulkanTexture {
	public:
		void loadFromFile(
			std::string             filename,
			VkFormat                format,
			vkc::VkcDevice*         device,
			VkQueue                 copyQueue,
			VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	};
}