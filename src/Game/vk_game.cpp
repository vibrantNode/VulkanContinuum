#include "vk_game.h"

namespace vkc
{
	Game::Game(VkcDevice& device, AssetManager& assetManager, Renderer& renderer)
		: _scene(device, assetManager)
	{
	}

	void Game::Init(GLFWwindow* window)
	{
		_scene.loadSceneData("DefaultScene");

		_player = std::make_shared<Player>(window);
		_player->Init();

		_scene.addPlayer(_player);	
	}

	void Game::Update(FrameInfo& frameInfo, GlobalUbo& ubo, float deltaTime)
	{
		_player->Update(deltaTime);
		_camera = _player->getCamera();
		ubo.view = _camera.getView();
		ubo.projection = _camera.getProjection();
		ubo.inverseView = _camera.getInverseView();

		_scene.update(frameInfo, ubo, deltaTime);
	}
	void Game::Render(FrameInfo& frameInfo)
	{
		_scene.render(frameInfo);
	}

	const VkcCamera& Game::getPlayerCamera() const {
		return _camera;
	}
	
	VkcGameObject::Map& Game::getGameObjects()
	{
		return _scene.getGameObjects();
	}
}
