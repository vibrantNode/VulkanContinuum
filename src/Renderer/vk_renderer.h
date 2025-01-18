#pragma once
 
#include "AppCore/vk_window.h"
#include "VK_abstraction/vk_device.h"
#include "VK_abstraction/vk_swapchain.h"



#include <memory>
#include <vector>
#include <cassert>
 
constexpr unsigned int FRAME_OVERLAP = 2;


namespace vkc {
	class Renderer {
	public:
		Renderer(VkWindow &window, VkcDevice& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;


		VkRenderPass getSwapChainRenderPass() const { return vkcSwapChain->getRenderPass(); }
		float getAspectRatio() const { return vkcSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }


		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
	private:
		void createCommandBuffers();

		void freeCommandBuffers();
		void recreateSwapchain();



		VkWindow& vkcWindow;
		VkcDevice& vkcDevice;

		std::unique_ptr<VkcSwapChain> vkcSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex = 0;
		bool isFrameStarted = false;
	
	};
}

