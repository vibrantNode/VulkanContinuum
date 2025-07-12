// vk_assetManager.h
#pragma once

// Project headers
#include "VK_abstraction/vk_obj_model.h"
#include "VK_abstraction/vk_glTFModel.h"
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_texture.h"
#include "VK_abstraction/vk_IModel.hpp"

// STD
#include <unordered_map>
#include <vector>
#include <array>
#include <string>

namespace vkc {

    class AssetManager {
    public:
        AssetManager(VkcDevice& device);

        void preloadGlobalAssets();

        std::shared_ptr<IModel> loadModel(
            const std::string& name,
            const std::string& filepath,
            uint32_t gltfFlags = 0u,
            float scale = 1.0f
        );

        std::shared_ptr<IModel> loadSkyboxModel(
            const std::string& modelName,
            const std::string& filepath
        );

        std::shared_ptr<VkcTexture> loadCubemap(
            const std::string& name,
            const std::array<std::string, 6>& faces
        );

        std::shared_ptr<VkcTexture> loadCubemap(
            const std::string& name,
            const std::string& ktxFilename,
            VkFormat format,
            VkImageUsageFlags usageFlags,
            VkImageLayout initialLayout
        );

        std::shared_ptr<VkcTexture> loadTexture(
            const std::string& name,
            const std::string& path,
            VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
            VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            bool forceLinearTiling = false
        );

        // Getters
        std::shared_ptr<IModel> getModel(const std::string& name) const;

        // name → texture lookup
        std::shared_ptr<VkcTexture> getTexture(const std::string& name) const;

        // index → texture lookup
        std::shared_ptr<VkcTexture> getTexture(size_t index) const;

        // name → index lookup
        size_t getTextureIndex(const std::string& name) const;

        // all textures in load order
        const std::vector<std::shared_ptr<VkcTexture>>& getAllTextures() const;

    private:
        // Models
        std::unordered_map<std::string, std::shared_ptr<IModel>> modelCache;

        // Textures
        std::unordered_map<std::string, std::shared_ptr<VkcTexture>> textures;  // name → texture
        std::unordered_map<std::string, size_t>                      textureIndexMap; // name → index
        std::vector<std::shared_ptr<VkcTexture>>                     textureList;     // index → texture

        VkcDevice& _device;
        VkQueue    _transferQueue;

        // Helpers
        static void registerTextureIfNeeded(
            const std::string& name,
            const std::shared_ptr<VkcTexture>& tex,
            std::unordered_map<std::string, std::shared_ptr<VkcTexture>>& textures,
            std::unordered_map<std::string, size_t>& textureIndexMap,
            std::vector<std::shared_ptr<VkcTexture>>& textureList);
    };

} // namespace vkc
