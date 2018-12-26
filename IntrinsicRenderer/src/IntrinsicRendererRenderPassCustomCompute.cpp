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
ImageRef _volLightingScatteringBufferImageRef;

PipelineRef _pipelineScatteringRef;

ComputeCallRef _computeCallScatteringRef;

_INTR_INLINE void
updatePerInstanceData(CameraRef p_CameraRef,
                      ComputeCallRef p_CurrentAccumComputeCallRef)
{
  NodeRef camNodeRef =
      NodeManager::getComponentForEntity(CameraManager::_entity(p_CameraRef));

  // Post effect data
  {
    const glm::vec2 scattering =
        PostEffectManager::_descVolumetricLightingScatteringDayNight(
            PostEffectManager::_blendTargetRef);

    _perInstanceData.data0.x =
        glm::mix(scattering.y, scattering.x, World::_currentDayNightFactor) *
        CustomCompute::_globalScatteringFactor;
    _perInstanceData.data0.z = Clustering::_globalIrradianceFactor;
  }

  _perInstanceData.haltonSamples =
      RenderProcess::UniformManager::_uniformDataSource.haltonSamples32;

  _perInstanceData.prevViewProjMatrix = prevViewProjMatrix;
  prevViewProjMatrix = CameraManager::_viewProjectionMatrix(p_CameraRef);
  _perInstanceData.projMatrix = CameraManager::_projectionMatrix(p_CameraRef);

  _perInstanceData.camPos = glm::vec4(NodeManager::_worldPosition(camNodeRef),
                                      TaskManager::_frameCounter);

  _perInstanceData.eyeVSVectorX = glm::vec4(
      glm::vec3(1.0 / _perInstanceData.projMatrix[0][0], 0.0, 0.0), 0.0);
  _perInstanceData.eyeVSVectorY = glm::vec4(
      glm::vec3(0.0, 1.0 / _perInstanceData.projMatrix[1][1], 0.0), 0.0);
  _perInstanceData.eyeVSVectorZ = glm::vec4(glm::vec3(0.0, 0.0, -1.0), 0.0);
  _perInstanceData.eyeWSVectorX =
      CameraManager::_inverseViewMatrix(p_CameraRef) *
      _perInstanceData.eyeVSVectorX;
  _perInstanceData.eyeWSVectorX.w = TaskManager::_totalTimePassed;
  _perInstanceData.eyeWSVectorY =
      CameraManager::_inverseViewMatrix(p_CameraRef) *
      _perInstanceData.eyeVSVectorY;
  _perInstanceData.eyeWSVectorZ =
      CameraManager::_inverseViewMatrix(p_CameraRef) *
      _perInstanceData.eyeVSVectorZ;

  _perInstanceData.nearFar =
      glm::vec4(CameraManager::_descNearPlane(p_CameraRef),
                CameraManager::_descFarPlane(p_CameraRef), 0.0f, 0.0f);

  Math::FrustumCorners viewSpaceCorners;
  Math::extractFrustumsCorners(
      CameraManager::_inverseProjectionMatrix(p_CameraRef), viewSpaceCorners);

  _perInstanceData.nearFarWidthHeight = glm::vec4(
      viewSpaceCorners.c[3].x - viewSpaceCorners.c[2].x /* Near Width */,
      viewSpaceCorners.c[2].y - viewSpaceCorners.c[1].y /* Near Height */,
      viewSpaceCorners.c[7].x - viewSpaceCorners.c[6].x /* Far Width */,
      viewSpaceCorners.c[6].y - viewSpaceCorners.c[5].y /* Far Height */);

  const _INTR_ARRAY(FrustumRef)& shadowFrustums =
      RenderProcess::Default::_shadowFrustums[p_CameraRef];

  for (uint32_t i = 0u; i < shadowFrustums.size(); ++i)
  {
    FrustumRef shadowFrustumRef = shadowFrustums[i];

    // Transform from camera view space => light proj. space
    _perInstanceData.shadowViewProjMatrix[i] =
        FrustumManager::_viewProjectionMatrix(shadowFrustumRef) *
        CameraManager::_inverseViewMatrix(p_CameraRef);
  }

  ComputeCallManager::updateUniformMemory({p_CurrentAccumComputeCallRef},
                                          &_perInstanceData,
                                          sizeof(PerInstanceData));
}

