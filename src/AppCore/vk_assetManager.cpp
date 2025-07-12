// vk_assetManager.cpp

// Project headers
#include "vk_assetManager.h"


namespace vkc {


	AssetManager::AssetManager(VkcDevice& device) : _device(device), _transferQueue(device.graphicsQueue())
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
        //const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;

        loadModel("helmet", PROJECT_ROOT_DIR "/res/models/gltf/FlightHelmet/glTF/FlightHelmet.gltf");
        loadModel("cerberus", PROJECT_ROOT_DIR "/res/models/gltf/cerberus/cerberus.gltf");
        loadModel("dragon", PROJECT_ROOT_DIR "/res/models/gltf/chinesedragon.gltf");
        loadModel("sponza", PROJECT_ROOT_DIR "/res/models/gltf/sponza/sponza.gltf");

        loadCubemap("environmentHDR",
            PROJECT_ROOT_DIR "/res/textures/ktx/hdr/gcanyon_cube.ktx",
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
        
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
		loadTexture("rock_array", PROJECT_ROOT_DIR "/res/textures/ktx/particle_gradient_rgba.ktx", VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
		loadTexture("metal_plate", PROJECT_ROOT_DIR "/res/textures/ktx/metalplate01_rgba.ktx", VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
		// Cerberus
        loadTexture("cerberus_a", PROJECT_ROOT_DIR "/res/models/gltf/cerberus/albedo.ktx", VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
        loadTexture("cerberus_n", PROJECT_ROOT_DIR "/res/models/gltf/cerberus/normal.ktx", VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
        loadTexture("cerberus_ao", PROJECT_ROOT_DIR "/res/models/gltf/cerberus/ao.ktx", VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
        loadTexture("cerberus_m", PROJECT_ROOT_DIR "/res/models/gltf/cerberus/metallic.ktx", VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
        loadTexture("cerberus_r", PROJECT_ROOT_DIR "/res/models/gltf/cerberus/roughness.ktx", VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
        
    }
    std::shared_ptr<IModel> AssetManager::loadModel(const std::string& name,
        const std::string& filepath,
        uint32_t gltfFlags,
        float scale) {
        if (auto it = modelCache.find(name); it != modelCache.end())
            return it->second;

        // Determine extension
        auto ext = filepath.substr(filepath.find_last_of('.') + 1);
        for (auto& c : ext) c = char(::tolower(c));

        std::shared_ptr<IModel> model;
        if (ext == "obj") {
            model = VkcOBJmodel::createModelFromFile(_device, filepath);
        }
        else if (ext == "gltf" || ext == "glb") {
            auto gltf = std::make_shared<vkglTF::Model>();
            gltf->loadFromFile(filepath, &_device, _device.graphicsQueue(), gltfFlags, scale);
            model = gltf;
        }
        else {
            throw std::runtime_error("Unsupported format: " + ext);
        }

        modelCache[name] = model;
        return model;
    }


    std::shared_ptr<VkcTexture> AssetManager::loadCubemap(
        const std::string& name,
        const std::array<std::string, 6>& faces)
    {
        if (auto it = textures.find(name); it != textures.end())
            return it->second;

        auto tex = std::make_shared<VkcTexture>(&_device);
        if (!tex->LoadCubemap(faces)) {
            throw std::runtime_error("Failed to load cubemap: " + name);
        }

        registerTextureIfNeeded(name, tex, textures, textureIndexMap, textureList);
        return tex;
    }
    std::shared_ptr<VkcTexture> AssetManager::loadCubemap(
        const std::string& name,
        const std::string& ktxFilename,
        VkFormat format,
        VkImageUsageFlags usageFlags,
        VkImageLayout initialLayout)
    {
        if (auto it = textures.find(name); it != textures.end())
            return it->second;

        auto tex = std::make_shared<VkcTexture>();
        tex->device = &_device;
        try {
            tex->KtxLoadCubemapFromFile(
                ktxFilename,
                format,
                &_device,
                _transferQueue,
                usageFlags,
                initialLayout
            );
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Failed to load HDR cubemap '" + name + "': " + e.what());
        }

        registerTextureIfNeeded(name, tex, textures, textureIndexMap, textureList);
        return tex;
    }


    std::shared_ptr<VkcTexture> AssetManager::loadTexture(
        const std::string& name,
        const std::string& filepath,
        VkFormat format,
        VkImageUsageFlags usageFlags,
        VkImageLayout layout,
        bool forceLinear)
    {
        if (auto it = textures.find(name); it != textures.end())
            return it->second;

        auto tex = std::make_shared<VkcTexture>(&_device);

        // Determine extension
        auto ext = filepath.substr(filepath.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        bool ok = false;
        if (ext == "ktx") {
            tex->KTXLoadFromFile(
                filepath,
                format,
                &_device,
                _transferQueue,
                usageFlags,
                layout,
                forceLinear
            );
            ok = true;
        }
        else {
            ok = tex->STBLoadFromFile(filepath);
        }

        if (!ok) {
            throw std::runtime_error("AssetManager: failed to load texture " + name);
        }

        registerTextureIfNeeded(name, tex, textures, textureIndexMap, textureList);
        return tex;
    }

    std::shared_ptr<IModel> AssetManager::loadSkyboxModel(const std::string& modelName, const std::string& filepath)
    {
        auto it = modelCache.find(modelName);
        if (it != modelCache.end()) {
            return it->second;
        }

        auto model = VkcOBJmodel::createModelFromFile(_device, filepath, true);
        modelCache[modelName] = model;
        return model;
    }

    // Getters
  //------------------------------------------------------------------------------
    std::shared_ptr<IModel> AssetManager::getModel(const std::string& name) const
    {
        auto it = modelCache.find(name);
        if (it == modelCache.end())
            throw std::runtime_error("Model not found: " + name);
        return it->second;
    }

    //------------------------------------------------------------------------------
    std::shared_ptr<VkcTexture> AssetManager::getTexture(const std::string& name) const
    {
        auto it = textures.find(name);
        if (it == textures.end())
            throw std::runtime_error("Texture not found: " + name);
        return it->second;
    }

    //------------------------------------------------------------------------------
    std::shared_ptr<VkcTexture> AssetManager::getTexture(size_t index) const
    {
        if (index >= textureList.size())
            throw std::runtime_error("Texture index out of range: " + std::to_string(index));
        return textureList[index];
    }

    //------------------------------------------------------------------------------
    size_t AssetManager::getTextureIndex(const std::string& name) const
    {
        auto it = textureIndexMap.find(name);
        if (it == textureIndexMap.end())
            throw std::runtime_error("Texture not found in index map: " + name);
        return it->second;
    }

    //------------------------------------------------------------------------------
    const std::vector<std::shared_ptr<VkcTexture>>& AssetManager::getAllTextures() const
    {
        return textureList;
    }

    // Helpers


    void AssetManager::registerTextureIfNeeded(
        const std::string& name,
        const std::shared_ptr<VkcTexture>& tex,
        std::unordered_map<std::string, std::shared_ptr<VkcTexture>>& textures,
        std::unordered_map<std::string, size_t>& textureIndexMap,
        std::vector<std::shared_ptr<VkcTexture>>& textureList)
    {
        if (textures.find(name) == textures.end()) {
            textures[name] = tex;
            textureList.push_back(tex);
            textureIndexMap[name] = textureList.size() - 1;
        }
    }
}