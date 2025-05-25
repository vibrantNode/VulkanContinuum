#pragma once
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_frameInfo.h"
#include "Renderer/vk_renderer.h"
#include "AppCore/vk_assetManager.h"
#include "Game/vk_scene.h"
#include "Game/vk_player.h"
#include "Game/Camera/vk_camera.h"


namespace vkc
{
	class Game
	{
	public:
		Game(VkcDevice& device, AssetManager& assetManager, Renderer& renderer);
		void Init(GLFWwindow* window);
		void Update(FrameInfo& frameInfo, GlobalUbo& ubo, float deltaTime);
		void Render(FrameInfo& frameInfo);
		
		VkcGameObject::Map& getGameObjects();
		const VkcCamera& getPlayerCamera() const;

		Scene& getScene() { return _scene; }
	private:
		Scene _scene;
		std::shared_ptr<Player> _player;
		VkcCamera _camera;
};
}