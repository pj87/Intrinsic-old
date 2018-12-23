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
#include "lib_lighting.glsl"
#include "lib_buffers.glsl"
#include "lib_vol_lighting.glsl"
#include "lib_noise.glsl"
#include "lib_clustering.glsl"
#include "ubos.inc.glsl"

layout(binding = 0) uniform PerInstance
{
  mat4 projMatrix;
  mat4 prevViewProjMatrix;

  vec4 eyeVSVectorX;
  vec4 eyeVSVectorY;
  vec4 eyeVSVectorZ;

  vec4 eyeWSVectorX;
  vec4 eyeWSVectorY;
  vec4 eyeWSVectorZ;

  vec4 data0;

  vec4 camPos;

  mat4 shadowViewProjMatrix[MAX_SHADOW_MAP_COUNT];

  vec4 nearFar;
  vec4 nearFarWidthHeight;

  vec4 haltonSamples;
}
uboPerInstance;

PER_FRAME_DATA(1);

layout(binding = 2) uniform sampler2DArrayShadow shadowBufferTex;
layout(binding = 3, r11f_g11f_b10f) uniform image3D output0Tex;
layout(binding = 4) uniform sampler3D prevVolLightBufferTex;
layout(binding = 5) buffer LightBuffer { Light lights[]; };
layout(binding = 6) buffer LightIndexBuffer { uint lightIndices[]; };
layout(binding = 7) buffer IrradProbeBuffer { IrradProbe irradProbes[]; };
layout(binding = 8) buffer IrradProbeIndexBuffer { uint irradProbeIndices[]; };
layout(binding = 9) uniform sampler2DArray shadowBufferExpTex;
layout(binding = 10) uniform sampler2D kelvinLutTex;
// Based on AC4 volumetric fog
layout(local_size_x = 4u, local_size_y = 4u, local_size_z = 4u) in;
void main()
{
  vec3 cellIndex = vec3(gl_GlobalInvocationID.xyz) + 0.5;
  //imageStore(output0Tex, ivec3(cellIndex), mix(fog, reprojFog, reprojWeight));
  imageStore(output0Tex, ivec3(cellIndex), vec4(1.0, 0.0, 0.0, 0.0));
  
}
