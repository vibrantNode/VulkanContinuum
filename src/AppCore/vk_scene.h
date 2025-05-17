#pragma once
#include <VK_abstraction/vk_camera.h>
#include <VK_abstraction/vk_gameObject.h>
#include <VK_abstraction/vk_frameInfo.h>
#include "AppCore/vk_assetManager.h"
#include "VK_abstraction/vk_device.h"
#include "Renderer/Types/vk_renderSystem.h"

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
	private:
		VkcDevice& device;
		VkcCamera camera;
		AssetManager& assetManager;
		std::vector<std::unique_ptr<VkcRenderSystem>> renderSystems;
		std::unordered_map <uint32_t, VkcGameObject> gameObjects;
	
		void loadDefaultScene();
		void loadEmptyPlanes();
	
	};
}