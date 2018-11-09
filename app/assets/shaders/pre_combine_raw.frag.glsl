#version 450

layout(binding = 0) uniform sampler2D albedoTex;
layout(binding = 1) uniform sampler2D albedoTranspTex;

layout(location = 0) in vec2 inUV0;
layout(location = 0) out vec4 outColor;

void main()
{
  outColor = vec4(1.0, 0.0, 0.0, 1.0);

  vec4 albedoTransparents = textureLod(albedoTranspTex, inUV0, 0.0).rgba;
  
  if (albedoTransparents.a <= 1.0e-6)
  {
    outColor.rgb = textureLod(albedoTex, inUV0, 0.0).rgb;
  }
}
