#pragma once


#include <vulkan/vulkan.h>
#include <string>

#include <array>
namespace vkc
{
	class VkcDevice; // Forward declare your device wrapper

	class VkcTexture
	{
	public:
		VkcTexture(VkcDevice* device);
		~VkcTexture();

		bool LoadFromFile(const std::string& filename);
		bool LoadCubemap(const std::array<std::string, 6>& faceFilePaths);

		void Destroy();
		void UpdateDescriptor();

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

		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;

		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkDescriptorImageInfo descriptor{};

		VkFormat format = VK_FORMAT_UNDEFINED;
		uint32_t mipLevels = 1;
		uint32_t layerCount = 1;

		bool isCubemap{ false };
		bool IsCubemap() const { return isCubemap; }   
	};

}
