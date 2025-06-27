#pragma once
#include "vk_renderSystem.h"
#include "AppCore/vk_assetManager.h"
#include "Renderer/vk_descriptorManager.h"
#include "VK_abstraction/vk_pipeline.h"
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_glTFModel.h"

// STD
#include <memory>
#include <vector>

namespace vkc
{
	class glTFRenderSystem : public VkcRenderSystem
	{
	public:
		glTFRenderSystem(
			VkcDevice& device,
			VkRenderPass renderPass,
			VkDescriptorSetLayout globalSetLayout
		);
		~glTFRenderSystem();
		void render(FrameInfo& frameInfo) override;

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VkcDevice& vkcDevice;
		VkDescriptorSetLayout globalSetLayout;
		VkDescriptorSetLayout textureSetLayout;

		std::unique_ptr<VkcPipeline> opaquePipeline;
		std::unique_ptr<VkcPipeline> maskPipeline;
		std::unique_ptr<VkcPipeline> blendPipeline;

		VkPipelineLayout pipelineLayout;

	};
}