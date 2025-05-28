#pragma once

#include "vk_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>


namespace vkc 
{
    class VkcDescriptorSetLayout 
    {
    public:
        class Builder 
        {
        public:
            Builder(VkcDevice& vkcDevice) : vkcDevice{ vkcDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count,
                VkDescriptorBindingFlags flags);

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags);

            std::unique_ptr<VkcDescriptorSetLayout> build() const;

        private:
            VkcDevice& vkcDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
            std::unordered_map<uint32_t, VkDescriptorBindingFlags> bindingFlags{};
        };

        VkcDescriptorSetLayout(
            VkcDevice& vkcDevice,
            const std::vector<VkDescriptorSetLayoutBinding>& bindingsVec,
            VkDescriptorSetLayout layout
        );
        VkcDescriptorSetLayout(VkcDevice& vkcDevice, VkDescriptorSetLayout layout)
            : vkcDevice{ vkcDevice }, descriptorSetLayout{ layout } {
        }
        ~VkcDescriptorSetLayout();
        VkcDescriptorSetLayout(const VkcDescriptorSetLayout&) = delete;
        VkcDescriptorSetLayout& operator=(const VkcDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        VkcDevice& vkcDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class VkcDescriptorWriter;
    };

    class VkcDescriptorPool 
    {
    public:
        class Builder 
        {
        public:
            Builder(VkcDevice& vkcDevice) : vkcDevice{ vkcDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<VkcDescriptorPool> build() const;

        private:
            VkcDevice& vkcDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        VkcDescriptorPool(
            VkcDevice& vkcDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VkcDescriptorPool();
        VkcDescriptorPool(const VkcDescriptorPool&) = delete;
        VkcDescriptorPool& operator=(const VkcDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor, uint32_t variableDescriptorCount) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        VkcDevice& vkcDevice;
        VkDescriptorPool descriptorPool;

        friend class VkcDescriptorWriter;
    };

    class VkcDescriptorWriter 
    {
    public:
        VkcDescriptorWriter(VkcDescriptorSetLayout& setLayout, VkcDescriptorPool& pool);

        VkcDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VkcDescriptorWriter& writeImage(uint32_t binding, const VkDescriptorImageInfo* imageInfo);
        VkcDescriptorWriter& writeImage(uint32_t binding, const VkDescriptorImageInfo* imageInfos, uint32_t count);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        VkcDescriptorSetLayout& setLayout;
        VkcDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
        uint32_t variableDescriptorCount = 0;
    };
}// namespace vkc