#version 450
#extension GL_KHR_vulkan_glsl : enable 
layout(set = 0, binding = 0) uniform sampler2D hdrImage;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

vec3 reinhardTonemap(vec3 color) {
    return color / (color + vec3(1.0));
}

void main() {
    vec3 hdr = texture(hdrImage, uv).rgb;
    vec3 ldr = reinhardTonemap(hdr);
    outColor = vec4(ldr, 1.0);
}