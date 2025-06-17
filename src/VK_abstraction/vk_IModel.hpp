#pragma once
#include <vulkan/vulkan.h>


namespace vkc
{
	struct IModel {
	public:
		virtual ~IModel() = default;
		virtual void bind(VkCommandBuffer commandBuffer) = 0;
		virtual void draw(VkCommandBuffer commandBuffer) = 0;
		
	};
}