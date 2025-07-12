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
		createPipelines(renderPass);
	}

	glTFRenderSystem::~glTFRenderSystem()
	{
		vkDestroyPipelineLayout(vkcDevice.device(), pipelineLayout, nullptr);
	}


	void glTFRenderSystem::render(FrameInfo& frameInfo) {

		// Bind the global descriptor set once
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			/* firstSet */ 0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr);

		for (auto& [id, go] : frameInfo.gameObjects) {
			if (!go.model || go.isSkybox || go.isOBJ) continue;
			auto gltfModel = std::static_pointer_cast<vkglTF::Model>(go.model);

			// Bind vertex/index buffers
			gltfModel->bind(frameInfo.commandBuffer);

			for (auto* node : gltfModel->linearNodes) {
				if (!node->mesh) continue;

				// 1) Update per-node UBO
				glm::mat4 world = go.transform.mat4() * node->getMatrix();
				glm::mat4 normalMat = glm::transpose(glm::inverse(world));
				memcpy(node->mesh->uniformBuffer.mapped, &world, sizeof(world));
				memcpy((char*)node->mesh->uniformBuffer.mapped + sizeof(world),
					&normalMat, sizeof(normalMat));

				// 2) Bind per-node UBO descriptor set (set = 1)
				vkCmdBindDescriptorSets(
					frameInfo.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineLayout,
					/* firstSet */ 1, 1,
					&node->mesh->uniformBuffer.descriptorSet,
					0, nullptr);

				// 3) Choose and bind the correct pipeline variant
				auto& mat = node->mesh->primitives[0]->material; // Assuming single primitive per node
				if (mat.alphaMode == vkglTF::Material::ALPHAMODE_OPAQUE) {
					opaquePipeline->bind(frameInfo.commandBuffer);
				}
				else if (mat.alphaMode == vkglTF::Material::ALPHAMODE_MASK) {
					maskPipeline->bind(frameInfo.commandBuffer);
				}
				else { // ALPHAMODE_BLEND
					blendPipeline->bind(frameInfo.commandBuffer);
				}

				// 4) Draw the primitive (binds set = 2 inside)
				gltfModel->drawNode(
					node,
					frameInfo.commandBuffer,
					vkglTF::RenderFlags::BindImages,
					pipelineLayout,
					/* bindImageSet */ 2
				);
			}
		}
	}



	void glTFRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{

		const std::vector<VkDescriptorSetLayout> layouts = {
			globalSetLayout,
			vkglTF::descriptorSetLayoutUbo,
			vkglTF::descriptorSetLayoutImage,
            vkglTF::descriptorSetLayoutIbl
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
    void glTFRenderSystem::createPipelines(VkRenderPass renderPass)
    {
        assert(pipelineLayout != VK_NULL_HANDLE);

        auto vertSpv = std::string(PROJECT_ROOT_DIR) + "/res/shaders/SpirV/glTFvert.vert.spv";
        auto fragSpv = std::string(PROJECT_ROOT_DIR) + "/res/shaders/SpirV/glTFfrag.frag.spv";

     
        std::vector<VkVertexInputBindingDescription>  bindings = {
            vkc::vkinit::vertexInputBindingDescription(
                0, sizeof(vkglTF::Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
        };
        std::vector<VkVertexInputAttributeDescription> attributes = {
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
                0, 3, VK_FORMAT_R32G32B32A32_SFLOAT,
                offsetof(vkglTF::Vertex, color)),
            vkc::vkinit::vertexInputAttributeDescription(
                0, 4, VK_FORMAT_R32G32B32A32_SFLOAT,
                offsetof(vkglTF::Vertex, tangent))
        };

        //
        // 1) OPAQUE pipeline
        //
        PipelineConfigInfo opaqueConfig{};
        VkcPipeline::defaultPipelineConfigInfo(opaqueConfig);
        opaqueConfig.pipelineLayout = pipelineLayout;
        opaqueConfig.renderPass = renderPass;
        opaqueConfig.bindingDescriptions = bindings;
        opaqueConfig.attributeDescriptions = attributes;
        // (blendEnable = VK_FALSE by default in defaultPipelineConfigInfo)
        opaquePipeline = std::make_unique<VkcPipeline>(
            vkcDevice, vertSpv, fragSpv, opaqueConfig);

        //
        // 2) MASK (alpha‐cutout) pipeline
        //
        PipelineConfigInfo maskConfig{};
        VkcPipeline::defaultPipelineConfigInfo(maskConfig);
        maskConfig.pipelineLayout = pipelineLayout;
        maskConfig.renderPass = renderPass;
        maskConfig.bindingDescriptions = bindings;
        maskConfig.attributeDescriptions = attributes;
        // disable blending for pure cutout
        maskConfig.colorBlendAttachment.blendEnable = VK_FALSE;
        maskConfig.colorBlendInfo.attachmentCount = 1;
        maskConfig.colorBlendInfo.pAttachments = &maskConfig.colorBlendAttachment;

        // set up specialization for ALPHA_MASK = true, cutoff = e.g. 0.5f
        struct SpecData { VkBool32 alphaMask; float cutoff; };
        static SpecData specData{ VK_TRUE, 0.5f };
        static VkSpecializationMapEntry mapEntries[2] = {
          { 0, offsetof(SpecData, alphaMask), sizeof(VkBool32) },
          { 1, offsetof(SpecData, cutoff),    sizeof(float)    }
        };
        static VkSpecializationInfo specInfo{};
        specInfo.mapEntryCount = 2;
        specInfo.pMapEntries = mapEntries;
        specInfo.dataSize = sizeof(specData);
        specInfo.pData = &specData;
        maskConfig.fragSpecInfo = &specInfo;

        maskPipeline = std::make_unique<VkcPipeline>(
            vkcDevice, vertSpv, fragSpv, maskConfig);

        //
        // 3) BLEND (alpha‐blend) pipeline
        //
        PipelineConfigInfo blendConfig{};
        VkcPipeline::defaultPipelineConfigInfo(blendConfig);
        blendConfig.pipelineLayout = pipelineLayout;
        blendConfig.renderPass = renderPass;
        blendConfig.bindingDescriptions = bindings;
        blendConfig.attributeDescriptions = attributes;

        // standard src-alpha / one-minus-src-alpha
        blendConfig.colorBlendAttachment.blendEnable = VK_TRUE;
        blendConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendConfig.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        blendConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendConfig.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        blendConfig.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        blendConfig.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blendConfig.colorBlendInfo.logicOpEnable = VK_FALSE;
        blendConfig.colorBlendInfo.attachmentCount = 1;
        blendConfig.colorBlendInfo.pAttachments = &blendConfig.colorBlendAttachment;

        blendPipeline = std::make_unique<VkcPipeline>(
            vkcDevice, vertSpv, fragSpv, blendConfig);
    }
}