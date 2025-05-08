cmake_minimum_required(VERSION "3.13")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

add_subdirectory("vendor/glfw")

find_package (Vulkan REQUIRED)
project(VKContinuum)
set(CMAKE_CXX_STANDARD 17)

set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.3.296.0/Include")
set(Vulkan_LIBRARY "C:/VulkanSDK/1.3.296.0/Lib/vulkan-1.lib")
include_directories(${Vulkan_INCLUDE_DIR}) 

add_executable(VKContinuum
        # src
        src/main.cpp
        src/AppCore/vk_core.cpp
        src/AppCore/vk_window.cpp 

        src/Renderer/vk_renderer.cpp 

        src/VK_abstraction/vk_initializers.cpp 
        src/VK_abstraction/vk_device.cpp 
        src/VK_abstraction/vk_swapchain.cpp 
        src/VK_abstraction/vk_descriptors.cpp 
        src/VK_abstraction/vk_pipeline.cpp 

        # vendor
        vendor/vkbootstrap/VkBootstrap.cpp 
       "src/Renderer/vk_buffer.h" "src/Renderer/vk_buffer.cpp" "src/Renderer/Types/vk_basicRenderSystem.cpp" "src/VK_abstraction/vk_camera.cpp" "src/VK_abstraction/vk_model.cpp" "src/VK_abstraction/vk_gameObject.cpp" "src/VK_abstraction/vk_input.cpp" "src/Renderer/Types/vk_pointLightSystem.cpp")

# Define PROJECT_ROOT_DIR for use in the code
target_compile_definitions(VKContinuum PRIVATE PROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")


# Include Directories
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
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Link vendor libs
target_link_libraries(VKContinuum PRIVATE 
    ${Vulkan_LIBRARY} 
    glfw
)

# Shader Compilation
file(GLOB SHADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/*.vert" "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/*.frag")

set(SPIRV_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/spirV")
file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(FILE_NAME ${SHADER_FILE} NAME)
    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT_DIR}/${FILE_NAME}.spv
        COMMAND ${CMAKE_COMMAND} -E echo "Compiling shader: ${FILE_NAME}"
        COMMAND glslangValidator -V ${SHADER_FILE} -o ${SPIRV_OUTPUT_DIR}/${FILE_NAME}.spv
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling ${FILE_NAME} to SPIR-V"
    )
    list(APPEND SPIRV_BINARIES ${SPIRV_OUTPUT_DIR}/${FILE_NAME}.spv)
endforeach()

add_custom_target(CompileShaders ALL DEPENDS ${SPIRV_BINARIES})

# Ensure shaders are compiled before the VKContinuum executable
add_dependencies(VKContinuum CompileShaders)