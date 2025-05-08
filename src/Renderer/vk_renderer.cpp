#include "vk_renderer.h"


#include <array>
#include <cassert>
#include <stdexcept>



namespace vkc {
	Renderer::Renderer(VkWindow& window, VkcDevice& device) : vkcWindow{ window }, vkcDevice{ device } {
		recreateSwapchain();
		createCommandBuffers();
	}

	Renderer::~Renderer() {
		freeCommandBuffers();
	}

	void Renderer::recreateSwapchain() {
		auto extent = vkcWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(vkcDevice.device());

		if (vkcSwapChain == nullptr) {
			vkcSwapChain = std::make_unique<VkcSwapChain>(vkcDevice, extent);
		}
		else {
			std::shared_ptr<VkcSwapChain> oldSwapChain = std::move(vkcSwapChain);
			vkcSwapChain = std::make_unique<VkcSwapChain>(vkcDevice, extent, oldSwapChain);
			if (!oldSwapChain->compareSwapFormats(*vkcSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}

		}
	}

	void Renderer::createCommandBuffers() {
		commandBuffers.resize(VkcSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = vkcDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(vkcDevice.device(), &allocInfo, commandBuffers.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

	}

	void Renderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
			vkcDevice.device(),
			vkcDevice.getCommandPool(),
			static_cast<float>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	VkCommandBuffer Renderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginFrame while already in progress");

		auto result = vkcSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapchain();
			return nullptr;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to aquire swap chain image");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		return commandBuffer;
	}

	void Renderer::endFrame() {
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
		auto result = vkcSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			vkcWindow.wasWindowResized()) {
			vkcWindow.resetWindowResizedFlag();
			recreateSwapchain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % VkcSwapChain::MAX_FRAMES_IN_FLIGHT;
	}


	void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vkcSwapChain->getRenderPass();
		renderPassInfo.framebuffer = vkcSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vkcSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 2.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vkcSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(vkcSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, vkcSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}

}
