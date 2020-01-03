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
#include "gbuffer.inc.glsl"

// Ubos
PER_MATERIAL_UBO;
PER_INSTANCE_UBO;

// Bindings
BINDINGS_GBUFFER;
layout(binding = 6) uniform sampler2D emissiveTex;

// Input
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inTangent;
layout(location = 2) in vec3 inBinormal;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec2 inUV0;
layout(location = 5) in vec3 inPosition;

// Output
OUTPUT

vec3 tex3D(vec3 pos, vec3 nor, sampler2D s) {
	return texture( s, pos.yz).xyz*abs(nor.x)+
	       texture( s, pos.xz).xyz*abs(nor.y)+
	       texture( s, pos.xy).xyz*abs(nor.z);
}

vec4 tex3d(vec3 pos, vec3 normal)
{
	pos /= 4.0;

	// loook up brick texture, blended across xyz axis based on normal.
	vec4 texX = texture(albedoTex, pos.yz);
	vec4 texY = texture(albedoTex, pos.xz);
	vec4 texZ = texture(albedoTex, pos.xy);
	//vec4 tex = mix(texX, texZ, abs(normal.z));
	//tex = mix(tex, texY, abs(normal.y));//.zxyw;
	
	vec3 avgNormal = abs(normal);
	avgNormal / (avgNormal.x + avgNormal.y + avgNormal.z);
	
	vec4 albedo = texX * avgNormal.x + texY * avgNormal.y + texZ * avgNormal.z;
	
	return albedo;
}

vec4 tex3d_1(vec3 pos, vec3 normal)
{
	pos /= 4.0;

	// loook up brick texture, blended across xyz axis based on normal.
	vec4 texX = texture(emissiveTex, pos.yz);
	vec4 texY = texture(emissiveTex, pos.xz);
	vec4 texZ = texture(emissiveTex, pos.xy);
	//vec4 tex = mix(texX, texZ, abs(normal.z));
	//tex = mix(tex, texY, abs(normal.y));//.zxyw;
	
	vec3 avgNormal = abs(normal);
	avgNormal / (avgNormal.x + avgNormal.y + avgNormal.z);
	
	vec4 albedo = texX * avgNormal.x + texY * avgNormal.y + texZ * avgNormal.z;
	
	return albedo;
}

void main()
{
  const mat3 TBN = mat3(inTangent, inBinormal, inNormal);
  //const vec2 uv0 = UV0_TRANSFORM_ANIMATED(inUV0);
  const vec2 uv0 = inUV0;

  GBuffer gbuffer;
  {
    vec3 pos = inPosition;// * 100.0;
	
	//pos.x = 0.0;
    //pos.y = mix(0.0, 1.0, pos.y);
	//pos.y = 0.0;
	//pos.z = 0.0;
	
    //gbuffer.albedo = vec4(normalize(inPosition), 1.0) + vec4(tex3D(normalize(inPosition), inNormal, albedoTex), 1.0) * uboPerInstance.colorTint;
	//gbuffer.albedo = /*vec4(normalize(pos), 1.0) + */vec4(tex3D(normalize(pos * vec3(100.0, 100.0, 100.0)), inNormal, albedoTex), 1.0) * uboPerInstance.colorTint;
	
	gbuffer.albedo = vec4(tex3D(pos * vec3(1.0, 1.0, 1.0), inNormal, albedoTex), 1.0) * uboPerInstance.colorTint;
	//gbuffer.albedo = vec4(normalize(pos), 1.0);
	//gbuffer.albedo = vec4(pos, 1.0);
    //gbuffer.albedo = vec4(inColor, 1.0) + tex3d(inPosition, inNormal) * uboPerInstance.colorTint;
	//gbuffer.albedo = vec4(inColor, 1.0) + mix(texture(albedoTex, uv0), texture(emissiveTex, uv0), 1.0) * uboPerInstance.colorTint;
	//gbuffer.albedo = texture(albedoTex, uv0) * uboPerInstance.colorTint;
	/*
    gbuffer.normal = normalize(TBN * textureNormal(normalTex, uv0));
    const vec2 pbr = texture(pbrTex, uv0).rg;
    gbuffer.metalMask = pbr.r + uboPerMaterial.pbrBias.r;
    gbuffer.specular = 0.5 + uboPerMaterial.pbrBias.g;
    gbuffer.roughness = adjustRoughness(pbr.g + uboPerMaterial.pbrBias.b,
                                        uboPerMaterial.data1.x);
    gbuffer.materialBufferIdx = uboPerMaterial.data0.x;
    gbuffer.emissive = texture(emissiveTex, uv0).r;
    gbuffer.occlusion = 1.0;
	*/
  }
  writeGBuffer(gbuffer, outAlbedo, outNormal, outParameter0);
}
