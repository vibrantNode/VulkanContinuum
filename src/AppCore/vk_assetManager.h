// vk_assetManager.h
#pragma once 

// Project headers
#include "VK_abstraction/vk_model.h"
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_texture.h"

// STD
#include <unordered_map>




namespace vkc {
    class AssetManager {
    public:
        AssetManager(VkcDevice& device);
        std::shared_ptr<VkcModel> loadModel(const std::string& modelName, const std::string& filepath);
        //loadShader(const std::string& vertexPath, const std::string& fragmentPath);
        void preloadGlobalAssets();
        // Getters
        std::shared_ptr<VkcModel> getModel(const std::string& modelName);
        std::shared_ptr<VkcTexture> loadTexture(const std::string& textureName, const std::string& filename);
        std::shared_ptr < VkcTexture> getTexture(const std::string& textureName);
    private:
        std::unordered_map<std::string, std::shared_ptr<VkcModel>> modelCache;
        std::unordered_map<std::string, std::shared_ptr<VkcTexture>> textureCache;
        //std::unordered_map<std::string, std::shared_ptr<Shader>> shaderCache;

        VkcDevice& _device;
    };
}