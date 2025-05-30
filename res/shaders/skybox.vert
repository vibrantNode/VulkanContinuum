#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView; 
    vec4 ambientLightColor;
} ubo;

layout(location = 0) out vec3 vDirection;

void main() {
    mat4 rotView = mat4(mat3(ubo.view));
    
    vec4 clipPos = ubo.projection * rotView * vec4(position, 1.0);
    gl_Position = clipPos.xyww; // Depth = 1.0 (far plane)
    
    vDirection = position; // Direction for cubemap sampling
}
