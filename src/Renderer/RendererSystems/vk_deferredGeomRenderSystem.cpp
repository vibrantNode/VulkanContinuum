#include "vk_deferredGeomRenderSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>



namespace vkc
{
	DeferredGeometrySystem::DeferredGeometrySystem(
		VkcDevice& device,
		GBuffer& gbuffer,
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout textureSetLayout,
		VkRenderPass geometryPass)
		: vkcDevice{ device },
		gbuffer{ gbuffer },
		globalSetLayout{ globalSetLayout },
		textureSetLayout{ textureSetLayout }
	{
		createPipelineLayout(globalSetLayout, textureSetLayout);
		createPipeline(geometryPass);
	}

	DeferredGeometrySystem::~DeferredGeometrySystem() {
		vkDestroyPipelineLayout(vkcDevice.device(), pipelineLayout, nullptr);
	}

	void DeferredGeometrySystem::createPipelineLayout(
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout textureSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(GeometryPushConstant);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
			globalSetLayout,     // set = 0: global UBO (projection, view, lights)
			textureSetLayout     // set = 1: bindless textures
		};

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		layoutInfo.pSetLayouts = descriptorSetLayouts.data();
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(vkcDevice.device(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create geometry pipeline layout!");
		}
	}

	void DeferredGeometrySystem::createPipeline(VkRenderPass geometryPass)
	{
		assert(pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		VkcPipeline::defaultPipelineConfigInfo(pipelineConfig);

		// Set our new render pass (the GBuffer MRT pass)
		pipelineConfig.renderPass = geometryPass;
		pipelineConfig.pipelineLayout = pipelineLayout;

		// The “subpass” index is 0, since we only have one subpass in geometryPass:
		pipelineConfig.subpass = 0;

		// Input‐assembly and rasterization settings remain similar to your old forward pipeline
		// (enable depth test/write, etc.)
		pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
		pipelineConfig.depthStencilInfo.depthWriteEnable = VK_TRUE;
		pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;

		// Vertex/fragment SPIR-V shaders that output to multiple attachments:
		//std::string vertShaderPath = std::string(PROJECT_ROOT_DIR) + "/res/Shaders/SpirV/deferredGeom.vert.spv";
		//std::string fragShaderPath = std::string(PROJECT_ROOT_DIR) + "/res/Shaders/SpirV/deferredGeom.frag.spv";

		geometryPipeline = std::make_unique<VkcPipeline>(
			vkcDevice,
			vertShaderPath.c_str(),
			fragShaderPath.c_str(),
			pipelineConfig
		);
	}

	void DeferredGeometrySystem::render(FrameInfo& frameInfo)
	{
		// 1) Bind geometry pipeline
		geometryPipeline->bind(frameInfo.commandBuffer);

		// 2) Bind descriptor sets: set=0 (global UBO), set=1 (bindless textures)
		std::array<VkDescriptorSet, 2> descriptorSets = {
			frameInfo.globalDescriptorSet,
			frameInfo.textureDescriptorSet
		};
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			static_cast<uint32_t>(descriptorSets.size()),
			descriptorSets.data(),
			0,
			nullptr
		);

		// 3) Loop over all game objects (same as your SimpleRenderSystem)
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (!obj.model || obj.isSkybox) {
				continue; // skybox goes in its own system later, or ignore here
			}

			GeometryPushConstant push{};
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();
			push.textureIndex = obj.textureIndex;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(GeometryPushConstant),
				&push
			);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}
}