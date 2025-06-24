// glTFfrag.frag
#version 450

//— Set 2: per-material sampler (Sascha’s descriptorSetLayoutImage)
layout(set = 2, binding = 0) uniform sampler2D materialSampler;

//— Interpolated inputs
layout(location = 0) in vec3  fragNormal;
layout(location = 1) in vec4  fragColor;
layout(location = 2) in vec2  fragUV;
layout(location = 3) in vec3  fragViewVec;
layout(location = 4) in vec3  fragLightVec;

//— Final color
layout(location = 0) out vec4 outFragColor;

void main() {
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(fragLightVec);
    vec3 V = normalize(fragViewVec);
    vec3 R = reflect(-L, N);

    // sample your glTF base-color texture
    vec4 texColor = texture(materialSampler, fragUV);

    // combine with vertex color if present
    vec3 baseColor = texColor.rgb * fragColor.rgb;

    // simple Blinn-Phong
    float diff = max(dot(N, L), 0.15);
    float spec = pow(max(dot(R, V), 0.0), 16.0) * 0.75;
    vec3 lighting = diff * baseColor + spec;

    outFragColor = vec4(lighting, texColor.a * fragColor.a);
}
