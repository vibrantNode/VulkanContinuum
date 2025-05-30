// vk_scene.h
#pragma once

// Project headers
#include "Game/Camera/vk_camera.h"
#include "Game/vk_gameObject.h"
#include <VK_abstraction/vk_frameInfo.h>
#include "AppCore/vk_assetManager.h"
#include "VK_abstraction/vk_device.h"
#include "Renderer/RendererSystems/vk_renderSystem.h"
#include "Game/vk_player.h"


#include <optional>

namespace vkc {
	class Scene {

	public:
		Scene(VkcDevice& device, AssetManager& assetManager);
		void addRenderSystem(std::unique_ptr<VkcRenderSystem> renderSystem);
		void loadSceneData(const std::string& sceneFile);
		void render(FrameInfo& frameInfo);
		void update(FrameInfo& frameInfo, GlobalUbo& ubo, float deltaTime);
		
		// Getters
		std::unordered_map<uint32_t, VkcGameObject>& getGameObjects() { return gameObjects; }

		VkcGameObject* getGameObject(uint32_t id) {
			auto it = gameObjects.find(id);
			return (it != gameObjects.end()) ? &it->second : nullptr;
		}

		// Misc
		void addGameObject(uint32_t id, VkcGameObject obj);
		void removeGameObject(uint32_t id);
		void addPlayer(std::shared_ptr<Player> player);
		void setSkyboxObject(VkcGameObject obj);
		std::optional<std::reference_wrapper<VkcGameObject>> getSkyboxObject();
	private:
		VkcDevice& device;
		VkcCamera activeCamera;
		AssetManager& assetManager;
		std::vector<std::unique_ptr<VkcRenderSystem>> renderSystems;
		std::unordered_map <uint32_t, VkcGameObject> gameObjects;
		std::optional<uint32_t> skyboxId;

		std::shared_ptr<Player> player;


		void loadDefaultScene();
		void loadEmptyPlanes();
		void loadLevel1();
	
	};
}