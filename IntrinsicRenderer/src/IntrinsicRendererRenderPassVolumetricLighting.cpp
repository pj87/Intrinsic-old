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

// Precompiled header file
#include "stdafx.h"

using namespace RResources;
using namespace CComponents;
using namespace CResources;

namespace Intrinsic
{
namespace Renderer
{
namespace RenderPass
{
namespace
{
struct PerInstanceDataESMGenerate
{
  glm::uvec4 arrayIdx;
};

struct PerInstanceDataESMBlur
{
  glm::vec4 blurParams;
  glm::uvec4 arrayIdx;
};

struct PerInstanceData
{
  glm::mat4 projMatrix;
  glm::mat4 prevViewProjMatrix;

  glm::vec4 eyeVSVectorX;
  glm::vec4 eyeVSVectorY;
  glm::vec4 eyeVSVectorZ;

  glm::vec4 eyeWSVectorX;
  glm::vec4 eyeWSVectorY;
  glm::vec4 eyeWSVectorZ;

  glm::vec4 data0;

  glm::vec4 camPos;

  glm::mat4 shadowViewProjMatrix[_INTR_MAX_SHADOW_MAP_COUNT];

  glm::vec4 nearFar;
  glm::vec4 nearFarWidthHeight;

  glm::vec4 haltonSamples;
} _perInstanceData;

glm::mat4 prevViewProjMatrix;

ImageRef _volLightingBufferImageRef;
ImageRef _volLightingBufferPrevFrameImageRef;
ImageRef _volLightingScatteringBufferImageRef;
ImageRef _shadowBufferExp;
ImageRef _shadowBufferExpPingPong;

RenderPassRef _renderPassRef;
FramebufferRefArray _framebufferRefs;
FramebufferRefArray _framebufferPingPongRefs;

PipelineRef _pipelineAccumRef;
PipelineRef _pipelineScatteringRef;

DrawCallRef _drawCallEsmGenerateRef;
DrawCallRef _drawCallEsmBlurRef;
DrawCallRef _drawCallEsmBlurPingPongRef;

ComputeCallRef _computeCallAccumRef;
ComputeCallRef _computeCallAccumPrevFrameRef;
ComputeCallRef _computeCallScatteringRef;
ComputeCallRef _computeCallScatteringPrevFrameRef;

_INTR_INLINE ComputeCallRef createComputeCallScattering(
    glm::vec3 p_Dim, BufferRef p_CurrentVolLightingBuffer)
{
  ComputeCallRef computeCallScatteringRef =
      ComputeCallManager::createComputeCall(_N(VolumetricLighting));
  {
    ComputeCallManager::resetToDefault(computeCallScatteringRef);
    ComputeCallManager::addResourceFlags(
        computeCallScatteringRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallScatteringRef) =
        glm::uvec3(Math::divideByMultiple(p_Dim.x, 8u),
                   Math::divideByMultiple(p_Dim.y, 8u), 1u);
    ComputeCallManager::_descPipeline(computeCallScatteringRef) =
        _pipelineScatteringRef;

    ComputeCallManager::bindBuffer(
        computeCallScatteringRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceData));
    ComputeCallManager::bindImage(
        computeCallScatteringRef, _N(volLightScatterBufferTex),
        GpuProgramType::kCompute, _volLightingScatteringBufferImageRef,
        Samplers::kInvalidSampler);
  }

  return computeCallScatteringRef;
}
}

// Static members
float VolumetricLighting::_globalScatteringFactor = 1.0f;

void VolumetricLighting::init()
{
  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  DrawCallRefArray drawCallsToCreate;

  // Pipeline layouts
  PipelineLayoutRef pipelineLayoutAccum;
  PipelineLayoutRef pipelineLayoutScattering;
  PipelineLayoutRef pipelineLayoutEsm;
  {
    {
      pipelineLayoutScattering = PipelineLayoutManager::createPipelineLayout(
          _N(VolumetricLightingScattering));
      PipelineLayoutManager::resetToDefault(pipelineLayoutScattering);

      GpuProgramManager::reflectPipelineLayout(
          8u,
          {GpuProgramManager::getResourceByName(
              "volumetric_lighting_scattering.comp")},
          pipelineLayoutScattering);
    }
    pipelineLayoutsToCreate.push_back(pipelineLayoutScattering);
  }

  // Pipeline
  PipelineRef pipelineEsmGenerateRef;
  PipelineRef pipelineEsmBlurRef;
  {
    {
      _pipelineScatteringRef =
          PipelineManager::createPipeline(_N(VolumetricLighting));
      PipelineManager::resetToDefault(_pipelineScatteringRef);

      PipelineManager::_descComputeProgram(_pipelineScatteringRef) =
          GpuProgramManager::getResourceByName(
              "volumetric_lighting_scattering.comp");
      PipelineManager::_descPipelineLayout(_pipelineScatteringRef) =
          pipelineLayoutScattering;
    }
    pipelinesToCreate.push_back(_pipelineScatteringRef);
  }

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  PipelineManager::createResources(pipelinesToCreate);

  ImageRefArray imgsToCreate;
  ComputeCallRefArray computeCallsToCreate;

  const glm::uvec3 computeDim = glm::uvec3(160u, 90u, 128u);

  // Images
  {
    _volLightingScatteringBufferImageRef =
        ImageManager::createImage(_N(VolumetricLightingScatteringBuffer));
    {
      ImageManager::resetToDefault(_volLightingScatteringBufferImageRef);
      ImageManager::addResourceFlags(
          _volLightingScatteringBufferImageRef,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      ImageManager::_descDimensions(_volLightingScatteringBufferImageRef) =
          computeDim;
      ImageManager::_descImageFormat(_volLightingScatteringBufferImageRef) =
          Format::kR16G16B16A16Float;
      ImageManager::_descImageType(_volLightingScatteringBufferImageRef) =
          ImageType::kTexture;
      ImageManager::_descImageFlags(_volLightingScatteringBufferImageRef) =
          ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
    }
    imgsToCreate.push_back(_volLightingScatteringBufferImageRef);
  }
  ImageManager::createResources(imgsToCreate);

  // Draw calls
  DrawCallManager::createResources(drawCallsToCreate);

  // Compute calls
  {
    // Scattering
    _computeCallScatteringRef =
        createComputeCallScattering(computeDim, _volLightingBufferImageRef);

    computeCallsToCreate.push_back(_computeCallScatteringRef);
  }
  ComputeCallManager::createResources(computeCallsToCreate);
}

// <-

void VolumetricLighting::onReinitRendering() {}

// <-

void VolumetricLighting::destroy() {}

// <-

void VolumetricLighting::render(float p_DeltaT, CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Render Volumetric Lighting");
  _INTR_PROFILE_GPU("Render Volumetric Lighting");

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  ComputeCallRef scatteringComputeCalltoUse = _computeCallScatteringRef;

  {
    RenderSystem::dispatchComputeCall(scatteringComputeCalltoUse,
                                      primaryCmdBuffer);
  }
}
}
}
}
