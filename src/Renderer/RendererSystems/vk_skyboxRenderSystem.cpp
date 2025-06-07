#include "vk_skyboxRenderSystem.h"

// External
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// STD
#include <stdexcept>
#include <cassert>
#include <array>


namespace vkc {

    SkyboxRenderSystem::SkyboxRenderSystem(VkcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout skyboxLayout)
        : vkcDevice{ device },
        skyboxLayout{ skyboxLayout }
    {
        createPipelineLayout(globalSetLayout, skyboxLayout);
        createPipeline(renderPass);
    }

    SkyboxRenderSystem::~SkyboxRenderSystem() {
        vkDestroyPipelineLayout(vkcDevice.device(), pipelineLayout, nullptr);
    }

    void SkyboxRenderSystem::render(FrameInfo& frameInfo) {

        auto skyboxOpt = frameInfo.scene->getSkyboxObject();
        if (!skyboxOpt.has_value()) return;

        auto& skybox = skyboxOpt.value().get();

        // Bind the skybox pipeline
        vkcPipeline->bind(frameInfo.commandBuffer);

        // Descriptor sets: 0 = global (view/proj), 1 = skybox cubemap
        std::array<VkDescriptorSet, 2> descriptorSets = {
            frameInfo.globalDescriptorSet,
            frameInfo.skyboxDescriptorSet
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
        if (skybox.model)
        {
            skybox.model->bind(frameInfo.commandBuffer);
            skybox.model->draw(frameInfo.commandBuffer);
        }
  

    }

    void SkyboxRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout skyboxSetLayout) {
        // Single descriptor set layout for global UBO (view/proj) and cubemap sampler
        std::vector<VkDescriptorSetLayout> layouts = { globalSetLayout, skyboxSetLayout};

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        layoutInfo.pSetLayouts = layouts.data();


        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(vkcDevice.device(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create skybox pipeline layout");
        }
    }

    void SkyboxRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != VK_NULL_HANDLE && "Pipeline layout must be created before pipeline");

        PipelineConfigInfo config{};
        VkcPipeline::defaultSkyboxConfigInfo(config);

        config.bindingDescriptions = VkcOBJmodel::SkyboxVertex::getBindingDescriptions();
        config.attributeDescriptions = VkcOBJmodel::SkyboxVertex::getAttributeDescriptions();

        config.renderPass = renderPass;
        config.pipelineLayout = pipelineLayout;

        std::string vertPath = std::string(PROJECT_ROOT_DIR) + "/res/Shaders/SpirV/skybox.vert.spv";
        std::string fragPath = std::string(PROJECT_ROOT_DIR) + "/res/Shaders/SpirV/skybox.frag.spv";

        vkcPipeline = std::make_unique<VkcPipeline>(
            vkcDevice,
            vertPath.c_str(),
            fragPath.c_str(),
            config
        );
    }

} // namespace vkc