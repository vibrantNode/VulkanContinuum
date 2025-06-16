#include "vk_texture.h"
#include "vk_device.h"

#include <stb_image.h>
#include <stdexcept>
#include <cstring>



namespace vkc
{
	VkcTexture::VkcTexture(VkcDevice* device) : m_pdevice(device) {}

	VkcTexture::~VkcTexture()
	{
		Destroy();
	}

	bool VkcTexture::STBLoadFromFile(const std::string& filename)
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
		m_pdevice->createBuffer(
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingMemory);

		void* data;
		vkMapMemory(m_pdevice->logicalDevice, stagingMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_pdevice->logicalDevice, stagingMemory);
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

		vkDestroyBuffer(m_pdevice->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(m_pdevice->logicalDevice, stagingMemory, nullptr);

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

	bool VkcTexture::KTXLoadFromFile(
		const std::string& filename,
		VkFormat           format,
		VkcDevice* device,
		VkQueue            copyQueue,
		VkImageUsageFlags  imageUsageFlags,
		VkImageLayout      imageLayout,
		bool               forceLinear
	)
	{
		ktxTexture* ktxTexture;
		ktxResult result = loadKTXFile(filename, &ktxTexture);
		assert(result == KTX_SUCCESS);

		this->m_pdevice = device;
		width = ktxTexture->baseWidth;
		height = ktxTexture->baseHeight;
		mipLevels = ktxTexture->numLevels;

		ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
		ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(device->physicalDevice, format, &formatProperties);

		VkBool32 useStaging = !forceLinear;

		VkMemoryAllocateInfo memAllocInfo = vkc::vkinit::memoryAllocateInfo();
		VkMemoryRequirements memReqs;

		VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		if (useStaging)
		{
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingMemory;

			VkBufferCreateInfo bufferCreateInfo = vkc::vkinit::bufferCreateInfo();
			bufferCreateInfo.size = ktxTextureSize;
			// This buffer is used as a transfer source for the buffer copy
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VK_CHECK_RESULT(vkCreateBuffer(device->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

			// Get memory requirements for the staging buffer (alignment, memory type bits)
			vkGetBufferMemoryRequirements(device->logicalDevice, stagingBuffer, &memReqs);

			memAllocInfo.allocationSize = memReqs.size;
			// Get memory type index for a host visible buffer
			memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &stagingMemory));
			VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));

			// Copy texture data into staging buffer
			uint8_t* data;
			VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, stagingMemory, 0, memReqs.size, 0, (void**)&data));
			memcpy(data, ktxTextureData, ktxTextureSize);
			vkUnmapMemory(device->logicalDevice, stagingMemory);

			// Setup buffer copy regions for each mip level
			std::vector<VkBufferImageCopy> bufferCopyRegions;

			for (uint32_t i = 0; i < mipLevels; i++)
			{
				ktx_size_t offset;
				KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
				assert(result == KTX_SUCCESS);

				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = i;
				bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = std::max(1u, ktxTexture->baseWidth >> i);
				bufferCopyRegion.imageExtent.height = std::max(1u, ktxTexture->baseHeight >> i);
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;
				

				bufferCopyRegions.push_back(bufferCopyRegion);
			}

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
			VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCreateInfo, nullptr, &image));

			vkGetImageMemoryRequirements(device->logicalDevice, image, &memReqs);

			memAllocInfo.allocationSize = memReqs.size;

			memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &deviceMemory));
			VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, image, deviceMemory, 0));

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
				static_cast<uint32_t>(bufferCopyRegions.size()),
				bufferCopyRegions.data()
			);

			// Change texture image layout to shader read after all mip levels have been copied
			this->imageLayout = imageLayout;
			vkc::tools::setImageLayout(
				copyCmd,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				imageLayout,
				subresourceRange);

			device->flushCommandBuffer(copyCmd, copyQueue);

			// Clean up staging resources
			vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
			vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
		}
		else
		{
			// Prefer using optimal tiling, as linear tiling 
			// may support only a small set of features 
			// depending on implementation (e.g. no mip maps, only one layer, etc.)

			// Check if this support is supported for linear tiling
			assert(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

			VkImage mappableImage;
			VkDeviceMemory mappableMemory;

			VkImageCreateInfo imageCreateInfo = vkc::vkinit::imageCreateInfo();
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = format;
			imageCreateInfo.extent = { width, height, 1 };
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
			imageCreateInfo.usage = imageUsageFlags;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			// Load mip map level 0 to linear tiling image
			VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCreateInfo, nullptr, &mappableImage));

			// Get memory requirements for this image 
			// like size and alignment
			vkGetImageMemoryRequirements(device->logicalDevice, mappableImage, &memReqs);
			// Set memory allocation size to required memory size
			memAllocInfo.allocationSize = memReqs.size;

			// Get memory type that can be mapped to host memory
			memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			// Allocate host memory
			VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &mappableMemory));

			// Bind allocated image for use
			VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, mappableImage, mappableMemory, 0));

			// Get sub resource layout
			// Mip map count, array layer, etc.
			VkImageSubresource subRes = {};
			subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subRes.mipLevel = 0;

			VkSubresourceLayout subResLayout;
			void* data;

			// Get sub resources layout 
			// Includes row pitch, size offsets, etc.
			vkGetImageSubresourceLayout(device->logicalDevice, mappableImage, &subRes, &subResLayout);

			// Map image memory
			VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, mappableMemory, 0, memReqs.size, 0, &data));

			// Copy image data into memory
			memcpy(data, ktxTextureData, memReqs.size);

			vkUnmapMemory(device->logicalDevice, mappableMemory);

			// Linear tiled images don't need to be staged
			// and can be directly used as textures
			image = mappableImage;
			deviceMemory = mappableMemory;
			this->imageLayout = imageLayout;

			// Setup image memory barrier
			vkc::tools::setImageLayout(copyCmd, image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, imageLayout);

			device->flushCommandBuffer(copyCmd, copyQueue);
		}
		ktxTexture_Destroy(ktxTexture);
		// Create sampler with anisotropic filtering
		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = static_cast<float>(mipLevels);
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;

		// Enable anisotropy if supported
		samplerCreateInfo.anisotropyEnable =
			device->enabledFeatures.samplerAnisotropy ? VK_TRUE : VK_FALSE;
		samplerCreateInfo.maxAnisotropy =
			samplerCreateInfo.anisotropyEnable
			? device->properties.limits.maxSamplerAnisotropy
			: 1.0f;

		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerCreateInfo, nullptr, &sampler));

		// Create image view
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1 };
		viewCreateInfo.image = image;
		VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &view));

		// Update descriptor for shader sampling
		updateDescriptor();

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
		m_pdevice->createBuffer(totalSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingMemory);

		// copy all 6 faces into the staging buffer, one after another
		void* data;
		vkMapMemory(m_pdevice->logicalDevice, stagingMemory, 0, totalSize, 0, &data);
		for (int i = 0; i < 6; i++) {
			memcpy((char*)data + layerSize * i, images[i], layerSize);
			stbi_image_free(images[i]);
		}
		vkUnmapMemory(m_pdevice->logicalDevice, stagingMemory);

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

		vkDestroyBuffer(m_pdevice->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(m_pdevice->logicalDevice, stagingMemory, nullptr);

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
		if (sampler) vkDestroySampler(m_pdevice->logicalDevice, sampler, nullptr);
		if (view) vkDestroyImageView(m_pdevice->logicalDevice, view, nullptr);
		if (image) vkDestroyImage(m_pdevice->logicalDevice, image, nullptr);
		if (deviceMemory) vkFreeMemory(m_pdevice->logicalDevice, deviceMemory, nullptr);
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

		if (vkCreateImage(m_pdevice->logicalDevice, &info, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}
		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(m_pdevice->logicalDevice, image, &memReq);
		deviceMemory = AllocateMemory(memReq, properties);
		vkBindImageMemory(m_pdevice->logicalDevice, image, deviceMemory, 0);
		return true;
	}

	VkDeviceMemory VkcTexture::AllocateMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties)
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = m_pdevice->findMemoryType(memRequirements.memoryTypeBits, properties);

		VkDeviceMemory memory;
		if (vkAllocateMemory(m_pdevice->logicalDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory!");
		}

		return memory;
	}

	void VkcTexture::updateDescriptor()
	{
	}

	void VkcTexture::destroy()
	{
	}

	ktxResult VkcTexture::loadKTXFile(std::string filename, ktxTexture** target)
	{
		ktxResult result = KTX_SUCCESS;
		if (!vkc::tools::fileExists(filename)) {
			throw std::runtime_error("KTX file not found: " + filename);
		}
		result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);
		
		return result;
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

		if (vkCreateImageView(m_pdevice->logicalDevice, &viewInfo, nullptr, &view) != VK_SUCCESS) {
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

		if (vkCreateSampler(m_pdevice->logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	void VkcTexture::TransitionImageLayout(
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		uint32_t    layerCount)
	{
		VkCommandBuffer cmd = m_pdevice->beginSingleTimeCommands();
		m_pdevice->transitionImageLayout(
			image, format,
			oldLayout, newLayout,
			mipLevels, layerCount);
		m_pdevice->endSingleTimeCommands(cmd);
	}

	void VkcTexture::CopyBufferToImage(
		VkBuffer buffer,
		uint32_t width,
		uint32_t height,
		uint32_t layerCount)
	{
		m_pdevice->copyBufferToImage(buffer,
			image,
			width, height,
			layerCount);
	}
	void VkcTexture::fromBuffer(void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight, VkcDevice* pdevice, VkQueue copyQueue, VkFilter filter, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout)
	{
		assert(buffer);

		this->m_pdevice = pdevice;
		width = texWidth;
		height = texHeight;
		mipLevels = 1;

	
		VkMemoryAllocateInfo memAllocInfo = vkc::vkinit::memoryAllocateInfo();
		VkMemoryRequirements memReqs;

		// Use a separate command buffer for texture loading
		VkCommandBuffer copyCmd = m_pdevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		// Create a host-visible staging buffer that contains the raw image data
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo = vkc::vkinit::bufferCreateInfo();
		bufferCreateInfo.size = bufferSize;
		// This buffer is used as a transfer source for the buffer copy
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		vkCreateBuffer(m_pdevice->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer);

		// Get memory requirements for the staging buffer (alignment, memory type bits)
		vkGetBufferMemoryRequirements(m_pdevice->logicalDevice, stagingBuffer, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;
		// Get memory type index for a host visible buffer
		memAllocInfo.memoryTypeIndex = m_pdevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vkAllocateMemory(m_pdevice->logicalDevice, &memAllocInfo, nullptr, &stagingMemory);
		vkBindBufferMemory(m_pdevice->logicalDevice, stagingBuffer, stagingMemory, 0);

		// Copy texture data into staging buffer
		uint8_t* data;
		vkMapMemory(m_pdevice->logicalDevice, stagingMemory, 0, memReqs.size, 0, (void**)&data);
		memcpy(data, buffer, bufferSize);
		vkUnmapMemory(m_pdevice->logicalDevice, stagingMemory);

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
		vkCreateImage(m_pdevice->logicalDevice, &imageCreateInfo, nullptr, &image);

		vkGetImageMemoryRequirements(m_pdevice->logicalDevice, image, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;

		memAllocInfo.memoryTypeIndex = m_pdevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkAllocateMemory(m_pdevice->logicalDevice, &memAllocInfo, nullptr, &deviceMemory);
		vkBindImageMemory(m_pdevice->logicalDevice, image, deviceMemory, 0);

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
		vkDestroyBuffer(m_pdevice->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(m_pdevice->logicalDevice, stagingMemory, nullptr);

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
		vkCreateSampler(m_pdevice->logicalDevice, &samplerCreateInfo, nullptr, &sampler);

		// Create image view
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.pNext = NULL;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.image = image;
		vkCreateImageView(m_pdevice->logicalDevice, &viewCreateInfo, nullptr, &view);

		// Update descriptor image info member that can be used for setting up descriptor sets
		UpdateDescriptor();
	}
}
