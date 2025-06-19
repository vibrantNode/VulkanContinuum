#version 450
#extension GL_KHR_vulkan_glsl : enable

//— Vertex inputs
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;     // glTF vertex color

//— Set 0: global camera + lights UBO (same as your other systems)
layout(set = 0, binding = 0) uniform UBOScene {
    mat4 projection;
    mat4 view;
    vec4 lightPos;   // world space
    vec4 viewPos;    // camera world space
} uboScene;

//— Push‑constants: per‑primitive model + normal
layout(push_constant) uniform PushConsts {
    mat4 model;
    mat4 normalMatrix;
} push;

//— Outputs to the fragment shader
layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec3 fragViewVec;
layout(location = 4) out vec3 fragLightVec;

void main() {
    // World‑space position
    vec4 worldPos = push.model * vec4(inPos, 1.0);
    // Standard MVP
    gl_Position = uboScene.projection * uboScene.view * worldPos;

    // Pass through varyings
    fragNormal   = normalize(mat3(push.normalMatrix) * inNormal);
    fragUV       = inUV;
    fragColor    = inColor;
    fragViewVec  = (uboScene.viewPos.xyz  - worldPos.xyz);
    fragLightVec = (uboScene.lightPos.xyz - worldPos.xyz);
}
