// vk_scene.cpp

// Project headers
#include "vk_scene.h"
#include "AppCore/vk_assetManager.h"
#include "Renderer/RendererSystems/vk_basicRenderSystem.h"
#include "Renderer/RendererSystems/vk_pointLightSystem.h"
#include "Game/Camera/vk_camera.h"

// External
#include <glm/gtc/matrix_transform.hpp>
#include <json.hpp>


// STD
#include <iostream>
#include <fstream>

using json = nlohmann::json;

namespace vkc {

    Scene::Scene(VkcDevice& device, AssetManager& assetManager)
        : device(device), assetManager(assetManager) {
    }

    void Scene::loadSceneData(const std::string& sceneFile)
    {
        std::string path = std::string(PROJECT_ROOT_DIR) + "/res/scenes/" + sceneFile + ".json";
        std::ifstream inFile(path);
        if (!inFile.is_open()) {
            throw std::runtime_error("Could not open scene file: " + path);
        }

        json sceneJson;
        inFile >> sceneJson;
        std::cout << "Loading scene: " << sceneFile << " (" << path << ")\n";

        // Parse game objects
        for (auto& objJson : sceneJson["objects"]) {
            // Special handling for spinning point lights
            if (objJson.value("special", "") == "spinning_lights") {
                int count = objJson.value("count", 6);
                float radius = objJson.value("radius", 4.8f);
                float height = objJson.value("height", -2.5f);
                float intensity = objJson.value("intensity", 15.8f);
                auto colorsJson = objJson["colors"];

                glm::vec3 basePosition = glm::normalize(glm::vec3(-1.f, 0.f, -1.f)) * radius;
                for (int i = 0; i < count; i++) {
                    auto pointLight = VkcGameObject::makePointLight(intensity);
                    auto c = colorsJson[i % colorsJson.size()];
                    pointLight.color = { c[0].get<float>(), c[1].get<float>(), c[2].get<float>() };

                    float angle = (i * glm::two_pi<float>()) / count;
                    glm::mat4 rot = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, -1.f, 0.f));
                    glm::vec3 pos = glm::vec3(rot * glm::vec4(basePosition, 1.f));
                    pos.y = height;
                    pointLight.transform.translation = pos;

                    gameObjects.emplace(pointLight.getId(), std::move(pointLight));
                }
                continue;
            }

            // Game object
            auto go = VkcGameObject::createGameObject();

            // Model
            if (auto it = objJson.find("model"); it != objJson.end()) {
                go.model = assetManager.getModel(it->get<std::string>());
            }

            // Transform
            auto pos = objJson.value("position", std::vector<float>{0.f, 0.f, 0.f});
            auto rot = objJson.value("rotation", std::vector<float>{0.f, 0.f, 0.f});
            auto scl = objJson.value("scale", std::vector<float>{1.f, 1.f, 1.f});
            go.transform.translation = { pos[0], pos[1], pos[2] };
            go.transform.rotation = { rot[0], rot[1], rot[2] };
            go.transform.scale = { scl[0], scl[1], scl[2] };

            // Skybox
            go.isSkybox = objJson.value("isSkybox", false);

            // Name-based texture lookup
            if (auto texIt = objJson.find("textureName"); texIt != objJson.end()) {
                std::string name = texIt->get<std::string>();
                go.texture = assetManager.getTexture(name);
                go.textureIndex = static_cast<int>(assetManager.getTextureIndex(name));
            }
            else {
                go.texture = nullptr;
            }

            // Insert into scene
            if (go.isSkybox) {
                setSkyboxObject(std::move(go));
            }
            else {
                gameObjects.emplace(go.getId(), std::move(go));
            }
        }
    }


    void Scene::update(FrameInfo& frameInfo, GlobalUbo& ubo, float deltaTime) 
    {
    
        // Update render systems
        for (auto& renderSystem : renderSystems) {
            renderSystem->update(frameInfo, ubo);
        }
    }
     
    void Scene::render(FrameInfo& frameInfo) 
    {
        // Render scene
        for (auto& renderSystem : renderSystems) {
            renderSystem->render(frameInfo);
    }
    }

    void Scene::addRenderSystem(std::unique_ptr<VkcRenderSystem> renderSystem) 
    {
        renderSystems.push_back(std::move(renderSystem));
    }

    void Scene::setSkyboxObject(VkcGameObject obj) {
        skyboxId = obj.getId();
        gameObjects.emplace(obj.getId(), std::move(obj));
    }

    std::optional<std::reference_wrapper<VkcGameObject>> Scene::getSkyboxObject() {
        if (!skyboxId) return std::nullopt;
        auto it = gameObjects.find(*skyboxId);
        if (it != gameObjects.end()) {
            return std::ref(it->second);
        }
        return std::nullopt;
    }

    void Scene::addGameObject(uint32_t id, VkcGameObject obj) 
    {
        gameObjects[id] = std::move(obj);
    }

    void Scene::removeGameObject(uint32_t id) 
    {
        gameObjects.erase(id);
    }

    void Scene::addPlayer(std::shared_ptr<Player> p)
    {
        player = std::move(p);
    }
} // namespace vkc