_INTR_INLINE ComputeCallRef createComputeCallScattering(
    glm::vec3 p_Dim, BufferRef p_CurrentVolLightingBuffer)
{
  ComputeCallRef computeCallScatteringRef =
      ComputeCallManager::createComputeCall(_N(CustomCompute));
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
        computeCallScatteringRef, _N(volLightBufferTex),
        GpuProgramType::kCompute, p_CurrentVolLightingBuffer,
        Samplers::kNearestClamp);
    ComputeCallManager::bindImage(
        computeCallScatteringRef, _N(computeCallBufferTex),
        GpuProgramType::kCompute, _volLightingScatteringBufferImageRef,
        Samplers::kInvalidSampler);
  }

  return computeCallScatteringRef;
}
}

// Static members
float CustomCompute::_globalScatteringFactor = 1.0f;

void CustomCompute::init()
{
  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;

  // Pipeline layouts
  PipelineLayoutRef pipelineLayoutScattering;
  {
    {
      pipelineLayoutScattering = PipelineLayoutManager::createPipelineLayout(
          _N(CustomCompute));
      PipelineLayoutManager::resetToDefault(pipelineLayoutScattering);

      GpuProgramManager::reflectPipelineLayout(
          8u,
          {GpuProgramManager::getResourceByName(
              "custom_compute.comp")},
          pipelineLayoutScattering);
    }
    pipelineLayoutsToCreate.push_back(pipelineLayoutScattering);
  }

  // Pipeline
  {
    {
      _pipelineScatteringRef =
          PipelineManager::createPipeline(_N(CustomCompute));
      PipelineManager::resetToDefault(_pipelineScatteringRef);

      PipelineManager::_descComputeProgram(_pipelineScatteringRef) =
          GpuProgramManager::getResourceByName(
              "custom_compute.comp");
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
    _volLightingBufferImageRef =
        ImageManager::createImage(_N(VolumetricLightingBuffer));
    {
      ImageManager::resetToDefault(_volLightingBufferImageRef);
      ImageManager::addResourceFlags(
          _volLightingBufferImageRef,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      ImageManager::_descDimensions(_volLightingBufferImageRef) = computeDim;
      ImageManager::_descImageFormat(_volLightingBufferImageRef) =
          Format::kR16G16B16A16Float;
      ImageManager::_descImageType(_volLightingBufferImageRef) =
          ImageType::kTexture;
      ImageManager::_descImageFlags(_volLightingBufferImageRef) =
          ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
    }
    imgsToCreate.push_back(_volLightingBufferImageRef);

    _volLightingScatteringBufferImageRef =
        ImageManager::createImage(_N(ComputeCallBuffer));
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

void CustomCompute::onReinitRendering() {}

// <-

void CustomCompute::destroy() {}

// <-

void CustomCompute::render(float p_DeltaT, CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Render Volumetric Lighting");
  _INTR_PROFILE_GPU("Render Volumetric Lighting");

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  ComputeCallRef scatteringComputeCalltoUse = _computeCallScatteringRef;

  // Maybe it will be useful in the future...
  /*
  {
    // Update per instance data
    updatePerInstanceData(p_CameraRef, scatteringComputeCalltoUse);
  }
  */

  ImageManager::insertImageMemoryBarrier(
      _volLightingScatteringBufferImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  {
    RenderSystem::dispatchComputeCall(scatteringComputeCalltoUse,
                                      primaryCmdBuffer);
  }
  ImageManager::insertImageMemoryBarrier(
      _volLightingScatteringBufferImageRef, VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
}
}
}
