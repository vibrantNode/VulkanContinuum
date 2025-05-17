#include "vk_scene.h"
#include "vk_assetManager.h"
#include "Renderer/Types/vk_basicRenderSystem.h"
#include "Renderer/Types/vk_pointLightSystem.h"

// external
#include <glm/gtc/matrix_transform.hpp>

// std
#include <iostream>


namespace vkc {

    Scene::Scene(VkcDevice& device, AssetManager& assetManager)
        : device(device), assetManager(assetManager), camera(glm::vec3(0.0f, 0.0f, 5.0f), 0.0f, 0.0f) {
    }

    void Scene::loadSceneData(const std::string& sceneFile) {
        std::cout << "Loading:" << sceneFile << std::endl;
        if (sceneFile == "DefaultScene") {
            loadDefaultScene();
        }
        else if (sceneFile == "EmptyPlanes") {
            loadEmptyPlanes();
        }
    }

    void Scene::update(FrameInfo& frameInfo, GlobalUbo& ubo, float deltaTime) {
        // Update camera or game object logic here
        for (auto& renderSystem : renderSystems) {
            renderSystem->update(frameInfo, ubo);
        }
    }

    void Scene::render(FrameInfo& frameInfo) {

        for (auto& renderSystem : renderSystems) {
            renderSystem->render(frameInfo);
    }
    }

    void Scene::addRenderSystem(std::unique_ptr<VkcRenderSystem> renderSystem) {
        renderSystems.push_back(std::move(renderSystem));
    }

    void Scene::addGameObject(uint32_t id, VkcGameObject obj) {
        gameObjects[id] = std::move(obj);
    }

    void Scene::removeGameObject(uint32_t id) {
        gameObjects.erase(id);
    }

    void Scene::loadDefaultScene() {

        // Note that we should be figuring out how to load assets once upfront and accessing later in this file ideally 

        auto floorModel = assetManager.getModel("quad");
        auto floor = VkcGameObject::createGameObject();
        floor.model = floorModel;
        floor.transform.translation = { 0.f, .5f, 0.f };
        floor.transform.scale = { 15.f, 15.f, 15.f };
        gameObjects.emplace(floor.getId(), std::move(floor));


        auto flatVaseModel = assetManager.getModel("flat_vase");
        //auto texture = assetManager.loadTexture("textures/flat_vase_diffuse.png");
        auto flatVase = VkcGameObject::createGameObject();
        flatVase.model = flatVaseModel;
        //flatVase.texture = texture;
        flatVase.transform.translation = { -0.5f, 0.5f, 0.0f };
        flatVase.transform.scale = { 3.0f, 1.5f, 3.0f };
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        auto smoothVaseModel = assetManager.getModel("smooth_vase");
        auto smoothVase = VkcGameObject::createGameObject();
        smoothVase.model = smoothVaseModel;
        smoothVase.transform.translation = { .5f, .5f, 0.f };
        smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        auto stoneSphereModel = assetManager.getModel("stone_sphere");
        auto stoneSphere = VkcGameObject::createGameObject();
        stoneSphere.model = stoneSphereModel;
        stoneSphere.transform.translation = { .0f, 0.f, 2.f };
        stoneSphere.transform.scale = { 2.5f, 2.5f, 2.5f };
        gameObjects.emplace(stoneSphere.getId(), std::move(stoneSphere));

        auto woodBarrelModel = assetManager.getModel("barrel");
        auto woodBarrel = VkcGameObject::createGameObject();
        woodBarrel.model = woodBarrelModel;
        woodBarrel.transform.translation = { 1.f, -.4f, -1.5f };
        woodBarrel.transform.scale = { 1.f, 1.f, 1.f };
        gameObjects.emplace(woodBarrel.getId(), std::move(woodBarrel));

        std::vector<glm::vec3> lightColors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        {1.f, 1.f, 1.f}
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = VkcGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            pointLight.pointLight->lightIntensity = .8f;
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.f, -1.f, 0.f });
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }

    void Scene::loadEmptyPlanes() {
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

        for (int i = 0; i < lightColors2.size(); ++i) {
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

 
} // namespace vkc