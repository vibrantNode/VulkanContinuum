// vk_core.cpp

// std
#include <iostream>
#include <chrono>
#include <thread>

// Vulkan
#include <vulkan/vulkan.h>


// Third party
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp> 

// Project source specific includes
#include "vk_core.h"
#include "VkBootstrap.h"




Core::Application* loadedApp = nullptr;

Core::Application& Core::Application::Get() { return *loadedApp; }

void Core::Application::Init()
{

    // Initialize GLFW window

    InitGLFW();

	InitVulkan();

    InitSwapChain();

	b_IsInitialized = true;
}

void Core::Application::RunApp()
{

    while (b_IsInitialized = true) {
        glfwPollEvents();
        
    }
}

void Core::Application::CleanUp()
{
}

constexpr bool bUseValidationLayers = false;

void Core::Application::InitVulkan()
{


    vkb::InstanceBuilder builder;

    // Make the Vulkan instance, with basic debug features
    auto inst_ret = builder.set_app_name("Vulkan Continuum")
        .request_validation_layers(bUseValidationLayers)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();

    vkb::Instance vkb_inst = inst_ret.value();

    // Grab the instance
    _instance = vkb_inst.instance;
    _debug_messenger = vkb_inst.debug_messenger;

    // Create the Vulkan surface for the GLFW window
    if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan surface!");
    }


    // Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features.dynamicRendering = VK_TRUE;
    features.synchronization2 = VK_TRUE;

    // Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12 = {};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.descriptorIndexing = VK_TRUE;

    // Use vkbootstrap to select a GPU
    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    vkb::PhysicalDevice physicalDevice = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features)
        .set_required_features_12(features12)
        .set_surface(_surface)
        .select()
        .value();

    // Create the final Vulkan device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };

    vkb::Device vkbDevice = deviceBuilder.build().value();

    // Get the VkDevice handle used in the rest of a Vulkan application
    _device = vkbDevice.device;
    _chosenGPU = physicalDevice.physical_device;

    // Use vkbootstrap to get a graphics queue
    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // Initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _chosenGPU;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &_allocator);

   
    // Push the allocator cleanup function to the deletion queue
    _mainDeletionQueue.push_function([&]() {
        vmaDestroyAllocator(_allocator);
        });
	
}

void Core::Application::InitSwapChain()
{
}

void Core::Application::InitGLFW()
{
    glfwInit();
    // Set GLFW to not create an OpenGL context, as we're using Vulkan
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);  // Optional: make the window non-resizable

    // Create the GLFW window
    _window = glfwCreateWindow(_windowExtent.width, _windowExtent.height, "Vulkan Continuum", nullptr, nullptr);
    if (!_window) {
        throw std::runtime_error("Failed to create GLFW window!");
    }
}
