#pragma once

#include <VK_abstraction/vk_device.h>
#include <VK_abstraction/vk_pipeline.h>
#include <VK_abstraction/vk_gameObject.h>
#include <VK_abstraction/vk_frameInfo.h>
#include <VK_abstraction/vk_camera.h>



#include <memory>
#include <vector>



namespace vkc {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(VkcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VkcDevice& vkcDevice;


		std::unique_ptr<VkcPipeline> vkcPipeline;
		VkPipelineLayout pipelineLayout;
	};
}
