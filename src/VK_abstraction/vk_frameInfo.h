#pragma once

#include "Game/Camera/vk_camera.h"
#include "Game/vk_gameObject.h"
// lib
#include <vulkan/vulkan.h>


namespace vkc {


#define MAX_LIGHTS 10

	struct Material
	{
		uint32_t textureIndex;
	};

	struct PointLight 
	{
		glm::vec4 position{};  // ignore w
		glm::vec4 color{};     // w is intensity
	};


	struct GlobalUbo
	{
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::mat4 inverseView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

	class Scene;

	struct FrameInfo 
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		const VkcCamera &camera;
		VkDescriptorSet globalDescriptorSet;
		VkDescriptorSet textureDescriptorSet;
		VkDescriptorSet skyboxDescriptorSet;
		VkcGameObject::Map &gameObjects;
		Scene* scene;
	};
}// namespace vkc