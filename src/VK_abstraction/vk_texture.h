#pragma once


#include <vulkan/vulkan.h>

#include <string>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <array>

#include "vk_initializers.h"
#include "vk_tools.h"

#include "ktx.h"
#include "ktxvulkan.h"


namespace vkc
{
	class VkcDevice; // Forward declare your device wrapper

	class VkcTexture
	{
	public:
		VkcTexture() = default;
		VkcTexture(VkcDevice* device);
		~VkcTexture();

		bool STBLoadFromFile(const std::string& filename);
		bool KTXLoadFromFile(
			const std::string& filename,
			VkFormat           format,
			VkcDevice*         device,
			VkQueue            copyQueue,
			VkImageUsageFlags  imageUsageFlags,
			VkImageLayout      imageLayout,
			bool               forceLinear
		);
		bool LoadCubemap(const std::array<std::string, 6>& faceFilePaths);
		void KtxLoadCubemapFromFile(std::string filename, VkFormat format, vkc::VkcDevice* device, VkQueue copyQueue, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout);
		void Destroy();
		void UpdateDescriptor();
		void fromBuffer(
			void* buffer,
			VkDeviceSize       bufferSize,
			VkFormat           format,
			uint32_t           texWidth,
			uint32_t           texHeight,
			VkcDevice* device,
			VkQueue            copyQueue,
			VkFilter           filter = VK_FILTER_LINEAR,
			VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkDescriptorImageInfo GetDescriptor() const { return descriptor; }
		VkSampler GetSampler() const { return sampler; }
		VkImageView GetImageView() const { return view; }
	private:
		bool CreateImage(
			uint32_t width, uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			uint32_t arrayLayers,
			VkImageCreateFlags flags = 0);
		void CreateImageView(
			VkFormat format,
			VkImageViewType viewType,
			uint32_t layerCount);

		void CreateSampler();
		void TransitionImageLayout(
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			uint32_t    layerCount);

		void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height, uint32_t layerCount);
		VkDeviceMemory AllocateMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties);



	public:
		VkcDevice* device;
		uint32_t              width{ 0 }, height{ 0 };
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;

		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkDescriptorImageInfo descriptor{};

		VkFormat format = VK_FORMAT_UNDEFINED;
		uint32_t mipLevels = 1;
		uint32_t layerCount = 1;

		void updateDescriptor();
		void destroy();
		ktxResult loadKTXFile(std::string filename, ktxTexture** target);

		bool isCubemap{ false };
		bool IsCubemap() const { return isCubemap; }
	};

}
