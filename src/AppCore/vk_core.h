#pragma once
#include <vector>
#include <memory>
#include <deque>
#include <functional>

#include "vma/vk_mem_alloc.h"
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_descriptors.h"
#include "VK_abstraction/vk_gameObject.h"
#include "Renderer/vk_renderer.h"
#include "vk_assetManager.h"
#include "vk_scene.h"

namespace vkc {

	class Application {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		Application();
		~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void RunApp();

	private:
		//std::vector<std::unique_ptr<Scene>> scenes;
		size_t activeSceneIndex = 0;
		//AssetManager assetManager;
	

		// Private Members
		VkWindow _window{ WIDTH, HEIGHT, "Vulkan window" };
		VkcDevice _device{ _window };
		Renderer _renderer{ _window, _device };
		AssetManager _assetManager{ _device };
		Scene _scene{ _device, _assetManager };

		std::unique_ptr<VkcDescriptorPool> globalPool{};
		
		VkcGameObject::Map gameObjects;




	};

}




