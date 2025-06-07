#pragma once

#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_pipeline.h"
#include "Game/vk_gameObject.h"
#include "Renderer/vk_descriptorManager.h"
#include "VK_abstraction/vk_frameInfo.h"
#include "Game/Camera/vk_camera.h"
#include "Renderer/RendererSystems/vk_renderSystem.h"
#include "Renderer/Types/GBuffer.h"




namespace vkc
{

	struct GeometryPushConstant {
		glm::mat4 modelMatrix;
		glm::mat4 normalMatrix;
		int textureIndex;
	};

	class DeferredGeometrySystem : public VkcRenderSystem {
	public:
		DeferredGeometrySystem(
			VkcDevice& device,
			GBuffer& gbuffer,
			VkDescriptorSetLayout globalSetLayout,
			VkDescriptorSetLayout textureSetLayout,
			VkRenderPass  geometryPass);

		~DeferredGeometrySystem();

		DeferredGeometrySystem(const DeferredGeometrySystem&) = delete;
		DeferredGeometrySystem& operator=(const DeferredGeometrySystem&) = delete;

		void render(FrameInfo& frameInfo) override;

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
		void createPipeline(VkRenderPass geometryPass);

		VkcDevice& vkcDevice;
		GBuffer& gbuffer;

		VkDescriptorSetLayout globalSetLayout;
		VkDescriptorSetLayout textureSetLayout;

		VkPipelineLayout pipelineLayout;
		std::unique_ptr<VkcPipeline> geometryPipeline;
	};
}