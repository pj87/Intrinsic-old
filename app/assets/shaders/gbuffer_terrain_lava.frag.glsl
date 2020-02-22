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
BINDINGS_TERRAIN;

// Input
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inTangent;
layout(location = 2) in vec3 inBinormal;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec2 inUV0;
layout(location = 5) in vec3 inPosition;
layout(location = 7) in vec3 inNormalTPM;

// Output
OUTPUT

const float pi = 3.14159;
const vec4 cHashA4 = vec4 (0., 1., 57., 58.);
const vec3 cHashA3 = vec3 (1., 57., 113.);
const float cHashM = 43758.54;

vec4 Hashv4f (float p)
{
  return fract (sin (p + cHashA4) * cHashM);
}

float Noisefv2 (vec2 p)
{
  vec2 ip = floor (p);
  vec2 fp = fract (p);
  fp = fp * fp * (3. - 2. * fp);
  vec4 t = Hashv4f (dot (ip, cHashA3.xy));
  return mix (mix (t.x, t.y, fp.x), mix (t.z, t.w, fp.x), fp.y);
}

float Fbmn (vec3 p, vec3 n)
{
  vec3 s;
  float a;
  s = vec3 (0.);
  a = 1.;
  for (int i = 0; i < 5; i ++) {
    s += a * vec3 (Noisefv2 (p.yz), Noisefv2 (p.zx), Noisefv2 (p.xy));
    a *= 0.5;
    p *= 2.;
  }
  return dot (s, abs (n));
}

vec3 VaryNf (vec3 p, vec3 n, float f)
{
  vec3 g;
  float s;
  vec3 e = vec3 (0.1, 0., 0.);
  s = Fbmn (p, n);
  g = vec3 (Fbmn (p + e.xyy, n) - s,
     Fbmn (p + e.yxy, n) - s, Fbmn (p + e.yyx, n) - s);
  return normalize (n + f * (g - n * dot (n, g)));
}

vec3 HsvToRgb (vec3 c)
{
  vec3 p = abs (fract (c.xxx + vec3 (1., 2./3., 1./3.)) * 6. - 3.);
  return c.z * mix (vec3 (1.), clamp (p - 1., 0., 1.), c.y);
}

vec2 Rot2D (vec2 q, float a)
{
  return q * cos (a) * vec2 (1., 1.) + q.yx * sin (a) * vec2 (-1., 1.);
}

mat3 flMat;
vec3 flPos, ltPos, ltAx;
float tCur;
float dstFar = 100.;

vec3 TrackPath (float t)
{
  return vec3 (10. * sin (0.1 * t) * sin (0.06 * t) * cos (0.033 * t) +
     3. * cos (0.025 * t), 6., t);
}

float GrndDf (vec3 p)
{
  const mat2 qRot = mat2 (1.6, -1.2, 1.2, 1.6);
  vec2 q, t, ta, v;
  float wAmp, pRough, ht;
  wAmp = 1.;
  pRough = 0.5;
  q = 0.4 * p.xz;
  ht = 0.;
  for (int j = 0; j < 3; j ++) {
    t = q + 2. * Noisefv2 (q) - 1.;
    ta = abs (sin (t));
    v = (1. - ta) * (ta + abs (cos (t)));
    v = pow (1. - v, vec2 (pRough));
    ht += (v.x + v.y) * wAmp;
    q *= 1.5 * qRot;
    wAmp *= 0.25;
    pRough = 0.6 * pRough + 0.2;
  }
  return p.y - ht;
}

float GrndRay (vec3 ro, vec3 rd)
{
  vec3 p;
  float dHit, h, s, sLo, sHi;
  s = 0.;
  sLo = 0.;
  dHit = dstFar;
  for (int j = 0; j < 100; j ++) {
    p = ro + s * rd;
    h = GrndDf (p);
    if (h < 0.) break;
    sLo = s;
    s += 0.8 * h + 0.005 * s;
    if (s > dstFar) break;
  }
  if (h < 0.) {
    sHi = s;
    for (int j = 0; j < 8; j ++) {
      s = 0.5 * (sLo + sHi);
      p = ro + s * rd;
      h = step (0., GrndDf (p));
      sLo += h * (s - sLo);
      sHi += (1. - h) * (s - sHi);
    }
    dHit = sHi;
  }
  return dHit;
}

vec3 GrndNf (vec3 p)
{
  vec4 v;
  const vec3 e = 0.0001 * vec3 (1., -1., 0.);
  v = vec4 (GrndDf (p + e.xxx), GrndDf (p + e.xyy),
     GrndDf (p + e.yxy), GrndDf (p + e.yyx));
  return normalize (vec3 (v.x - v.y - v.z - v.w) + 2. * v.yzw);
}

float GrndGlow (vec3 ro, vec3 rd)
{
  float gl, f, d;
  gl = 0.;
  f = 1.;
  d = 0.;
  for (int j = 0; j < 5; j ++) {
    d += 0.4;
    gl += f * max (d - GrndDf (ro + rd * d), 0.);
    f *= 0.5;
  }
  return clamp (gl, 0., 1.);
}

