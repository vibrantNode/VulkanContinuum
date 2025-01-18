#pragma once

#include <vector>
#include <memory>
#include <deque>
#include <functional>

#include "vma/vk_mem_alloc.h"

#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_descriptors.h"

#include "Renderer/vk_renderer.h"

namespace vkc {
	struct DeletionQueue {
		std::deque<std::function<void()>> deleters;

		void push_function(std::function<void()>&& function) {
			deleters.push_back(function);
		}

		void flush() {
			// reverse iterate the deletion queue to exectue all the functions
			for (auto it = deleters.rbegin(); it != deleters.rend(); it++) {
				(*it)();
			}
			deleters.clear();
		}
	};

 
	class Application {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		Application();
		~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;


		void RunApp();


		// Memory management
		VmaAllocator _allocator;
		DeletionQueue _mainDeletionQueue;


	private:

		// Private Members
		VkWindow _window{ WIDTH, HEIGHT, "Vulkan window" };
		VkcDevice _device{ _window };
		Renderer _renderer{ _window, _device };
 

		std::unique_ptr<VkcDescriptorPool> globalPool{};
	

	};

}




