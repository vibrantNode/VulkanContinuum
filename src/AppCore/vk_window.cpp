#include "vk_window.h"
#include <GLFW/glfw3.h>
#include <stdexcept>



namespace vkc {
	VkWindow::VkWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	VkWindow::~VkWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	void VkWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void VkWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}

	void VkWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto Window = reinterpret_cast<VkWindow*>(glfwGetWindowUserPointer(window));
		Window->framebufferResized = true;
		Window->width = width;
		Window->height = height;
	}
}