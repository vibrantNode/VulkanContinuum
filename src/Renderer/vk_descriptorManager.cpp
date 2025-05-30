#pragma once
#include "vk_descriptorManager.h"
#include <vulkan/vulkan.h>


namespace vkc {

    DescriptorManager::DescriptorManager(VkcDevice& device)
        : _device(device) {

    }

    DescriptorManager::~DescriptorManager() = default;

    void DescriptorManager::Initialize(const DescriptorConfig& config)
    {
        addGlobalUniforms(config.maxFrames, config.uniformBufferSize);
        if (config.assetManager) {
            addBindlessTextures(*config.assetManager);

            addSkyboxTextures(config.assetManager->getTexture("skybox"));
        }
        buildLayouts();
    }

    DescriptorManager& DescriptorManager::addGlobalUniforms(uint32_t maxFrames, size_t uboSizeBytes) {
        _maxFrames = maxFrames;
        _uboSize = uboSizeBytes;
        return *this;
    }

    DescriptorManager& DescriptorManager::addBindlessTextures(const AssetManager& assetManager) {
        auto allTextures = assetManager.getAllTextures();
        _maxTextures = static_cast<uint32_t>(allTextures.size());

        _imageInfos.clear();
        _imageInfos.reserve(_maxTextures);

        for (auto& tex : allTextures) {
            if (tex->IsCubemap()) continue;
            VkDescriptorImageInfo info{};
            info.sampler = tex->GetSampler();
            info.imageView = tex->GetImageView();
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            _imageInfos.push_back(info);
        }

        return *this;
    }

    DescriptorManager& DescriptorManager::addSkyboxTextures(const std::shared_ptr<VkcTexture>& tex) {
        _skyboxImageInfo = {
            tex->GetSampler(),
            tex->GetImageView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        return *this;
    }

    void DescriptorManager::buildLayouts() {
        // === Create Pool ===
        _pool = VkcDescriptorPool::Builder(_device)
            .setMaxSets(_maxFrames + 2)  // frame sets + 1 texture set
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _maxFrames)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _maxTextures + 1)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
            .build();

        // === Global Uniform Layout ===
        _globalLayout = VkcDescriptorSetLayout::Builder(_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        // === Texture Set Layout ===
        _textureLayout = VkcDescriptorSetLayout::Builder(_device)
            .addBinding(
                0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                _maxTextures,
                VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
            .build();

        // === Skybox Cubemap Layout ===
        _skyboxLayout = VkcDescriptorSetLayout::Builder(_device)
            .addBinding(
                0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                1,
                0)
            .build();

        // === UBO Buffers ===
        _uboBuffers.resize(_maxFrames);
        for (uint32_t i = 0; i < _maxFrames; ++i) {
            _uboBuffers[i] = std::make_unique<VkcBuffer>(
                _device,
                _uboSize,
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            _uboBuffers[i]->map();
        }
    }

    void DescriptorManager::createDescriptorSets() {
        // === Global Uniform Sets ===
        _frameDescriptorSets.resize(_maxFrames);
        for (uint32_t i = 0; i < _maxFrames; ++i) {
            auto bufferInfo = _uboBuffers[i]->descriptorInfo();
            VkcDescriptorWriter(*_globalLayout, *_pool)
                .writeBuffer(0, &bufferInfo)
                .build(_frameDescriptorSets[i]);
        }

        // === Texture Set (Bindless) ===
        VkcDescriptorWriter(*_textureLayout, *_pool)
            .writeImage(0, _imageInfos.data(), static_cast<uint32_t>(_imageInfos.size()))
            .build(_textureDescriptorSet);

        // === Texture set (Skybox) ===
        VkcDescriptorWriter(*_skyboxLayout, *_pool)
            .writeImage(0, &_skyboxImageInfo)
            .build(_skyboxDescriptorSet);

    }


    std::vector<VkDescriptorSet> DescriptorManager::createFrameUniformSets() {
        std::vector<VkDescriptorSet> sets(_maxFrames);
        for (uint32_t i = 0; i < _maxFrames; i++) {
            
            VkDescriptorBufferInfo bufInfo = _uboBuffers[i]->descriptorInfo();
            VkcDescriptorWriter(*_globalLayout, *_pool)
                .writeBuffer(0, &bufInfo)
                .build(sets[i]);
        }
        return sets;
    }

    VkDescriptorSet DescriptorManager::createTextureSet(
        const std::vector<VkDescriptorImageInfo>& infos) {
        VkDescriptorSet set;
        VkcDescriptorWriter(*_textureLayout, *_pool)
            .writeImage(1, infos.data(), static_cast<uint32_t>(infos.size()))
            .build(set);
        return set;

   
    }

    DescriptorLayouts DescriptorManager::getAllLayouts() const
    {
        return DescriptorLayouts{
             _globalLayout ? _globalLayout->getDescriptorSetLayout() : VK_NULL_HANDLE,
             _textureLayout ? _textureLayout->getDescriptorSetLayout() : VK_NULL_HANDLE,
             _skyboxLayout ? _skyboxLayout->getDescriptorSetLayout() : VK_NULL_HANDLE
        };
    }


    VkDescriptorSetLayout DescriptorManager::getGlobalLayout() const {
        return _globalLayout->getDescriptorSetLayout();
    }

    VkDescriptorSetLayout DescriptorManager::getTextureLayout() const {
        return _textureLayout->getDescriptorSetLayout();
    }


    const std::vector<VkDescriptorSet>& DescriptorManager::getGlobalDescriptorSets() const {
        return _frameDescriptorSets;
    }

    VkDescriptorSet DescriptorManager::getTextureDescriptorSet() const {
        return _textureDescriptorSet;
    }
    VkDescriptorSet DescriptorManager::getSkyboxDescriptorSet() const {
        return _skyboxDescriptorSet;
    }

    const std::vector<std::unique_ptr<VkcBuffer>>& DescriptorManager::getUboBuffers() const {
        return _uboBuffers;
    }
} // namespace vkc
