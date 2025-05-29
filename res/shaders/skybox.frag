// skybox.frag
#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 vTexCoords;

layout(set = 1, binding = 0) uniform samplerCube skybox;

layout(location = 0) out vec4 outColor;

void main() {
    // Sample the cubemap in the direction from the center
    outColor = texture(skybox, normalize(vTexCoords));
}