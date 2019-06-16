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
std::vector<
    std::tuple<std::unique_ptr<Name>, unsigned int, unsigned int, float>>
    PseudoInstancing::meshes;

BufferRef PseudoInstancing::vertexBufferRef;

void PseudoInstancing::addPseudoInstancingMesh(Name&& name,
                                               const unsigned& sizeX,
                                               const unsigned& sizeY,
                                               const float& probability)
{
  meshes.push_back(
      std::make_tuple(std::make_unique<Name>(name), sizeX, sizeY, probability));
}

bool PseudoInstancing::isInstancedMesh(const Name& meshName)
{
  for (auto& i : Intrinsic::Renderer::PseudoInstancing::meshes)
  {
    const Name& name = *(std::get<0>(i));

    if (meshName == name)
      return true;
  }

  return false;
}

std::vector<
    std::tuple<std::unique_ptr<Name>, unsigned int, unsigned int, float>>&
PseudoInstancing::getMeshes()
{
  return meshes;
}

std::tuple<std::unique_ptr<Name>, unsigned int, unsigned int, float>&
PseudoInstancing::getMeshSizes(const Name& meshName)
{
  for (auto& i : Intrinsic::Renderer::PseudoInstancing::meshes)
  {
    const Name& name = *(std::get<0>(i));

    if (meshName == name)
      return i;
  }
}

void PseudoInstancing::addInstancedBufferRef(BufferRef bufferRef)
{
  vertexBufferRef = bufferRef;
}

void PseudoInstancing::update()
{
  uint32_t* _vertexBufferGpuMemory =
      (uint32_t*)BufferManager::getGpuMemory(vertexBufferRef);

  for (int i = 0; i < 100; i++)
  {
    glm::vec2 packedPosition0 = glm::unpackHalf2x16(static_cast<unsigned int>(*(_vertexBufferGpuMemory + i)));
    _INTR_LOG_WARNING("%f %f", packedPosition0.x, packedPosition0.y);
  }

  /*
  uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
      BufferManager::_descSizeInBytes(vertexBufferRef));

  for (int i = 0; i < 10; i++)
          for (int j = 0; j < 10; j++)
          {
          
                  
                  uint32_t packedPosition0 = glm::packHalf2x16(
                          glm::vec2(positions[subMeshIdx][i].x + posX,
  positions[subMeshIdx][i].y)); uint32_t packedPosition1 =
                          glm::packHalf2x16(glm::vec2(positions[subMeshIdx][i].z
  + posZ, 0.0f));

                  tempBuffer[(len * j + i) * 3u] = packedPosition0;
                  tempBuffer[(len * j + i) * 3u + 1u] = packedPosition0 >> 16u;
                  tempBuffer[(len * j + i) * 3u + 2u] = packedPosition1;
          }

  BufferManager::_descInitialData(vertexBufferRef) = tempBuffer;
  */
}

} // namespace Renderer
} // namespace Intrinsic