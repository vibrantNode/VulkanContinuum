// vk_basicRenderSystem.cpp
#include "vk_basicRenderSystem.h"

// External
#define GLM_FORCE_RADIANS	
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// STD
#include <string>
#include <array>
#include <cassert>
#include <stdexcept>


namespace vkc 
{

	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
		int textureIndex;
	};

	SimpleRenderSystem::SimpleRenderSystem(VkcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout)
		: vkcDevice{ device }, globalSetLayout{ globalSetLayout }, textureSetLayout{ textureSetLayout }
	{
		createPipelineLayout(globalSetLayout, textureSetLayout);
		createPipeline(renderPass);

	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(vkcDevice.device(), pipelineLayout, nullptr);
	}



	void SimpleRenderSystem::render(FrameInfo& frameInfo)
	{

		vkcPipeline->bind(frameInfo.commandBuffer);

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

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;

			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();
			push.textureIndex = obj.textureIndex;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			if (obj.model) {
				obj.model->bind(frameInfo.commandBuffer);
				obj.model->draw(frameInfo.commandBuffer);
			}

		}

	}



	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout) {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		// Include both descriptor set layouts
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
			globalSetLayout,
			textureSetLayout
		};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(vkcDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}
	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) 
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		VkcPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;

		// Construct paths using PROJECT_ROOT_DIR
		std::string vertShaderPath = std::string(PROJECT_ROOT_DIR) + "/res/Shaders/SpirV/vert.vert.spv";
		std::string fragShaderPath = std::string(PROJECT_ROOT_DIR) + "/res/Shaders/SpirV/frag.frag.spv";

		vkcPipeline = std::make_unique<VkcPipeline>(
			vkcDevice,
			vertShaderPath.c_str(),
			fragShaderPath.c_str(),
			pipelineConfig
		);
	}
}// namespace vkc