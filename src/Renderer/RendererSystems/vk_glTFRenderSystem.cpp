#include "vk_glTFRenderSystem.h"
#include "VK_abstraction/vk_tools.h"


namespace vkc
{
	glTFRenderSystem::glTFRenderSystem(
		VkcDevice& device,
		VkRenderPass renderPass,
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout materialSetLayout
	)
		: vkcDevice(device),
		globalSetLayout(globalSetLayout),
		materialSetLayout(materialSetLayout)
	{
		createPipelineLayout(globalSetLayout, materialSetLayout);
		createPipeline(renderPass);
	}

	glTFRenderSystem::~glTFRenderSystem()
	{
		vkDestroyPipelineLayout(vkcDevice.device(), pipelineLayout, nullptr);
	}


	void glTFRenderSystem::render(FrameInfo& frameInfo) {
		for (auto& [id, go] : frameInfo.gameObjects) {
			if (!go.model || go.isSkybox) continue;
			auto gltfModel = std::static_pointer_cast<vkglTF::Model>(go.model);

			// bind pipeline
			vkcPipeline->bind(frameInfo.commandBuffer);

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				0,
				1,
				&frameInfo.globalDescriptorSet,
				0, nullptr);

			constexpr uint32_t renderFlags = vkglTF::RenderFlags::BindImages;
			constexpr uint32_t bindImageSet = 1;

			gltfModel->bind(frameInfo.commandBuffer);
			gltfModel->draw(
				frameInfo.commandBuffer,
				renderFlags,
				pipelineLayout,
				bindImageSet);
		}
	}



	void glTFRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout materialSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4) * 2;

		std::vector<VkDescriptorSetLayout> layouts = {
			globalSetLayout,
			materialSetLayout
		};

		VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineLayoutCI.pSetLayouts = layouts.data();
		pipelineLayoutCI.pushConstantRangeCount = 1;
		pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(vkcDevice.device(), &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create GLTF pipeline layout");
		}
	}
	void glTFRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != VK_NULL_HANDLE && "Pipeline layout must be created before creating the pipeline");

		PipelineConfigInfo config{};
		VkcPipeline::defaultPipelineConfigInfo(config);

		std::string vertPath = std::string(PROJECT_ROOT_DIR) + "/res/shaders/SpirV/glTFvert.vert.spv";
		std::string fragPath = std::string(PROJECT_ROOT_DIR) + "/res/shaders/SpirV/glTFfrag.frag.spv";

		config.pipelineLayout = pipelineLayout;
		config.renderPass = renderPass;
		config.attributeDescriptions = {
		vkc::vkinit::vertexInputAttributeDescription(
		0, 0, VK_FORMAT_R32G32B32_SFLOAT,
		offsetof(vkglTF::Vertex, pos)),
		vkc::vkinit::vertexInputAttributeDescription(
		0, 1, VK_FORMAT_R32G32B32_SFLOAT,
		offsetof(vkglTF::Vertex, normal)),
		vkc::vkinit::vertexInputAttributeDescription(
		0, 2, VK_FORMAT_R32G32_SFLOAT,
		offsetof(vkglTF::Vertex, uv)),
		vkc::vkinit::vertexInputAttributeDescription(
		0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)),
		};

		vkcPipeline = std::make_unique<VkcPipeline>(
			vkcDevice,
			vertPath.c_str(),
			fragPath.c_str(),
			config
		);
	}
}