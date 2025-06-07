// vk_assetManager.cpp

// Project headers
#include "vk_assetManager.h"


namespace vkc {


    AssetManager::AssetManager(VkcDevice& device) : _device(device) 
    {
    }
    void AssetManager::preloadGlobalAssets() 
    {
        loadModel("quad", PROJECT_ROOT_DIR "/res/models/quad.obj");
        loadModel("flat_vase", PROJECT_ROOT_DIR "/res/models/flat_vase.obj");
        loadModel("smooth_vase", PROJECT_ROOT_DIR "/res/models/smooth_vase.obj");
        loadModel("barrel", PROJECT_ROOT_DIR "/res/models/Barrel_OBJ.obj");
        loadModel("stone_sphere", PROJECT_ROOT_DIR "/res/models/StoneSphere.obj");
        loadModel("living_room", PROJECT_ROOT_DIR "/res/models/InteriorTest.obj");
        loadModel("viking_room", PROJECT_ROOT_DIR "/res/models/VikingRoom.obj");
        //loadModel("gltf_test", PROJECT_ROOT_DIR "/res/models/gltf/test/scene.gltf");

        loadSkyboxModel("cube", PROJECT_ROOT_DIR "/res/models/cube.obj");
       
        loadCubemap("skybox", { {
         PROJECT_ROOT_DIR "/res/textures/SpaceSkybox/right.png",
         PROJECT_ROOT_DIR "/res/textures/SpaceSkybox/left.png",
         PROJECT_ROOT_DIR "/res/textures/SpaceSkybox/top.png",
         PROJECT_ROOT_DIR "/res/textures/SpaceSkybox/bot.png",
         PROJECT_ROOT_DIR "/res/textures/SpaceSkybox/front.png",
         PROJECT_ROOT_DIR "/res/textures/SpaceSkybox/back.png"
         } });

        loadTexture("texture", PROJECT_ROOT_DIR "/res/textures/spaceFloor.jpg");
        loadTexture("texture2", PROJECT_ROOT_DIR "/res/textures/container2.png");
        loadTexture("texture3", PROJECT_ROOT_DIR "/res/textures/stoneWall.jpg");
        loadTexture("texture4", PROJECT_ROOT_DIR "/res/textures/viking_room.png");
     
    }

    std::shared_ptr<VkcTexture> AssetManager::loadCubemap(
        const std::string& name,
        const std::array<std::string, 6>& faces)
    {
        auto it = textureCache.find(name);
        if (it != textureCache.end())
            return it->second;

        auto texture = std::make_shared<VkcTexture>(&_device);
        if (!texture->LoadCubemap(faces)) {
            throw std::runtime_error("Failed to load cubemap: " + name);
        }

        textureCache[name] = texture;
        return texture;
    }



    std::shared_ptr<VkcTexture> AssetManager::loadTexture(const std::string& textureName, const std::string& filepath)
    {
        // Check if texture is already loaded
        auto it = textureCache.find(textureName);
        if (it != textureCache.end())
        {
            return it->second; // Return cached texture
        }

        // Create new texture and load it
        auto texture = std::make_shared<VkcTexture>(&_device);
        if (!texture->LoadFromFile(filepath))
        {
            throw std::runtime_error("Failed to load texture: " + textureName);
        }

        // Cache the loaded texture
        textureCache[textureName] = texture;

        return texture;
    }


    std::shared_ptr<VkcOBJmodel> AssetManager::loadModel(const std::string& modelName, const std::string& filepath)
    {
        // Check if the model is already loaded
        auto it = modelCache.find(modelName);
        if (it != modelCache.end()) {
            return it->second;
        }

        // Load the model and store it in the cache
        auto model = VkcOBJmodel::createModelFromFile(_device, filepath);
        modelCache[modelName] = model;
        return model;
    }

    std::shared_ptr<VkcOBJmodel> AssetManager::loadSkyboxModel(const std::string& modelName, const std::string& filepath)
    {
        auto it = modelCache.find(modelName);
        if (it != modelCache.end()) {
            return it->second;
        }

        auto model = VkcOBJmodel::createModelFromFile(_device, filepath, true);
        modelCache[modelName] = model;
        return model;
    }
  
    std::shared_ptr<VkcOBJmodel> AssetManager::getModel(const std::string& modelName) 
    {
        auto it = modelCache.find(modelName);
        if (it != modelCache.end()) {
            return it->second;
        }
        else {
            throw std::runtime_error("Model not found in cache: " + modelName);
        }
    }
    std::shared_ptr<VkcTexture> AssetManager::getTexture(const std::string& filename) const
    {
        auto it = textureCache.find(filename);
        if (it != textureCache.end()) {
            return it->second;
        }
        else {
            throw std::runtime_error("Texture not found in cache: " + filename);
        }
    }
    std::vector<std::shared_ptr<VkcTexture>> AssetManager::getAllTextures() const {
        std::vector<std::shared_ptr<VkcTexture>> textures;
        for (const auto& [key, texture] : textureCache) {
            textures.push_back(texture);
        }
        return textures;
    }
}