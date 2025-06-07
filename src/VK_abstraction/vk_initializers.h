#pragma once
#include "Common/vkc_types.h"

namespace vkc
{

    namespace vkinit
    {


        //> init_cmd
        VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
        VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1);
        //< init_cmd

        VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);
        VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd);

        VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);

        VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);

        VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo,
            VkSemaphoreSubmitInfo* waitSemaphoreInfo);
        VkPresentInfoKHR present_info();

        VkRenderingAttachmentInfo attachment_info(VkImageView view, VkClearValue* clear, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/);

        VkRenderingAttachmentInfo depth_attachment_info(VkImageView view,
            VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/);

        VkRenderingInfo rendering_info(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment,
            VkRenderingAttachmentInfo* depthAttachment);

        VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask);

        VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
        VkDescriptorSetLayoutBinding descriptorset_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags,
            uint32_t binding);
        VkDescriptorSetLayoutCreateInfo descriptorset_layout_create_info(VkDescriptorSetLayoutBinding* bindings,
            uint32_t bindingCount);
        VkWriteDescriptorSet write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet,
            VkDescriptorImageInfo* imageInfo, uint32_t binding);
        VkWriteDescriptorSet write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet,
            VkDescriptorBufferInfo* bufferInfo, uint32_t binding);
        VkDescriptorBufferInfo buffer_info(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

        VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
        VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
        VkPipelineLayoutCreateInfo pipeline_layout_create_info();
        VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage,
            VkShaderModule shaderModule,
            const char* entry = "main");


        // Sacha willems:

        inline VkCommandBufferBeginInfo commandBufferBeginInfo()
        {
            VkCommandBufferBeginInfo cmdBufferBeginInfo{};
            cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            return cmdBufferBeginInfo;
        }

        inline VkMemoryAllocateInfo memoryAllocateInfo()
        {
            VkMemoryAllocateInfo memAllocInfo{};
            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            return memAllocInfo;
        }

        inline VkBufferCreateInfo bufferCreateInfo()
        {
            VkBufferCreateInfo bufCreateInfo{};
            bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            return bufCreateInfo;
        }
        inline VkBufferCreateInfo bufferCreateInfo(
            VkBufferUsageFlags usage,
            VkDeviceSize size)
        {
            VkBufferCreateInfo bufCreateInfo{};
            bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufCreateInfo.usage = usage;
            bufCreateInfo.size = size;
            return bufCreateInfo;
        }

        inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(
            VkCommandPool commandPool,
            VkCommandBufferLevel level,
            uint32_t bufferCount)
        {
            VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.commandPool = commandPool;
            commandBufferAllocateInfo.level = level;
            commandBufferAllocateInfo.commandBufferCount = bufferCount;
            return commandBufferAllocateInfo;
        }

        inline VkImageCreateInfo imageCreateInfo()
        {
            VkImageCreateInfo imageCreateInfo{};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            return imageCreateInfo;
        }

        /** @brief Initialize an image memory barrier with no image transfer ownership */
        inline VkImageMemoryBarrier imageMemoryBarrier()
        {
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            return imageMemoryBarrier;
        }


    }
}