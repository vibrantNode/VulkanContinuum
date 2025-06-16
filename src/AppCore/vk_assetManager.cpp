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
        loadModel("character", PROJECT_ROOT_DIR "/res/models/Square Character/Square Character.obj");
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

        loadTexture("floor", PROJECT_ROOT_DIR "/res/textures/spaceFloor.jpg");
        loadTexture("container", PROJECT_ROOT_DIR "/res/textures/container2.png");
        loadTexture("stoneWall", PROJECT_ROOT_DIR "/res/textures/stoneWall.jpg");
        loadTexture("vikingRoom", PROJECT_ROOT_DIR "/res/textures/viking_room.png");
		loadTexture("stoneFloor01", PROJECT_ROOT_DIR "/res/textures/ktx/stonefloor01_color_rgba.ktx", VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
		loadTexture("fireplaceColorMap", PROJECT_ROOT_DIR "/res/textures/ktx/fireplace_colormap_rgba.ktx", VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
        
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



    std::shared_ptr<VkcTexture> AssetManager::loadTexture(
        const std::string& textureName,
        const std::string& filepath,
        VkFormat format,
        VkImageUsageFlags usageFlags,
        VkImageLayout layout,
        bool forceLinear)
    {
        // 1) cache lookup
        auto it = textureCache.find(textureName);
        if (it != textureCache.end())
            return it->second;

        // 2) create texture instance
        auto texture = std::make_shared<VkcTexture>(&_device);

        // 3) dispatch to STB vs KTX
        //    simple extension check (case‐insensitive)
        auto ext = filepath.substr(filepath.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        bool ok = false;
        if (ext == "ktx") {
            // KTX path needs the queue to copy on
            VkQueue copyQueue = _device.graphicsQueue();
            texture->KTXLoadFromFile(
                filepath,
                format,
                &_device,
                copyQueue,
                usageFlags,
                layout,
                forceLinear
            );
            ok = true;
        }
        else {
            // STB path
            ok = texture->STBLoadFromFile(filepath);
        }

        if (!ok) {
            throw std::runtime_error("AssetManager: failed to load texture " + textureName);
        }

        // 4) cache & return
        textureCache[textureName] = texture;

        size_t index = textureList.size();
		textureList.push_back(texture);
		textureIndexMap[textureName] = index;
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


    std::shared_ptr<VkcTexture> AssetManager::getTexture(size_t index) const
    {
		if (index >= textureList.size()) {
            throw std::out_of_range("Texture index out of range");
		}
		return textureList[index];
    }

    size_t AssetManager::getTextureIndex(const std::string& name) const
    {
		auto it = textureIndexMap.find(name);
        if (it == textureIndexMap.end()) 
			throw std::runtime_error("Texture not found in index map: " + name);
		return it->second;
    }

    const std::vector<std::shared_ptr<VkcTexture>>&
        AssetManager::getAllTextures() const {
        return textureList;
    }
}