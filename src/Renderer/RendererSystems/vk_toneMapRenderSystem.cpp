#include "vk_toneMapRenderSystem.h"


namespace vkc
{
	ToneMapSystem::ToneMapSystem(VkcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorLayout) : vkcDevice(device), descriptorSetLayout(descriptorLayout)
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}
	
	ToneMapSystem::~ToneMapSystem()
	{
	}
	void ToneMapSystem::render(FrameInfo& frameInfo)
	{
		toneMapPipeline->bind(frameInfo.commandBuffer);
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.hdrDescriptorSet,  // contains hdrImage + sampler
			0, nullptr);

		// Fullscreen triangle, 3 vertices, no vertex buffer
		vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);
	}
	void ToneMapSystem::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &descriptorSetLayout; // sampler2D hdrImage

		if (vkCreatePipelineLayout(vkcDevice.device(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create tone mapping pipeline layout");
	}
	void ToneMapSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr);

		PipelineConfigInfo config{};
		VkcPipeline::defaultPipelineConfigInfo(config);
		config.renderPass = renderPass;
		config.pipelineLayout = pipelineLayout;

		config.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		config.attributeDescriptions.clear();  // no vertex input
		config.bindingDescriptions.clear();

		auto vertSpv = std::string(PROJECT_ROOT_DIR) + "/res/shaders/SpirV/tone_map.vert.spv";
		auto fragSpv = std::string(PROJECT_ROOT_DIR) + "/res/shaders/SpirV/tone_map.frag.spv";

		toneMapPipeline = std::make_unique<VkcPipeline>(
			vkcDevice,
			vertSpv,
			fragSpv,
			config);
	}
}