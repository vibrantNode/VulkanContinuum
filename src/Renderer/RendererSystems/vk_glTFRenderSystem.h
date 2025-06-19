#pragma once
#include "vk_renderSystem.h"
#include "AppCore/vk_assetManager.h"
#include "Renderer/vk_descriptorManager.h"
#include "VK_abstraction/vk_pipeline.h"
#include "VK_abstraction/vk_device.h"


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
			VkDescriptorSetLayout globalSetLayout,
			VkDescriptorSetLayout materialSetLayout
		);
		~glTFRenderSystem();
		void render(FrameInfo& frameInfo) override;

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout materialSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VkcDevice& vkcDevice;


		VkDescriptorSetLayout globalSetLayout;
		VkDescriptorSetLayout textureSetLayout;
		VkDescriptorSetLayout materialSetLayout;


		std::unique_ptr<VkcPipeline> vkcPipeline;
		VkPipelineLayout pipelineLayout;

	};
}