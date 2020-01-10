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

_INTR_INLINE ComputeCallRef createComputeCallTexture(
    std::unique_ptr<DynamicGeneratedTexture>& texture, glm::vec3 p_Dim)
{
  ComputeCallRef computeCallTextureRef =
      ComputeCallManager::createComputeCall(_N(TextureGeneration));
  {
    ComputeCallManager::resetToDefault(computeCallTextureRef);
    ComputeCallManager::addResourceFlags(
        computeCallTextureRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallTextureRef) =
        glm::uvec3(p_Dim);
    ComputeCallManager::_descPipeline(computeCallTextureRef) =
        texture->_pipelineTextureRef;

    ComputeCallManager::bindImage(
        computeCallTextureRef, _N(_TextureTex), GpuProgramType::kCompute,
        texture->_textureImageRef, Samplers::kNearestRepeat);
    ComputeCallManager::bindImage(
        computeCallTextureRef, _N(_NormalTex), GpuProgramType::kCompute,
        texture->_normalImageRef, Samplers::kNearestRepeat);
    ComputeCallManager::bindImage(
        computeCallTextureRef, _N(_PBRTex), GpuProgramType::kCompute,
        texture->_pbrImageRef, Samplers::kNearestRepeat);
  }

  return computeCallTextureRef;
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
    PipelineLayoutRef pipelineLayoutTexture;
    {
      {
        pipelineLayoutTexture =
            PipelineLayoutManager::createPipelineLayout(_N(TextureGeneration));
        PipelineLayoutManager::resetToDefault(pipelineLayoutTexture);

        GpuProgramManager::reflectPipelineLayout(
            1u, {GpuProgramManager::getResourceByName(*(texture->shaders[1]))},
            pipelineLayoutTexture);
      }
      pipelineLayoutsToCreate.push_back(pipelineLayoutTexture);
    }

    // Pipeline
    {
        PipelineRef _pipelineTextureRef =
            PipelineManager::createPipeline(_N(TextureGeneration));
      {
        PipelineManager::resetToDefault(_pipelineTextureRef);

        PipelineManager::_descComputeProgram(_pipelineTextureRef) =
            GpuProgramManager::getResourceByName(*(texture->shaders[1]));
        PipelineManager::_descPipelineLayout(_pipelineTextureRef) =
            pipelineLayoutTexture;
      }
      texture->_pipelineTextureRef = _pipelineTextureRef;
      pipelinesToCreate.push_back(_pipelineTextureRef);
    }

    const glm::uvec3 computeDim = 
		glm::uvec3(sqrt(texture->sizes[0]), 
				   sqrt(texture->sizes[1]), 
				   sqrt(texture->sizes[2]));
    {
      // Texture
      ComputeCallRef _computeCallTextureRef =
          createComputeCallTexture(texture, computeDim);

	  texture->_computeCallTextureRef = _computeCallTextureRef;
      computeCallsToCreate.push_back(_computeCallTextureRef);
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
  ImageRefArray imgsToCreate;
    
  for (auto& texture : dynamicGenerationTextures)
  {
      ImageRef _textureImageRef =
          ImageManager::getResourceByName(_N(terrain_rock));
      {
        ImageManager::addResourceFlags(
            _textureImageRef, 
			Dod::Resources::ResourceFlags::kResourceVolatile);
        ImageManager::_descImageFormat(_textureImageRef) =
            Format::kB8G8R8A8UNorm;
        ImageManager::_descImageType(_textureImageRef) = 
			ImageType::kTexture;
        ImageManager::_descImageFlags(_textureImageRef) =
            ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
      }
      texture->_textureImageRef = _textureImageRef;
      imgsToCreate.push_back(_textureImageRef);

	  ImageRef _normalImageRef =
          ImageManager::getResourceByName(_N(terrain_rock_N));
      {
        ImageManager::addResourceFlags(
            _normalImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
        //ImageManager::_descImageFormat(_normalImageRef) =
            // Format::kB8G8R8A8UNorm;
        //ImageManager::_descImageType(_normalImageRef) = ImageType::kTexture;
        //ImageManager::_descImageFlags(_normalImageRef) =
        //    ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
      }
      texture->_normalImageRef = _normalImageRef;
      imgsToCreate.push_back(_normalImageRef);

	  ImageRef _pbrImageRef =
          ImageManager::getResourceByName(_N(terrain_rock_PBR));
      {
        ImageManager::addResourceFlags(
            _pbrImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
        //ImageManager::_descImageFormat(_pbrImageRef) =
        //    Format::kB8G8R8A8UNorm;
        //ImageManager::_descImageType(_pbrImageRef) = ImageType::kTexture;
        //ImageManager::_descImageFlags(_pbrImageRef) =
        //    ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
      }
      texture->_pbrImageRef = _pbrImageRef;
      imgsToCreate.push_back(_pbrImageRef);
  }
  
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
      RenderSystem::dispatchComputeCall(texture->_computeCallTextureRef,
                                        primaryCmdBuffer);
    }

    ImageManager::insertImageMemoryBarrier(texture->_textureImageRef, 
										   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
										   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	ImageManager::insertImageMemoryBarrier(texture->_normalImageRef, 
										   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
										   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	ImageManager::insertImageMemoryBarrier(texture->_pbrImageRef, 
										   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
										   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	texture->isCalled = true;
    texture->counter++;
  }
}

} // namespace RenderPass
} // namespace Renderer
} // namespace Intrinsic
