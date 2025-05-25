#pragma once

// Project headers
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_pipeline.h"
#include "Game/vk_gameObject.h"
#include "VK_abstraction/vk_frameInfo.h"
#include "Game/Camera/vk_camera.h"
#include "Renderer/RendererSystems/vk_renderSystem.h"

// STD
#include <memory>
#include <vector>



namespace vkc
{
	class SkyboxRenderSystem : public VkcRenderSystem
	{
	public:
		SkyboxRenderSystem(VkcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		void render(FrameInfo& frameInfo, VkDescriptorSet skyboxDescriptorSet);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VkcDevice& vkcDevice;
		std::unique_ptr<VkcPipeline> vkcPipeline;
		VkPipelineLayout pipelineLayout;

	};
}