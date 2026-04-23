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
#include <random>
#include <fstream>

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
BufferRef _voxelBufferRef;
BufferRef _sizeBufferRef;
BufferRef _noiseParametersRef;

ImageRef _normalsImageRef;
ImageRef _gradient3dImageRef;
ImageRef _permTable2dImageRef;

PipelineRef _pipelinePerlinRef;
PipelineRef _pipelineNormalRef;

ComputeCallRef _computeCallPerlinRef;
ComputeCallRef _computeCallNormalRef;

const int N = 64;

int sizes[] = {N, N};
float noiseParams[] = {0.02, 2.0, 0.5};

_INTR_INLINE ComputeCallRef createComputeCallPerlin(glm::vec3 p_Dim)
{
  ComputeCallRef computeCallPerlinRef =
      ComputeCallManager::createComputeCall(_N(PerlinNoiseGeneration));
  {
    ComputeCallManager::resetToDefault(computeCallPerlinRef);
    ComputeCallManager::addResourceFlags(
        computeCallPerlinRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallPerlinRef) =
        glm::uvec3(8u, 8u, 8u);
    ComputeCallManager::_descPipeline(computeCallPerlinRef) =
        _pipelinePerlinRef;

    ComputeCallManager::bindBuffer(
        computeCallPerlinRef, _N(_VoxelBuffer), GpuProgramType::kCompute,
        _voxelBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(_voxelBufferRef));
    ComputeCallManager::bindImage(
		computeCallPerlinRef, _N(_Gradient3D),
        GpuProgramType::kCompute, 
		_gradient3dImageRef, Samplers::kNearestRepeat);
    ComputeCallManager::bindImage(
        computeCallPerlinRef, _N(_PermTable2D), GpuProgramType::kCompute,
        _permTable2dImageRef, Samplers::kNearestRepeat);
    ComputeCallManager::bindBuffer(
        computeCallPerlinRef, _N(_SizeBuffer), GpuProgramType::kCompute,
        _sizeBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(_sizeBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPerlinRef, _N(_ParametersBuffer), GpuProgramType::kCompute,
        _noiseParametersRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(_noiseParametersRef));
  }

  return computeCallPerlinRef;
}

_INTR_INLINE ComputeCallRef createComputeCallNormal(glm::vec3 p_Dim)
{
  ComputeCallRef computeCallNormalRef =
      ComputeCallManager::createComputeCall(_N(NormalGeneration));
  {
    ComputeCallManager::resetToDefault(computeCallNormalRef);
    ComputeCallManager::addResourceFlags(
        computeCallNormalRef, 
		Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallNormalRef) =
        glm::uvec3(8u, 8u, 8u);
    ComputeCallManager::_descPipeline(computeCallNormalRef) =
        _pipelineNormalRef;

    ComputeCallManager::bindImage(
		computeCallNormalRef, _N(_NormalTex),
		GpuProgramType::kCompute, _normalsImageRef,
        Samplers::kNearestRepeat);
    ComputeCallManager::bindBuffer(
        computeCallNormalRef, _N(_NoiseBuffer), 
		GpuProgramType::kCompute,
        _voxelBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(_voxelBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallNormalRef, _N(_SizeBuffer), 
		GpuProgramType::kCompute,
        _sizeBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(_sizeBufferRef));
  }

  return computeCallNormalRef;
}

} // namespace

// Static members

void GeometryGeneration::postInit()
{
  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;

  // Pipeline layouts
  PipelineLayoutRef pipelineLayoutPerlin;
  {
    {
      pipelineLayoutPerlin = PipelineLayoutManager::createPipelineLayout(
          _N(PerlinNoiseGeneration));
      PipelineLayoutManager::resetToDefault(pipelineLayoutPerlin);

      GpuProgramManager::reflectPipelineLayout(
          8u, {GpuProgramManager::getResourceByName("perlin_noise_3d.comp")},
          pipelineLayoutPerlin);
    }
    pipelineLayoutsToCreate.push_back(pipelineLayoutPerlin);
  }

  // Pipeline
  {
    {
      _pipelinePerlinRef =
          PipelineManager::createPipeline(_N(PerlinNoiseGeneration));
      PipelineManager::resetToDefault(_pipelinePerlinRef);

      PipelineManager::_descComputeProgram(_pipelinePerlinRef) =
          GpuProgramManager::getResourceByName("perlin_noise_3d.comp");
      PipelineManager::_descPipelineLayout(_pipelinePerlinRef) =
          pipelineLayoutPerlin;
    }
    pipelinesToCreate.push_back(_pipelinePerlinRef);
  }

  // Pipeline layouts
  PipelineLayoutRef pipelineLayoutNormal;
  {
    {
      pipelineLayoutNormal =
          PipelineLayoutManager::createPipelineLayout(_N(NormalGeneration));
      PipelineLayoutManager::resetToDefault(pipelineLayoutNormal);

      GpuProgramManager::reflectPipelineLayout(
          8u, {GpuProgramManager::getResourceByName("normal_generation.comp")},
          pipelineLayoutNormal);
    }
    pipelineLayoutsToCreate.push_back(pipelineLayoutNormal);
  }

  // Pipeline
  {
    {
      _pipelineNormalRef =
          PipelineManager::createPipeline(_N(NormalGeneration));
      PipelineManager::resetToDefault(_pipelineNormalRef);

      PipelineManager::_descComputeProgram(_pipelineNormalRef) =
          GpuProgramManager::getResourceByName("normal_generation.comp");
      PipelineManager::_descPipelineLayout(_pipelineNormalRef) =
          pipelineLayoutNormal;
    }
    pipelinesToCreate.push_back(_pipelineNormalRef);
  }

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  PipelineManager::createResources(pipelinesToCreate);

  ComputeCallRefArray computeCallsToCreate;

  const glm::uvec3 computeDim = glm::uvec3(8u, 8u, 8u);
  {
    // Perlin
    _computeCallPerlinRef = createComputeCallPerlin(computeDim);

    computeCallsToCreate.push_back(_computeCallPerlinRef);
  }

  {
    // Normal
    _computeCallNormalRef = createComputeCallNormal(computeDim);

    computeCallsToCreate.push_back(_computeCallNormalRef);
  }

  ComputeCallManager::createResources(computeCallsToCreate);
}

void GeometryGeneration::init()
{
  // Buffers
  BufferRefArray buffersToCreate;
  {
    _sizeBufferRef = BufferManager::createBuffer(_N(_SizeBuffer));
    {
      BufferManager::resetToDefault(_sizeBufferRef);
      BufferManager::addResourceFlags(
          _sizeBufferRef, 
		  Dod::Resources::ResourceFlags::kResourceVolatile);
      BufferManager::_descBufferType(_sizeBufferRef) = 
		  BufferType::kStorage;
      BufferManager::_descSizeInBytes(_sizeBufferRef) = 
		  2 * sizeof(int);
      BufferManager::_descInitialData(_sizeBufferRef) = 
		  sizes;
    }
    buffersToCreate.push_back(_sizeBufferRef);

    _noiseParametersRef = BufferManager::createBuffer(_N(_ParametersBuffer));
    {
      BufferManager::resetToDefault(_noiseParametersRef);
      BufferManager::addResourceFlags(
          _noiseParametersRef,
          Dod::Resources::ResourceFlags::kResourceVolatile);
      BufferManager::_descBufferType(_noiseParametersRef) =
          BufferType::kStorage;
      BufferManager::_descSizeInBytes(_noiseParametersRef) = 
		  3 * sizeof(float);
      BufferManager::_descInitialData(_noiseParametersRef) =
      	  noiseParams;
    }
    buffersToCreate.push_back(_noiseParametersRef);

    _voxelBufferRef = BufferManager::createBuffer(_N(_Voxels));
    {
      BufferManager::resetToDefault(_voxelBufferRef);
      BufferManager::addResourceFlags(
          _voxelBufferRef, 
		  Dod::Resources::ResourceFlags::kResourceVolatile);
      BufferManager::_descBufferType(_voxelBufferRef) = 
		  BufferType::kStorage;
      BufferManager::_descSizeInBytes(_voxelBufferRef) = 
		  N * N * N * sizeof(float);
    }
    buffersToCreate.push_back(_voxelBufferRef);
  }

  BufferManager::createResources(buffersToCreate);

  ImageRefArray imgsToCreate;

  // Images 
  {
    _gradient3dImageRef =
	  ImageManager::getResourceByName(_N(gradient3d));

	_permTable2dImageRef =
      ImageManager::getResourceByName(_N(perm_table2d));

    _normalsImageRef =
	  ImageManager::createImage(_N(normalsTex));
	{
	  ImageManager::resetToDefault(_normalsImageRef);
	  ImageManager::addResourceFlags(
		_normalsImageRef,
		Dod::Resources::ResourceFlags::kResourceVolatile);

	  ImageManager::_descDimensions(_normalsImageRef) =
        glm::uvec3(64u, 64u, 64u);
	  ImageManager::_descImageFormat(_normalsImageRef) =
		Format::kR16G16B16A16Float;
	  ImageManager::_descImageType(_normalsImageRef) =
		ImageType::kTexture;
	  ImageManager::_descImageFlags(_normalsImageRef) =
		ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
	}
	imgsToCreate.push_back(_normalsImageRef);
  }

  ImageManager::createResources(imgsToCreate);

  VkCommandBuffer initCmd = RenderSystem::beginTemporaryCommandBuffer();
  ImageManager::insertImageMemoryBarrier(
      initCmd, _normalsImageRef,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
  RenderSystem::flushTemporaryCommandBuffer();
}

// <-

void GeometryGeneration::onReinitRendering() {}

// <-

void GeometryGeneration::destroy() {}

// <-

void GeometryGeneration::render(float p_DeltaT, CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Render Volumetric Lighting");
  _INTR_PROFILE_GPU("Render Volumetric Lighting");

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  {
    RenderSystem::dispatchComputeCall(_computeCallNormalRef, primaryCmdBuffer);
  }

  ImageManager::insertImageMemoryBarrier(_normalsImageRef,
                                         VK_IMAGE_LAYOUT_GENERAL,
                                         VK_IMAGE_LAYOUT_GENERAL,
                                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  {
    RenderSystem::dispatchComputeCall(_computeCallPerlinRef,
                                      primaryCmdBuffer);
  }

  BufferManager::insertBufferMemoryBarrier(
      _voxelBufferRef, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
} // namespace RenderPass
} // namespace Renderer
} // namespace Intrinsic
