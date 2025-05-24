// vk_scene.cpp
// 
// 
// Project headers
#include "vk_scene.h"
#include "vk_assetManager.h"
#include "Renderer/RendererSystems/vk_basicRenderSystem.h"
#include "Renderer/RendererSystems/vk_pointLightSystem.h"
#include "VK_abstraction/vk_camera.h"
// External
#include <glm/gtc/matrix_transform.hpp>

// std
#include <iostream>

#include <fstream>



namespace vkc {

    Scene::Scene(VkcDevice& device, AssetManager& assetManager)
        : device(device), assetManager(assetManager) {
    }

    void Scene::loadSceneData(const std::string& sceneFile) 
    {
        std::cout << "Loading: " << sceneFile << std::endl;
        if (sceneFile == "DefaultScene") {
            loadDefaultScene();
        }
        else if (sceneFile == "EmptyPlanes") {
            loadEmptyPlanes();
        }
        else if (sceneFile == "Level1") {
            loadLevel1();
        }
    }

    void Scene::update(FrameInfo& frameInfo, GlobalUbo& ubo, float deltaTime) 
    {
        // Update camera or game object logic 
        for (auto& renderSystem : renderSystems) {
            renderSystem->update(frameInfo, ubo);
        }
    }

    void Scene::render(FrameInfo& frameInfo) 
    {

        for (auto& renderSystem : renderSystems) {
            renderSystem->render(frameInfo);
    }
    }

    void Scene::addRenderSystem(std::unique_ptr<VkcRenderSystem> renderSystem) 
    {
        renderSystems.push_back(std::move(renderSystem));
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
        //addGameObject(player->getId(), *player;
    }