vec3 tex3D(vec3 pos, vec3 nor, sampler2D s) {
    return texture( s, pos.yz).xyz*abs(nor.x)+
           texture( s, pos.xz).xyz*abs(nor.y)+
           texture( s, pos.xy).xyz*abs(nor.z);
}

vec3 tex3DNormal(vec3 pos, vec3 nor, sampler2D s) {
    return textureNormal( s, pos.yz).xyz*abs(nor.x)+
           textureNormal( s, pos.xz).xyz*abs(nor.y)+
           textureNormal( s, pos.xy).xyz*abs(nor.z);
}

vec3 blend(vec3 grass0, vec3 stone0, vec3 stone1, vec3 blendMask, float noise)
{
  return mix(grass0, mix(stone0, stone1, 1.0 - noise),
             clamp(blendMask.b * 3.0, 0.0, 1.0));
}

vec3 contrast(vec3 color, float contrast)
{
  return ((color.rgb - 0.5) * max(contrast, 0)) + 0.5;
}

float contrast(float color, float contrast)
{
  return ((color - 0.5) * max(contrast, 0)) + 0.5;
}

mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );

    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

void main()
{
  //const mat3 TBN = mat3(inTangent, inBinormal, inNormal);
  const mat3 TBN = cotangent_frame(inNormal, inPosition, inUV0);

  const vec2 uv0 = UV0_TRANSFORM_ANIMATED(inUV0);
  const vec2 uv0Raw = UV0(inUV0);

  const vec4 albedo0 = vec4(tex3D(inPosition, inNormalTPM, albedoTex0), 1.0);
  const vec3 normal0 = tex3DNormal(inPosition, inNormalTPM, normalTex0);
  const vec3 pbr0 = tex3D(inPosition, inNormalTPM, pbrTex0).rgg;

  const vec4 albedo1 = vec4(tex3D(inPosition * 0.5, inNormalTPM, albedoTex1), 1.0);
  const vec3 normal1 = tex3DNormal(inPosition * 0.5, inNormalTPM, normalTex1);
  const vec3 pbr1 = tex3D(inPosition * 0.1, inNormalTPM, pbrTex1).rgg;

  const vec4 albedo2 = vec4(tex3D(inPosition * 0.25, inNormalTPM, albedoTex2), 1.0);
  const vec3 normal2 = tex3DNormal(inPosition * 0.25, inNormalTPM, normalTex2);
  const vec3 pbr2 = tex3D(inPosition * 0.1, inNormalTPM, pbrTex2).rgg;

  float noise = clamp(tex3D(inPosition * 10.0, inNormalTPM, noiseTex).r, 0.0, 1.0);
  vec3 blendMask = tex3D(inPosition * 10.0, inNormalTPM, blendMaskTex).rgb;

  vec3 albedo = vec3(0.0);// blend(albedo0.rgb, albedo1.rgb, albedo2.rgb, blendMask, noise);
  
  vec3 normal = blend(normal0.rgb, normal1.rgb, normal2.rgb, blendMask, noise);
  vec2 pbr = vec2(0.0); //blend(pbr0.rgb, pbr1.rgb, pbr2.rgb, blendMask, noise).rg;

  float occlusion =
      clamp(mix(clamp(noise * 5.0, 0.0, 1.0) * blendMask.b, 1.0 - blendMask.r,
                clamp((1.0 - blendMask.g) * 2.0 - 0.9, 0.0, 1.0)) *
                    2.0 +
                0.2,
            0.0, 1.0);
  albedo *= occlusion;

  GBuffer gbuffer;
  { 
    vec3 vn, ltDir;
    float dstGrnd, di, atten, glw, dk;
	atten = 30. * pow (min (di, 1.), 1.3) * pow (max (dot (ltAx, ltDir), 0.), 64.);
	vec3 ro = inPosition;
	vec3 rd = vec3(0.0);
    vn = GrndNf (ro);
    vn = VaryNf (5. * ro, vn, max (2., 6. - 0.3 * dstGrnd));
    glw = GrndGlow (ro, vn);
	
	albedo = vec3(min (0.5 * Fbmn (31. * ro, vn), 1.));
	
	gbuffer.albedo = vec4(albedo, 1.0) * uboPerInstance.colorTint;	
    gbuffer.normal = normalize(TBN * normal);
    gbuffer.metalMask = pbr.r + uboPerMaterial.pbrBias.r;
    gbuffer.specular = 0.5 + uboPerMaterial.pbrBias.g;
    gbuffer.roughness = pbr.g + uboPerMaterial.pbrBias.b;
    gbuffer.materialBufferIdx = uboPerMaterial.data0.x;
    gbuffer.occlusion = 1.0;
    gbuffer.emissive = 0.0;
  }
  writeGBuffer(gbuffer, outAlbedo, outNormal, outParameter0);
}
