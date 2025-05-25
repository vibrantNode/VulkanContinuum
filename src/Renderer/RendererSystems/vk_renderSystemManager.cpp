#include "vk_renderSystemManager.h"

namespace vkc {

    void RenderSystemManager::initialize(VkcDevice& device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalLayout)
    {
        // Build each system
        systems.push_back(std::make_unique<SimpleRenderSystem>(
            device, renderPass, globalLayout));

        systems.push_back(std::make_unique<PointLightSystem>(
            device, renderPass, globalLayout));

        // … later you can add shadow, postprocess, UI, etc.
    }

    void RenderSystemManager::registerSystems(Scene& scene) {
        for (auto& sys : systems) {
            scene.addRenderSystem(std::move(sys));
        }
        systems.clear();  // ownership moved into scene
    }

}
