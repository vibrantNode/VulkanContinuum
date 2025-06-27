// gltf.vert
#version 450
#extension GL_KHR_vulkan_glsl : enable

//— Vertex inputs
layout(location = 0) in vec3  inPos;
layout(location = 1) in vec3  inNormal;
layout(location = 2) in vec2  inUV;
layout(location = 3) in vec4  inColor;
layout(location = 4) in vec4  inTangent;   // .xyz = tangent, .w = bitangent sign

//— Scene UBO (set 0)
struct PointLight {
    vec4 position;
    vec4 color;
};
layout(std140, set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    vec4 lightDirection;
    vec4 viewPos;   
    PointLight pointLights[10];
    int     numLights;
    ivec3 _pad;
} ubo;


layout(set = 1, binding = 0) uniform PerNode {
    mat4 modelMatrix;
    mat4 normalMatrix;           // inverse-transpose of model
} perNode;

//— Outputs to fragment
layout(location = 0) out vec3  fragNormal;
layout(location = 1) out vec4  fragColor;
layout(location = 2) out vec2  fragUV;
layout(location = 3) out vec3  fragViewVec;
layout(location = 4) out vec3  fragLightVec;
layout(location = 5) out vec4  fragTangent;
layout(location = 6) out vec4  fragLightColor;
void main() {
    // world-space position
    vec4 worldPos = perNode.modelMatrix * vec4(inPos, 1.0);
    gl_Position   = ubo.projection * ubo.view * worldPos;

    // normals & tangents in world-space
    fragNormal   = normalize(mat3(perNode.normalMatrix) * inNormal);
    fragTangent  = normalize(perNode.normalMatrix * inTangent); 
    // color & UV
    fragColor    = inColor;
    fragUV       = inUV;

    // view & light vectors
    vec3 camPos   = (ubo.invView * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 lightPos = ubo.pointLights[0].position.xyz;

    fragViewVec  = camPos   - worldPos.xyz;
    fragLightVec = lightPos - worldPos.xyz;
    fragLightColor = ubo.pointLights[0].color;
}