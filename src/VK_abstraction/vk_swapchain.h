// vk_swapchain.h
#pragma once

// Project headers
#include "vk_device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// STD
#include <string>
#include <vector>
#include <memory>


namespace vkc
{

    class VkcSwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        VkcSwapChain(VkcDevice& deviceRef, VkExtent2D windowExtent);
        VkcSwapChain(VkcDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr <VkcSwapChain>previous);
        ~VkcSwapChain();

        VkcSwapChain(const VkcSwapChain&) = delete;
        VkcSwapChain& operator=(const VkcSwapChain&) = delete;

        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        VkRenderPass getRenderPass() { return renderPass; }
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }
        size_t imageCount() { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }

        float extentAspectRatio() {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }
        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t* imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

        bool compareSwapFormats(const VkcSwapChain& swapChain) const {
            return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
                swapChain.swapChainImageFormat == swapChainImageFormat;
        }

    private:
        void init();
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkFormat swapChainImageFormat;
        VkFormat swapChainDepthFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemory;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        VkcDevice& device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;
        std::shared_ptr<VkcSwapChain> oldSwapChain;

        size_t                  swapChainImageCount = 0;
        std::vector<VkSemaphore> imageAvailableSemaphores; // one per image
        std::vector<VkSemaphore> renderFinishedSemaphores; // one per image
        std::vector<VkFence>     inFlightFences;           // one per CPU-frame
        std::vector<VkFence>     imagesInFlight;           // last-used fence per image
        size_t                  currentFrame = 0;
    };

}// namespace vkc
