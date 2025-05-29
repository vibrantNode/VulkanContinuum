#include "vk_renderSystemManager.h"
#include "vk_skyboxRenderSystem.h"

namespace vkc {

    void RenderSystemManager::initialize(VkcDevice& device,
        VkRenderPass renderPass,
        const DescriptorLayouts& layouts,
        DescriptorManager& descriptorManager,
        AssetManager& assetManager)
    {
        _descriptorManager = &descriptorManager;

        systems.push_back(std::make_unique<SimpleRenderSystem>(
            device,
            renderPass,
            layouts.globalLayout,
            layouts.textureLayout));

        systems.push_back(std::make_unique<PointLightSystem>(
            device,
            renderPass,
            layouts.globalLayout));

 /*       auto skyboxModel = assetManager.getModel("cube");
        if (!skyboxModel) {
            throw std::runtime_error("Skybox model 'cube' not found! Did you forget to load it?");
        }
        systems.push_back(std::make_unique<SkyboxRenderSystem>(
            device,
            renderPass,
            layouts.globalLayout,
            skyboxModel,
            layouts.skyboxLayout
        ));*/

    }

    void RenderSystemManager::registerSystems(Scene& scene) {
        for (auto& sys : systems) {
            scene.addRenderSystem(std::move(sys));
        }
        systems.clear(); 
    }

} // namespace vkc