// skybox.vert
#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;        // optional if you’d rather flip
    vec4 ambientLightColor;
    // … other members …
} ubo;

layout(location = 0) out vec3 vTexCoords;

void main() {
    // Strip translation from the view matrix so the cube follows the camera
    mat4 rotView = mat4(mat3(ubo.view));

     vec4 pos = rotView * vec4(position * 100.0, 1.0); // exaggerate to test

    gl_Position = ubo.projection * pos;
    vTexCoords = position; // direction vector into the cubemap
}