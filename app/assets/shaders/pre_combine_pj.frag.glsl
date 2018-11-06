// Copyright 2017 Benjamin Glatzel
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "lib_math.glsl"
#include "lib_buffers.glsl"
#include "lib_vol_lighting.glsl"
#include "ubos.inc.glsl"
#include "lib_lighting.glsl"

PER_INSTANCE_DATA_PRE_COMBINE;

layout(binding = 1) uniform sampler2D albedoTex;
layout(binding = 2) uniform sampler2D normalTex;
layout(binding = 3) uniform sampler2D param0Tex;
layout(binding = 4) uniform sampler2D albedoTranspTex;

layout(location = 0) in vec2 inUV0;
layout(location = 0) out vec4 outColor;

void main()
{
  outColor = vec4(1.0, 0.0, 0.0, 1.0);

  vec4 albedoTransparents = textureLod(albedoTranspTex, inUV0, 0.0).rgba;
  
  if (albedoTransparents.a <= EPSILON)
  {
    outColor.rgb = textureLod(albedoTex, inUV0, 0.0).rgb;
  }
}
