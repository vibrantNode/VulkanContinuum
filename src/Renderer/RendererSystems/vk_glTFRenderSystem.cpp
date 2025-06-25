#include "vk_glTFRenderSystem.h"
#include "VK_abstraction/vk_tools.h"


namespace vkc
{
	glTFRenderSystem::glTFRenderSystem(
		VkcDevice& device,
		VkRenderPass renderPass,
		VkDescriptorSetLayout globalSetLayout
	)
		: vkcDevice(device),
		globalSetLayout(globalSetLayout)
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	glTFRenderSystem::~glTFRenderSystem()
	{
		vkDestroyPipelineLayout(vkcDevice.device(), pipelineLayout, nullptr);
	}


	void glTFRenderSystem::render(FrameInfo& frameInfo) {

		// bind pipeline
		vkcPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			/*firstSet=*/0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr);


		for (auto& [id, go] : frameInfo.gameObjects) {
			if (!go.model || go.isSkybox || go.isOBJ) continue;
			auto gltfModel = std::static_pointer_cast<vkglTF::Model>(go.model);


			gltfModel->bind(frameInfo.commandBuffer);

			for (auto* node : gltfModel->linearNodes) {
				if (!node->mesh) continue;

				// Update per-node UBO
				glm::mat4 world = go.transform.mat4() * node->getMatrix();
				glm::mat4 normalMat = glm::transpose(glm::inverse(world));
				memcpy(node->mesh->uniformBuffer.mapped, &world, sizeof(world));// exception thrown read access violation
				memcpy((char*)node->mesh->uniformBuffer.mapped + sizeof(world), &normalMat, sizeof(normalMat));

				// Bind descriptor sets: 0 = global, 1 = per-node UBO
				vkCmdBindDescriptorSets(
					frameInfo.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineLayout,
					1, 1,
					&node->mesh->uniformBuffer.descriptorSet,
					0, nullptr);

				// Draw just this node (it binds set 2 inside for material)

				gltfModel->drawNode(
					node,
					frameInfo.commandBuffer,
					vkglTF::RenderFlags::BindImages,
					pipelineLayout,
					2
				);
				//std::cout << "Vertex count: " << gltfModel->vertices.count << std::endl;
				//std::cout << "Index count: " << gltfModel->indices.count << std::endl;
				//std::cout << "Node count: " << gltfModel->nodes.size() << std::endl;

			}
			
		}
	}



	void glTFRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{

		const std::vector<VkDescriptorSetLayout> layouts = {
			globalSetLayout,
			vkglTF::descriptorSetLayoutUbo,
			vkglTF::descriptorSetLayoutImage
		};
		

		VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineLayoutCI.pSetLayouts = layouts.data();
		pipelineLayoutCI.pPushConstantRanges = nullptr;
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
				0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, pos)),
			vkc::vkinit::vertexInputAttributeDescription(
				0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, normal)),
			vkc::vkinit::vertexInputAttributeDescription(
				0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(vkglTF::Vertex, uv)),
			vkc::vkinit::vertexInputAttributeDescription(
				0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vkglTF::Vertex, color)),
		/*	vkc::vkinit::vertexInputAttributeDescription(
				0, 4, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vkglTF::Vertex, joint0)),
			vkc::vkinit::vertexInputAttributeDescription(
				0, 5, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vkglTF::Vertex, weight0)),*/
			vkc::vkinit::vertexInputAttributeDescription(
				0, 4, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vkglTF::Vertex, tangent))
		};
		config.bindingDescriptions = {
		vkc::vkinit::vertexInputBindingDescription(0, sizeof(vkglTF::Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
		};
		vkcPipeline = std::make_unique<VkcPipeline>(
			vkcDevice,
			vertPath.c_str(),
			fragPath.c_str(),
			config
		);
	}
}