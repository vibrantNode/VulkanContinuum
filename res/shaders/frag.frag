#version 450
#extension GL_EXT_nonuniform_qualifier : enable


layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(location = 4) flat in int inTexIndex;

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;
//layout(location = 4) flat in int outTexIndex; // Receive from vertex shader

layout (location = 0) out vec4 outColor;


//layout(set = 0, binding = 0) uniform sampler2D textures[];

struct PointLight {
	vec4 position;
	vec4 color;
	};


layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
} ubo;

//layout(set = 0, binding = 1) uniform sampler2D texSampler; 


layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
  int textureIndex;
} push;

void main() {
 vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 specularLight = vec3(0.0);
  vec3 surfaceNormal = normalize(fragNormalWorld);

  vec3 cameraPosWorld = ubo.invView[3].xyx;
  vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 directionToLight = light.position.xyz - fragPosWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
    directionToLight = normalize(directionToLight);

    float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
    vec3 intensity = light.color.xyz * light.color.w * attenuation;

    diffuseLight += intensity * cosAngIncidence;

    vec3 halfAngle = normalize(directionToLight + viewDirection);
    float blinnTerm = dot(surfaceNormal, halfAngle);
    blinnTerm = clamp(blinnTerm, 0, 1);
    blinnTerm = pow(blinnTerm, 250.0);
    specularLight += intensity * blinnTerm;
  }

  vec4 textureColor = texture(textures[nonuniformEXT(inTexIndex)], fragUV);

  // Combine lighting with texture color (modulate texture with lighting)
  vec3 finalColor = (diffuseLight + specularLight) * textureColor.rgb;

  outColor = vec4(finalColor, textureColor.a);
}

//  vec4 textureColor = texture(nonuniformEXT[outTexIndex], fragUV); // 