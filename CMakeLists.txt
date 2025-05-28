cmake_minimum_required(VERSION 3.13)

# Set project info early
project(VKContinuum)
set(CMAKE_CXX_STANDARD 17)

# MSVC Runtime setting
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Vulkan SDK
set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.3.296.0/Include")
set(Vulkan_LIBRARY "C:/VulkanSDK/1.3.296.0/Lib/vulkan-1.lib")
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIR})

# GLFW
add_subdirectory("vendor/glfw")



# Executable
add_executable(VKContinuum
    src/main.cpp
    src/AppCore/_vkCore.cpp
    src/AppCore/vk_window.cpp
    src/Renderer/vk_renderer.cpp
    src/VK_abstraction/vk_initializers.cpp
    src/VK_abstraction/vk_device.cpp
    src/VK_abstraction/vk_swapchain.cpp
    src/VK_abstraction/vk_descriptors.cpp
    src/VK_abstraction/vk_pipeline.cpp
    src/VK_abstraction/vk_buffer.h
    src/VK_abstraction/vk_buffer.cpp
    vendor/vkbootstrap/VkBootstrap.cpp
    src/Renderer/RendererSystems/vk_basicRenderSystem.cpp
    src/Game/Camera/vk_camera.cpp
    src/VK_abstraction/vk_model.cpp
    src/Game/vk_gameObject.cpp
    src/Game/Input/vk_input.cpp
    src/Renderer/RendererSystems/vk_pointLightSystem.cpp
    src/AppCore/vk_assetManager.cpp
    src/Game/vk_scene.cpp
    src/Game/vk_player.cpp
    src/VK_abstraction/vk_texture.cpp
    src/Renderer/RendererSystems/vk_skyboxRenderSystem.cpp
    src/Game/vk_game.cpp
    src/Renderer/RendererSystems/vk_renderSystemManager.cpp
 "src/Renderer/vk_descriptorManager.cpp")

# Definitions
target_compile_definitions(VKContinuum PRIVATE PROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Include paths
target_include_directories(VKContinuum PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/GLFW"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyobjloader"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/vkbootstrap"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/vma"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/vk_video"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinygltf"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/ktx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AppCore"
)

# Link libraries
target_link_libraries(VKContinuum PRIVATE 
    ${Vulkan_LIBRARY} 
    glfw
)

# Shader compilation
file(GLOB SHADER_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/*.vert" 
    "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/*.frag"
)

set(SPIRV_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/spirV")
file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(FILE_NAME ${SHADER_FILE} NAME)
    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT_DIR}/${FILE_NAME}.spv
        COMMAND ${CMAKE_COMMAND} -E echo "Compiling shader: ${FILE_NAME}"
        COMMAND glslangValidator -V --target-env vulkan1.3 ${SHADER_FILE} -o ${SPIRV_OUTPUT_DIR}/${FILE_NAME}.spv
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling ${FILE_NAME} to SPIR-V"
    )
    list(APPEND SPIRV_BINARIES ${SPIRV_OUTPUT_DIR}/${FILE_NAME}.spv)
endforeach()

add_custom_target(CompileShaders ALL DEPENDS ${SPIRV_BINARIES})
add_dependencies(VKContinuum CompileShaders)
