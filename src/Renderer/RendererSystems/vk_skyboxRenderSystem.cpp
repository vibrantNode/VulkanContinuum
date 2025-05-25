#include "vk_skyboxRenderSystem.h"





namespace vkc
{
	SkyboxRenderSystem::SkyboxRenderSystem(VkcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: vkcDevice{ device }
	{
	}
	void SkyboxRenderSystem::render(FrameInfo& frameInfo, VkDescriptorSet skyboxDescriptorSet)
	{
	}
	void SkyboxRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
	}
	void SkyboxRenderSystem::createPipeline(VkRenderPass renderPass)
	{
	}
}