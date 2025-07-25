#pragma once
#include "VK_abstraction/vk_descriptors.h"
#include "VK_abstraction/vk_buffer.h"
#include "Game/vk_scene.h"
#include "AppCore/vk_assetManager.h"

#include <vector>

namespace vkc {
    struct DescriptorLayouts {
        VkDescriptorSetLayout globalLayout;
        VkDescriptorSetLayout textureLayout;
        VkDescriptorSetLayout skyboxLayout;
        VkDescriptorSetLayout descriptorSetLayoutImage;
    };

    struct DescriptorConfig {
        uint32_t maxFrames;
        size_t uniformBufferSize;
        const AssetManager* assetManager; 
		Scene* scene; // Optional, for skybox textures
    };

    class DescriptorManager {
    public:
        DescriptorManager(VkcDevice& device);
        ~DescriptorManager();

        void Initialize(const DescriptorConfig& config);

        DescriptorManager& addGlobalUniforms(uint32_t maxFrames, size_t uboSizeBytes);
        DescriptorManager& addBindlessTextures(const AssetManager& assetManager);
        DescriptorManager& addSkyboxTextures(const std::shared_ptr<VkcTexture>& tex);

        void buildLayouts();

        void createDescriptorSets();

        std::vector<VkDescriptorSet> createFrameUniformSets();
        VkDescriptorSet              createTextureSet(
            const std::vector<VkDescriptorImageInfo>& infos);

        // Getters
        DescriptorLayouts getAllLayouts() const;
        VkDescriptorSetLayout getGlobalLayout() const;
        VkDescriptorSetLayout getTextureLayout() const;

        const std::vector<VkDescriptorSet>& getGlobalDescriptorSets() const;

        VkDescriptorSet getTextureDescriptorSet() const;
        VkDescriptorSet getSkyboxDescriptorSet() const;

        const std::vector<std::unique_ptr<VkcBuffer>>& getUboBuffers() const;

    private:
        VkcDevice&                                           _device;
        std::unique_ptr<VkcDescriptorPool>                   _pool;
        std::unique_ptr<VkcDescriptorSetLayout>              _globalLayout;
        std::unique_ptr<VkcDescriptorSetLayout>              _textureLayout;
        std::unique_ptr<VkcDescriptorSetLayout>              _skyboxLayout;

        size_t                                               _uboSize           = 0;
        uint32_t                                             _maxFrames         = 0;
        uint32_t                                             _maxTextures       = 1000;

        // Owned resources
        std::vector<std::unique_ptr<VkcBuffer>>                         _uboBuffers;
        std::vector<VkDescriptorSet>                           _frameDescriptorSets;
        VkDescriptorSet                     _textureDescriptorSet{ VK_NULL_HANDLE };
        std::vector<VkDescriptorImageInfo>                              _imageInfos;
        
        VkDescriptorSet                                        _skyboxDescriptorSet;
        VkDescriptorImageInfo                                      _skyboxImageInfo;
    };

} // namespace vkc
