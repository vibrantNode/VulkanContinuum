#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec4 inJoint0;     // unused for now
layout(location = 5) in vec4 inWeight0;    // unused for now
layout(location = 6) in vec4 inTangent;    // unused for now

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(set = 1, binding = 0) uniform PerNode {
    mat4 modelMatrix;
    mat4 normalMatrix;
} perNode;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec3 fragViewVec;
layout(location = 4) out vec3 fragLightVec;

void main() {
    vec4 worldPos = perNode.modelMatrix * vec4(inPos, 1.0);
    gl_Position = ubo.projection * ubo.view * worldPos;

    fragNormal   = normalize(mat3(perNode.normalMatrix) * inNormal);
    fragUV       = inUV;
    fragColor    = inColor;

    vec3 camPos   = (ubo.invView * vec4(0,0,0,1)).xyz;
    vec3 lightPos = (ubo.pointLights[0].position).xyz;

    fragViewVec  = camPos   - worldPos.xyz;
    fragLightVec = lightPos - worldPos.xyz;
}
