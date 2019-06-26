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
uint16_t* PseudoInstancing::tempBufferRef;

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

void PseudoInstancing::addTempBuffer(uint16_t* tempBuffer)
{ 
  tempBufferRef = tempBuffer;
}

void updatePosition(uint16_t* tempBuffer, unsigned int i, glm::vec3& offset)
{
  uint16_t* pos0 = &tempBuffer[i];
  uint16_t* pos1 = &tempBuffer[i + 1];
  uint16_t* pos2 = &tempBuffer[i + 2];

  float result0 = glm::unpackHalf1x16(*pos0);
  float result1 = glm::unpackHalf1x16(*pos1);
  float result2 = glm::unpackHalf1x16(*pos2);

  glm::vec3 pos = glm::vec3(result0, result1, result2);

  pos += offset;

  *pos0 = glm::packHalf1x16(pos.x);
  *pos1 = glm::packHalf1x16(pos.y);
  *pos2 = glm::packHalf1x16(pos.z);

  //_INTR_LOG_WARNING("0x%X %f %f", *_Position, result0, result1);
}

void rotatePosition(uint16_t* tempBuffer, unsigned int i, glm::vec3& offset)
{
  uint16_t* pos0 = &tempBuffer[i];
  uint16_t* pos1 = &tempBuffer[i + 1];
  uint16_t* pos2 = &tempBuffer[i + 2];

  float result0 = glm::unpackHalf1x16(*pos0);
  float result1 = glm::unpackHalf1x16(*pos1);
  float result2 = glm::unpackHalf1x16(*pos2);

  glm::vec4 pos = glm::vec4(result0, result1, result2, 0.0);

  glm::mat4 matrix(glm::cos(0.1), -glm::sin(0.1), 0.0, 0.0, 
				   glm::sin(0.1),  glm::cos(0.1), 0.0, 0.0, 
				             0.0,            0.0, 1.0, 0.0,
						     0.0,            0.0, 0.0, 1.0);

  glm::vec4 result = matrix * pos;

  //pos += offset;
  //glm::vec3 pos = glm::vec3(result.x, result.y, result.z);

  *pos0 = glm::packHalf1x16(result.x);
  *pos1 = glm::packHalf1x16(result.y);
  *pos2 = glm::packHalf1x16(result.z);

  //_INTR_LOG_WARNING("0x%X %f %f", *_Position, result0, result1);
}

int PseudoInstancing::getMeshIndicesRange(int x, int z) 
{
  int index = z * 10 + x;
  int index1 = 3 * 24 * (z * 10 + x);
  _INTR_LOG_WARNING("index w meshu: %d", index);
  _INTR_LOG_WARNING("index w meshu: %d", index1);

  return index1;
}

int PseudoInstancing::getIndex(int x, int z, int sizeX, int numMeshVertices)
{
  int index = 3 * numMeshVertices * (z * sizeX + x);
  _INTR_LOG_WARNING("index w meshu: %d", index);

  return index;
}

void PseudoInstancing::update()
{
  uint16_t* _vertexBufferGpuMemory =
      (uint16_t*)BufferManager::getGpuMemory(vertexBufferRef);

  //for (int i = 24 * 3; i < 48 * 3; i += 3)
  int index = getIndex(2, 2, 10, 24);
  int index1 = getIndex(3, 2, 10, 24);

  for (int i = index; i < index1; i += 3)
  {
    updatePosition(_vertexBufferGpuMemory, i, glm::vec3(0.0f, 0.01f, 0.0f));
  }

  int index2 = getIndex(0, 0, 10, 24);
  int index3 = getIndex(1, 0, 10, 24);

  for (int i = index2; i < index3; i += 3)
  {
    rotatePosition(_vertexBufferGpuMemory, i, glm::vec3(0.0f, 0.01f, 0.0f));
  }

  BufferManager::updateResources(
      vertexBufferRef, reinterpret_cast<void*>(_vertexBufferGpuMemory));

  //getMeshIndicesRange(1, 1);
}

} // namespace Renderer
} // namespace Intrinsic