    void Scene::loadDefaultScene()
    {

        auto floorModel = assetManager.getModel("quad");
        auto floor = VkcGameObject::createGameObject();
        floor.model = floorModel;
        floor.transform.translation = { 0.f, .5f, 0.f };
        floor.transform.scale = { 9.f, 9.f, 9.f };
        floor.textureIndex = 0;
        gameObjects.emplace(floor.getId(), std::move(floor));

        auto flatVaseModel = assetManager.getModel("flat_vase");
        auto flatVase = VkcGameObject::createGameObject();
        flatVase.model = flatVaseModel;
        flatVase.transform.translation = { -0.5f, 0.5f, 0.0f };
        flatVase.transform.scale = { 3.0f, 1.5f, 3.0f };
        flatVase.textureIndex = 0;
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        auto smoothVaseModel = assetManager.getModel("smooth_vase");
        auto smoothVase = VkcGameObject::createGameObject();
        smoothVase.model = smoothVaseModel;
        smoothVase.transform.translation = { .5f, .5f, 0.f };
        smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        auto woodBarrelModel = assetManager.getModel("barrel");
        auto woodBarrel = VkcGameObject::createGameObject();
        woodBarrel.model = woodBarrelModel;
        woodBarrel.transform.translation = { 1.f, -.4f, -1.5f };
        woodBarrel.transform.scale = { 1.f, 1.f, 1.f };
        woodBarrel.textureIndex = 1;
        gameObjects.emplace(woodBarrel.getId(), std::move(woodBarrel));

        auto stoneSphereModel = assetManager.getModel("stone_sphere");
        auto stoneSphere = VkcGameObject::createGameObject();
        stoneSphere.model = stoneSphereModel;
        stoneSphere.transform.translation = { 1.0f, 0.f, 2.f };
        stoneSphere.transform.scale = { .5f, .5f, .5f };
        stoneSphere.textureIndex = 2;
        gameObjects.emplace(stoneSphere.getId(), std::move(stoneSphere));
        
 
        std::vector<glm::vec3> lightColors
        {
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        {1.f, 1.f, 1.f}
        };

        for (int i = 0; i < lightColors.size(); i++) 
        {
            auto pointLight = VkcGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            pointLight.pointLight->lightIntensity = 3.8f;
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.f, -1.f, 0.f });
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }

    void Scene::loadEmptyPlanes() 
    {
        auto planeModel = assetManager.getModel("quad");
        auto plane = VkcGameObject::createGameObject();
        plane.model = planeModel;
        plane.transform.translation = { 0.f, .5f, 0.f };
        plane.transform.scale = { 15.f, 15.f, 15.f };
        gameObjects.emplace(plane.getId(), std::move(plane));

        // Add colored point lights
        std::vector<glm::vec3> lightColors2{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f}
        };

        for (int i = 0; i < lightColors2.size(); ++i) 
        {
            auto pointLights = VkcGameObject::makePointLight(0.2f);
            pointLights.color = lightColors2[i];
            pointLights.pointLight->lightIntensity = 1.0f;
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors2.size(),
                { 0.f, -1.f, 0.f });
            // Position lights above each plane
            pointLights.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));

            gameObjects.emplace(pointLights.getId(), std::move(pointLights));
        }
    }

    void Scene::loadLevel1()
    {

        auto lvl1FlatVaseModel = assetManager.getModel("flat_vase");
        auto lvl1FlatVase = VkcGameObject::createGameObject();
        lvl1FlatVase.model = lvl1FlatVaseModel;
        lvl1FlatVase.transform.translation = { -0.5f, 0.5f, 0.0f };
        lvl1FlatVase.transform.scale = { 3.0f, 1.5f, 3.0f };
        lvl1FlatVase.textureIndex = 2;
        gameObjects.emplace(lvl1FlatVase.getId(), std::move(lvl1FlatVase));

        auto lvl1SmoothVaseModel = assetManager.getModel("smooth_vase");
        auto lvl1SmoothVase = VkcGameObject::createGameObject();
        lvl1SmoothVase.model = lvl1SmoothVaseModel;
        lvl1SmoothVase.transform.translation = { .5f, .5f, 0.f };
        lvl1SmoothVase.transform.scale = { 3.f, 1.5f, 3.f };
        lvl1SmoothVase.textureIndex = 2;
        gameObjects.emplace(lvl1SmoothVase.getId(), std::move(lvl1SmoothVase));

        auto lvl1WoodBarrelModel = assetManager.getModel("barrel");
        auto lvl1WoodBarrel = VkcGameObject::createGameObject();
        lvl1WoodBarrel.model = lvl1WoodBarrelModel;
        lvl1WoodBarrel.transform.translation = { 1.f, -.4f, -1.5f };
        lvl1WoodBarrel.transform.scale = { 1.f, 1.f, 1.f };
        lvl1WoodBarrel.textureIndex = 1;
        gameObjects.emplace(lvl1WoodBarrel.getId(), std::move(lvl1WoodBarrel));

        auto lvl1StoneSphereModel = assetManager.getModel("stone_sphere");
        auto lvl1StoneSphere = VkcGameObject::createGameObject();
        lvl1StoneSphere.model = lvl1StoneSphereModel;
        lvl1StoneSphere.transform.translation = { 0.f, 1.f, 0.f };
        lvl1StoneSphere.transform.scale = { 10.5f, .5f, 10.5f };
        lvl1StoneSphere.textureIndex = 0;
        gameObjects.emplace(lvl1StoneSphere.getId(), std::move(lvl1StoneSphere));

        // Add colored point lights
        std::vector<glm::vec3> lvl1LightColor{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f}
        };

        for (int i = 0; i < lvl1LightColor.size(); ++i)
        {
            auto pointLights = VkcGameObject::makePointLight(0.2f);
            pointLights.color = lvl1LightColor[i];
            pointLights.pointLight->lightIntensity = 6.0f;
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lvl1LightColor.size(),
                { 0.f, -1.f, 0.f });
            // Position lights above each plane
            pointLights.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));

            gameObjects.emplace(pointLights.getId(), std::move(pointLights));
        }
    }

 
} // namespace vkc