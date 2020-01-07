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

_INTR_INLINE ComputeCallRef createComputeCallNormal(
    std::unique_ptr<DynamicGeneratedTexture>& texture, glm::vec3 p_Dim)
{
  ComputeCallRef computeCallNormalRef =
      ComputeCallManager::createComputeCall(_N(NormalGeneration));
  {
    ComputeCallManager::resetToDefault(computeCallNormalRef);
    ComputeCallManager::addResourceFlags(
        computeCallNormalRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallNormalRef) =
        glm::uvec3(p_Dim);
    ComputeCallManager::_descPipeline(computeCallNormalRef) =
        texture->_pipelineNormalRef;

    ComputeCallManager::bindImage(
        computeCallNormalRef, _N(_NormalTex), GpuProgramType::kCompute,
        texture->_normalsImageRef, Samplers::kNearestRepeat);
  }

  return computeCallNormalRef;
}

} // namespace

// Static members

std::vector<std::unique_ptr<DynamicGeneratedTexture>>
    DynamicTextureGeneration::dynamicGenerationTextures;

//std::vector<std::unique_ptr<Name>> pseudoInstancedMeshes;

void DynamicTextureGeneration::postInit()
{
  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  ComputeCallRefArray computeCallsToCreate;

  for (auto& texture : dynamicGenerationTextures)
  {
    // Pipeline layouts
    PipelineLayoutRef pipelineLayoutNormal;
    {
      {
        pipelineLayoutNormal =
            PipelineLayoutManager::createPipelineLayout(_N(NormalGeneration));
        PipelineLayoutManager::resetToDefault(pipelineLayoutNormal);

        GpuProgramManager::reflectPipelineLayout(
            1u, {GpuProgramManager::getResourceByName(*(texture->shaders[1]))},
            pipelineLayoutNormal);
      }
      pipelineLayoutsToCreate.push_back(pipelineLayoutNormal);
    }

    // Pipeline
    {
        PipelineRef _pipelineNormalRef =
            PipelineManager::createPipeline(_N(NormalGeneration));
      {
        PipelineManager::resetToDefault(_pipelineNormalRef);

        PipelineManager::_descComputeProgram(_pipelineNormalRef) =
            GpuProgramManager::getResourceByName(*(texture->shaders[1]));
        PipelineManager::_descPipelineLayout(_pipelineNormalRef) =
            pipelineLayoutNormal;
      }
      texture->_pipelineNormalRef = _pipelineNormalRef;
      pipelinesToCreate.push_back(_pipelineNormalRef);
    }

    const glm::uvec3 computeDim = 
		glm::uvec3(sqrt(texture->sizes[0]), 
				   sqrt(texture->sizes[1]), 
				   sqrt(texture->sizes[2]));
    {
      // Normal
      ComputeCallRef _computeCallNormalRef =
          createComputeCallNormal(texture, computeDim);

	  texture->_computeCallNormalRef = _computeCallNormalRef;
      computeCallsToCreate.push_back(_computeCallNormalRef);
    }
  }

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  PipelineManager::createResources(pipelinesToCreate);
  ComputeCallManager::createResources(computeCallsToCreate);
}

void DynamicTextureGeneration::addDynamicGeneradtedTexture(
    const int& sizeX, const int& sizeY, const int& sizeZ, 
	const Name& textureName, const Name&& voxelGenerationShadera,
    const Name&& normalGenerationShader, const Name&& geometryGenerationShader, 
	bool isDynamic)
{
  std::unique_ptr<DynamicGeneratedTexture> dynamicGenerationTexture =
      std::make_unique<DynamicGeneratedTexture>(
          sizeX, sizeY, sizeZ, 
          std::move(textureName), std::move(voxelGenerationShadera),
          std::move(normalGenerationShader),
          std::move(geometryGenerationShader), 
		  isDynamic);

  dynamicGenerationTextures.push_back(std::move(dynamicGenerationTexture));
}

bool DynamicTextureGeneration::isOverridenTexture(const Name& textureName)
{
  for (auto& texture : dynamicGenerationTextures)
  {
    if ((*texture->textureName) == textureName)
      return true;
  }
  return false;
}

void DynamicTextureGeneration::init()
{
  // Buffers
  BufferRefArray buffersToCreate;
  ImageRefArray imgsToCreate;
    
  for (auto& texture : dynamicGenerationTextures)
  {
      ImageRef _normalsImageRef =
          ImageManager::getResourceByName(_N(terrain_rock));
      {
        //ImageManager::resetToDefault(_normalsImageRef);
        ImageManager::addResourceFlags(
            _normalsImageRef, 
			Dod::Resources::ResourceFlags::kResourceVolatile);

		// hack (for the demo): sqrt in normals texture are only for the
        // fractals
        const Name& name = *(texture->textureName);
        /*
        if (name != _N(pbr_test_0125) && name != _N(pbr_test_025) && name != _N(house))
        {
          ImageManager::_descDimensions(_normalsImageRef) = glm::uvec3(
              sqrt(texture->sizes[0]), sqrt(texture->sizes[1]), sqrt(texture->sizes[2]));
        }
        else
        {
          ImageManager::_descDimensions(_normalsImageRef) =
              glm::uvec3(texture->sizes[0], texture->sizes[1], texture->sizes[2]);
        }
        // end of hack (for the demo)
		*/
        ImageManager::_descImageFormat(_normalsImageRef) =
            Format::kB8G8R8A8UNorm;
        ImageManager::_descImageType(_normalsImageRef) = 
			ImageType::kTexture;
        ImageManager::_descImageFlags(_normalsImageRef) =
            ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
      }
      texture->_normalsImageRef = _normalsImageRef;
      imgsToCreate.push_back(_normalsImageRef);
  }
  
  BufferManager::createResources(buffersToCreate);
  ImageManager::createResources(imgsToCreate);
}

// <-

void DynamicTextureGeneration::onReinitRendering() {}

// <-

void DynamicTextureGeneration::destroy() {}

// <-

_INTR_INLINE static void updateDataMemory(void* p_Data, BufferRef bufferRef, 
										  uint32_t p_Size, uint32_t p_Offset)
{
  // Update staging memory
  {
    memcpy(BufferManager::getGpuMemory(bufferRef), p_Data, p_Size);
  }

  // ... and copy to device
  VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

  VkBufferCopy bufferCopy = {};
  {
    bufferCopy.dstOffset = p_Offset;
    bufferCopy.srcOffset = 0u;
    bufferCopy.size = p_Size;
  }

  vkCmdCopyBuffer(copyCmd, BufferManager::_vkBuffer(bufferRef),
                  BufferManager::_vkBuffer(bufferRef), 1u,
                  &bufferCopy);

  RenderSystem::flushTemporaryCommandBuffer();
}

void DynamicTextureGeneration::render(float p_DeltaT, CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Render Dynamic Geometry Generation");
  _INTR_PROFILE_GPU("Dynamic Geometry Generation");

  for (auto& texture : dynamicGenerationTextures)
  {
    if (texture->isDynamic)
	{
	}
	else
	{
	  if (texture->isCalled && texture->counter > 2)
		continue;
	}

    VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

    {
      RenderSystem::dispatchComputeCall(texture->_computeCallNormalRef,
                                        primaryCmdBuffer);
    }

    ImageManager::insertImageMemoryBarrier(texture->_normalsImageRef, 
										   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
										   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	texture->isCalled = true;
    texture->counter++;
  }
}

} // namespace RenderPass
} // namespace Renderer
} // namespace Intrinsic
