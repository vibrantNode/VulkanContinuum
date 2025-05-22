#include "vk_descriptors.h"

// std
#include <cassert>
#include <stdexcept>

uint32_t maxTextures = 1000;


namespace vkc {

    // *************** Descriptor Set Layout Builder *********************

    VkcDescriptorSetLayout::Builder& VkcDescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count,
        VkDescriptorBindingFlags flags) {

        assert(bindings.count(binding) == 0 && "Binding already in use");

        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        layoutBinding.pImmutableSamplers = nullptr;

        bindings[binding] = layoutBinding;
        bindingFlags[binding] = flags;

        return *this;
    }
    VkcDescriptorSetLayout::Builder& VkcDescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags) {

        // Default count = 1, no special flags
        return addBinding(binding, descriptorType, stageFlags, 1, 0);
    }
    std::unique_ptr<VkcDescriptorSetLayout> VkcDescriptorSetLayout::Builder::build() const {
        std::vector<VkDescriptorSetLayoutBinding> setBindings;
        std::vector<VkDescriptorBindingFlags> setBindingFlags;

        for (const auto& [binding, layoutBinding] : bindings) {
            setBindings.push_back(layoutBinding);
            setBindingFlags.push_back(bindingFlags.at(binding));
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsInfo.bindingCount = static_cast<uint32_t>(setBindingFlags.size());
        bindingFlagsInfo.pBindingFlags = setBindingFlags.data();

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = &bindingFlagsInfo;
        layoutInfo.bindingCount = static_cast<uint32_t>(setBindings.size());
        layoutInfo.pBindings = setBindings.data();
        layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(vkcDevice.device(), &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        return std::make_unique<VkcDescriptorSetLayout>(vkcDevice, setBindings, layout);
    }
    // *************** Descriptor Set Layout *********************

    VkcDescriptorSetLayout::VkcDescriptorSetLayout(
        VkcDevice& vkcDevice,
        const std::vector<VkDescriptorSetLayoutBinding>& bindingsVec,
        VkDescriptorSetLayout layout
    ) : vkcDevice{ vkcDevice }, descriptorSetLayout{ layout } {

        for (const auto& binding : bindingsVec) {
            bindings[binding.binding] = binding;
        }
    }


    VkcDescriptorSetLayout::~VkcDescriptorSetLayout() {
        if (descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(vkcDevice.device(), descriptorSetLayout, nullptr);
            descriptorSetLayout = VK_NULL_HANDLE;
        }
    }

    // *************** Descriptor Pool Builder *********************

    VkcDescriptorPool::Builder& VkcDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    VkcDescriptorPool::Builder& VkcDescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    VkcDescriptorPool::Builder& VkcDescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<VkcDescriptorPool> VkcDescriptorPool::Builder::build() const {
        return std::make_unique<VkcDescriptorPool>(vkcDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    VkcDescriptorPool::VkcDescriptorPool(
        VkcDevice& vkcDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes)
        : vkcDevice{ vkcDevice } {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(vkcDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    VkcDescriptorPool::~VkcDescriptorPool() {
        if (descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(vkcDevice.device(), descriptorPool, nullptr);
            descriptorPool = VK_NULL_HANDLE;
        }
    }

    bool VkcDescriptorPool::allocateDescriptor(const 
        VkDescriptorSetLayout descriptorSetLayout,
        VkDescriptorSet& descriptor,
        uint32_t variableDescriptorCount
    ) 
        const {
        VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
        countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        countInfo.descriptorSetCount = 1;
        countInfo.pDescriptorCounts = &variableDescriptorCount;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.pNext = &countInfo;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(vkcDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void VkcDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            vkcDevice.device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void VkcDescriptorPool::resetPool() {
        vkResetDescriptorPool(vkcDevice.device(), descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    VkcDescriptorWriter::VkcDescriptorWriter(VkcDescriptorSetLayout& setLayout, VkcDescriptorPool& pool)
        : setLayout{ setLayout }, pool{ pool } {
    }
    uint32_t variableDescriptorCount = 0;
    VkcDescriptorWriter& VkcDescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");
       
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    VkcDescriptorWriter& VkcDescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo* imageInfo
        ) {
        auto& bindingDescription = setLayout.bindings[binding];
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

      

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }
    VkcDescriptorWriter& VkcDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfos, uint32_t count) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        assert(setLayout.bindings.at(binding).descriptorCount >= count && "Too many image descriptors for binding");


        variableDescriptorCount = count;
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorType = setLayout.bindings[binding].descriptorType;
        write.descriptorCount = count;
        write.pImageInfo = imageInfos;
        write.pBufferInfo = nullptr;
        write.pTexelBufferView = nullptr;

        writes.push_back(write);
        return *this;
    }

    bool VkcDescriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set, variableDescriptorCount);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void VkcDescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.vkcDevice.device(), writes.size(), writes.data(), 0, nullptr);
    }


}// namespace vkc