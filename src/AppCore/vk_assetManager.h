// vk_assetManager.h
#pragma once 

// Project headers
#include "VK_abstraction/vk_obj_model.h"
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_texture.h"

// STD
#include <unordered_map>
#include <array>



namespace vkc {

    class AssetManager {
    public:
        AssetManager(VkcDevice& device);
       
        void preloadGlobalAssets();


        std::shared_ptr<VkcOBJmodel> loadModel(const std::string& modelName, const std::string& filepath);
        std::shared_ptr<VkcOBJmodel> loadSkyboxModel(const std::string& modelName, const std::string& filepath);
        std::shared_ptr<VkcTexture> loadCubemap(const std::string& name, const std::array<std::string, 6>& faces);
        std::shared_ptr<VkcTexture> loadTexture(
            const std::string& name,
            const std::string& path,
            VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
            VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            bool forceLinearTiling = false
        );

        // Getters
        std::shared_ptr<VkcOBJmodel> getModel(const std::string& modelName);


        // name → texture lookup
        std::shared_ptr<VkcTexture> getTexture(const std::string& name) const;

        // index → texture lookup
        std::shared_ptr<VkcTexture> getTexture(size_t index) const;

        // name → index lookup
        size_t getTextureIndex(const std::string& name) const;

        // all textures in load order
        const std::vector<std::shared_ptr<VkcTexture>>& getAllTextures() const;


    private:
        std::unordered_map<std::string, std::shared_ptr<VkcOBJmodel>> modelCache;

        std::unordered_map<std::string, std::shared_ptr<VkcTexture>> textureCache;   // fast name→texture
        std::vector   <std::shared_ptr<VkcTexture>>                   textureList;    // stable index
        std::unordered_map<std::string, size_t>                       textureIndexMap;// name→index

        VkcDevice& _device;

    };
}