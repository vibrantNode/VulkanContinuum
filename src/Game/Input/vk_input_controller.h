#pragma once
#include <GLFW/glfw3.h>
#include "VK_abstraction/vk_gameObject.h"


namespace vkc
{
	class InputController {
	public:
		virtual void handleInput(GLFWwindow* window, float deltaTime, VkcGameObject& object) = 0;
		virtual ~InputController() = default;
	};
}