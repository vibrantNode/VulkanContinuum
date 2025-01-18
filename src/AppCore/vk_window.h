#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>


namespace vkc {

	class VkWindow {
	public:
		VkWindow(int w, int h, std::string name);
		~VkWindow();

		VkWindow(const VkWindow&) = delete;
		VkWindow& operator=(const VkWindow&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
		GLFWwindow* getGLFWwindow() const { return window; }


		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	private:

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();

		int width;
		int height;
		bool framebufferResized = false;


		std::string windowName;
		GLFWwindow* window;
	};



}