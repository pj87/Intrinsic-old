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
uint16_t* PseudoInstancing::vertexBufferTmp;

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

void PseudoInstancing::addInstancedBufferTmp(uint16_t* bufferTmp)
{
  vertexBufferTmp = bufferTmp;
}

BufferRef PseudoInstancing::getInstancedBufferRef() { return vertexBufferRef; }

uint16_t* PseudoInstancing::getInstancedBufferTmp() { return vertexBufferTmp; }

glm::vec2 getPosition(unsigned int i, uint32_t* vertexMemory)
{
  glm::vec2 ret(0.0);

  if (i % 2 == 0)
  {
    unsigned int index = i + i / 2;
    ret = glm::unpackHalf2x16(vertexMemory[index]);
    // glm::vec2 tmp2 = glm::unpackHalf2x16(vertexMemory[index + 1]);

    // ret = glm::vec3(tmp1.x, tmp1.y, tmp1.x);
  }
  else
  {
    unsigned int index = i + (i - 1) / 2;
    ret = glm::unpackHalf2x16(vertexMemory[index + 1]);

    // ret = glm::vec3(tmp1.y, tmp2.x, tmp2.y);
  }

  return ret;
}

void storePosition(unsigned int i, glm::vec3 pos, uint16_t* tempBuffer)
{
  /*
  if (i % 2 == 0)
  {
    uint index = i + i / 2;
    _Positions[index] = glm::packHalf2x16(pos.xy);
    vec2 tmp = glm::unpackHalf2x16(_Positions[index + 1]);
    _Positions[index + 1] = glm::packHalf2x16(vec2(pos.z, tmp.y));
  }
  else
  {
    uint index = i + (i - 1) / 2;
    vec2 tmp = glm::unpackHalf2x16(_Positions[index]);
    _Positions[index] = glm::packHalf2x16(vec2(tmp.x, pos.x));
    _Positions[index + 1] = glm::packHalf2x16(vec2(pos.y, pos.z));
  }
  */
}
/*
int getIndex(int x, int z, int len)
{
        return
}
*/
void PseudoInstancing::update()
{
  //uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
  //    BufferManager::_descSizeInBytes(vertexBufferRef));

  uint16_t* tempBuffer =
      Intrinsic::Renderer::PseudoInstancing::getInstancedBufferTmp();

  _INTR_LOG_WARNING("vertexBufferRef size: %d, vertices: %d",
                    BufferManager::_descSizeInBytes(vertexBufferRef),
                    BufferManager::_descSizeInBytes(vertexBufferRef) / 8);

  uint16_t* _vertexBufferGpuMemory =
      (uint16_t*)BufferManager::getGpuMemory(vertexBufferRef);

  // for (int i = 0; i < 216; i++)
  for (int i = 0; i < BufferManager::_descSizeInBytes(vertexBufferRef) / 8; i++)
  {
    // glm::vec2 unpackedPosition0 = getPosition(i, _vertexBufferGpuMemory);
    // glm::vec2 unpackedPosition = glm::unpackHalf2x16(static_cast<unsigned
    // int>(*(_vertexBufferGpuMemory + i))); _INTR_LOG_WARNING("%f %f 0X%x",
    //unpackedPosition0.x,
    //                  unpackedPosition0.y,
    //                  *(_vertexBufferGpuMemory + i));

    //_INTR_LOG_WARNING("0x%X", *(_vertexBufferGpuMemory + i));

    //tempBuffer[i] = *(_vertexBufferGpuMemory + i);

    // unpackedPosition0.y += 1.0;
    /*
        glm::vec2 pos1 = glm::vec2(unpackedPosition0.x, unpackedPosition0.y);
    glm::vec2 pos2 = glm::vec2(unpackedPosition0.z, 0.0f);

        uint32_t packedPosition0 = glm::packHalf2x16(pos1);
    uint32_t packedPosition1 = glm::packHalf2x16(pos2);

        tempBuffer[3 * i] = packedPosition0;
    tempBuffer[3 * i + 1] = packedPosition0 >> 16;
    tempBuffer[3 * i + 2] = packedPosition1;

    //tempBuffer[i] = rand() % 0xFFFF;
    //tempBuffer[i] = 0;
        */
    tempBuffer[i] = 0;
    // tempBuffer[i] = rand() % 0xFFFF;
  }

  BufferManager::updateResources(BufferManager::_dynamicBuffers,
                                 reinterpret_cast<void*>(tempBuffer));

  _INTR_LOG_WARNING("------ Odczytuje po zapisie ----------");

  for (int i = 0; i < BufferManager::_descSizeInBytes(vertexBufferRef) / 8; i++)
  {
    _INTR_LOG_WARNING("0x%X", tempBuffer[i]);
  }

  _INTR_LOG_WARNING("------ Koniec odczytu po zapise ----------");

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