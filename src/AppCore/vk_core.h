#pragma once

#include <vector>
#include <deque>
#include <functional>

#include "vk_mem_alloc.h"
#include "VkBootstrap.h"


#include "IMCommon/vk_types.h"
#include <GLFW/glfw3.h>

namespace Core {
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
		static Application& Get();

		// Main
		void Init();

		void RunApp();

		void CleanUp();
		

		// App core members
		bool b_IsInitialized{ false };
		int _frameNumber{ 0 };

		bool freeze_rendering = false;
		bool resize_requested = false;

		// Window members
		GLFWwindow* _window;
		VkExtent2D _windowExtent{ 1700, 900 };


		// Vulkan Members
		VkInstance _instance;
		VkSurfaceKHR _surface;
		VkDebugUtilsMessengerEXT _debug_messenger;
		VkPhysicalDevice _chosenGPU;
		VkDevice _device;

		VkQueue _graphicsQueue;
		uint32_t _graphicsQueueFamily;

		// Memory management
		VmaAllocator _allocator;
		DeletionQueue _mainDeletionQueue;




		// Getters
		inline VkDevice GetDevice() const { return _device; }
		inline VkPhysicalDevice GetPhysicalDevice() const { return _chosenGPU; }
		inline VkExtent2D GetWindowExtent() const { return _windowExtent; }

	private:

		// Private Members

	

		// Initialization
		void InitVulkan();

		void InitSwapChain();

		// Helper functions
		void InitGLFW();
	};

}




