#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(set = 1, binding = 0) uniform samplerCube skybox;

layout(location = 0) in vec3 vDirection;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(skybox, normalize(vDirection));
}
