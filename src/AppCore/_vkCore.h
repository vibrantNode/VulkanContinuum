#pragma once
#include <vector>
#include <memory>

#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_descriptors.h"
#include "Game/vk_gameObject.h"
#include "Renderer/vk_renderer.h"
#include "vk_assetManager.h"
#include "Game/vk_scene.h"
#include "Game/vk_game.h"
#include "Renderer/RendererSystems/vk_renderSystemManager.h"
#include "Renderer/vk_descriptorManager.h"


namespace vkc {

	class Application {
	public:
		static constexpr int WIDTH = 1440;
		static constexpr int HEIGHT = 810;

		Application();
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void RunApp();

	private:

		// Private Members
		VkWindow _window{ WIDTH, HEIGHT, "Vulkan window" };
		VkcDevice _device{ _window };
		Renderer _renderer{ _window, _device };
		AssetManager _assetManager{ _device };
		Game _game{ _device, _assetManager, _renderer };
		DescriptorManager _descriptorManager{ _device };
		RenderSystemManager _renderSystemManager;
	};


}// namespace vkc



