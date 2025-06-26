#include "vk_game.h"

namespace vkc
{
	Game::Game(VkcDevice& device, AssetManager& assetManager, Renderer& renderer)
		: _scene(device, assetManager)
	{
	}

	void Game::Init(GLFWwindow* window)
	{
		_scene.loadSceneData("defaultScene");

		_player = std::make_shared<Player>(window);
		_player->Init();

		_scene.addPlayer(_player);	
	}

	void Game::Update(FrameInfo& frameInfo, GlobalUbo& ubo, float dt)
	{
		_player->Update(dt);
		_camera = _player->getCamera();

		ubo.view = _camera.getView();
		ubo.projection = _camera.getProjection();
		ubo.inverseView = _camera.getInverseView();
		ubo.viewPos = glm::vec4(_camera.getPosition(), 1.0f);
	
		_scene.update(frameInfo, ubo, dt);
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
