// vk_basicRenerSystem.h
#pragma once

// Project headers
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_pipeline.h"
#include "Game/vk_gameObject.h"
#include "VK_abstraction/vk_frameInfo.h"
#include "Game/Camera/vk_camera.h"
#include "Renderer/RendererSystems/vk_renderSystem.h"
#include "Renderer/vk_descriptorManager.h"

// STD
#include <memory>
#include <vector>



namespace vkc {
	class SimpleRenderSystem : public VkcRenderSystem {
	public:
		SimpleRenderSystem(VkcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void render(FrameInfo& frameInfo) override;
	
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VkcDevice& vkcDevice;

		//DescriptorManager& _descriptorManager;

		VkDescriptorSetLayout globalSetLayout;
		VkDescriptorSetLayout textureSetLayout;
	

		std::unique_ptr<VkcPipeline> vkcPipeline;
		VkPipelineLayout pipelineLayout;
	};
}// namespace vkc
