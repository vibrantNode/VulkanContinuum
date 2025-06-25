// standard.frag
#version 450
#extension GL_KHR_vulkan_glsl : enable

//— Material textures (set 2)
layout(set = 2, binding = 0) uniform sampler2D materialSampler;
layout(set = 2, binding = 1) uniform sampler2D normalSampler;

//— Interpolated inputs
layout(location = 0) in vec3  fragNormal;
layout(location = 1) in vec4  fragColor;
layout(location = 2) in vec2  fragUV;
layout(location = 3) in vec3  fragViewVec;
layout(location = 4) in vec3  fragLightVec;
layout(location = 5) in vec4  fragTangent;

//— Push in ambient from UBO
layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    // (others unused here…)
} ubo;

//— Specialization constants for alpha masking
layout(constant_id = 0) const bool  ALPHA_MASK        = false;
layout(constant_id = 1) const float ALPHA_MASK_CUTOFF = 0.0;

//— Final color
layout(location = 0) out vec4 outFragColor;

void main() {
    // — Sample base‐color & apply vertex tint
    vec4 baseTex = texture(materialSampler, fragUV);
    vec4 color   = baseTex * fragColor;

    // — Optional alpha test
    if (ALPHA_MASK && color.a < ALPHA_MASK_CUTOFF) {
        discard;
    }

    // — Build TBN matrix
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent.xyz);
    vec3 B = cross(N, T) * fragTangent.w;
    mat3 TBN = mat3(T, B, N);

    // — Decode normal‐map, transform to world‐space
    vec3 nmap = texture(normalSampler, fragUV).xyz * 2.0 - 1.0;
    N = normalize(TBN * nmap);

    // — Lighting vectors
    vec3 L = normalize(fragLightVec);
    vec3 V = normalize(fragViewVec);
    vec3 R = reflect(-L, N);

    // — Ambient + Blinn-Phong
    float ambient = ubo.ambientLightColor.r;
    float diff    = max(dot(N, L), ambient);
    float spec    = pow(max(dot(R, V), 0.0), 32.0);

    vec3 lighting = diff * color.rgb + spec * vec3(1.0);
    outFragColor  = vec4(lighting, color.a);
}
