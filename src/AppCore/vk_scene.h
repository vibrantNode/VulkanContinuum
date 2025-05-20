// vk_scene.h
#pragma once

// Project headers
#include <VK_abstraction/vk_camera.h>
#include <VK_abstraction/vk_gameObject.h>
#include <VK_abstraction/vk_frameInfo.h>
#include "AppCore/vk_assetManager.h"
#include "VK_abstraction/vk_device.h"
#include "Renderer/RendererSystems/vk_renderSystem.h"
#include "VK_abstraction/vk_camera.h"
#include "Game/vk_player.h"

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

		//VkcCamera& getActiveCamera();


		VkcGameObject* getGameObject(uint32_t id) {
			auto it = gameObjects.find(id);
			return (it != gameObjects.end()) ? &it->second : nullptr;
		}

		// Misc
		void addGameObject(uint32_t id, VkcGameObject obj);
		void removeGameObject(uint32_t id);
		void addPlayer(std::shared_ptr<Player> p);
	private:
		VkcDevice& device;
		VkcCamera activeCamera;
		AssetManager& assetManager;
		std::vector<std::unique_ptr<VkcRenderSystem>> renderSystems;
		std::unordered_map <uint32_t, VkcGameObject> gameObjects;
		std::shared_ptr<Player> player;
		void loadDefaultScene();
		void loadEmptyPlanes();
		void loadLevel1();
	
	};
}