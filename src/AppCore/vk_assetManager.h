#pragma once 
#include "VK_abstraction/vk_model.h"
#include "VK_abstraction/vk_device.h"
#include <unordered_map>

namespace vkc {
    class AssetManager {
    public:
        AssetManager(VkcDevice& device);
        std::shared_ptr<VkcModel> loadModel(const std::string& modelName, const std::string& filepath);
        //std::shared_ptr<VkcTexture> loadTexture(const std::string& filepath);
        //loadShader(const std::string& vertexPath, const std::string& fragmentPath);
        void preloadGlobalAssets();
        // Getters
        std::shared_ptr<VkcModel> getModel(const std::string& modelName);

    private:
        std::unordered_map<std::string, std::shared_ptr<VkcModel>> modelCache;
        //std::unordered_map<std::string, std::shared_ptr<VkcTexture>> textureCache;
        //std::unordered_map<std::string, std::shared_ptr<Shader>> shaderCache;

        VkcDevice& _device;
    };
}