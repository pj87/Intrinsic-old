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

glm::vec2 getPositionVec2(unsigned int i, uint16_t* tempBuffer)
{
  uint32_t* _Positions = reinterpret_cast<uint32_t*>(tempBuffer);

  return glm::unpackHalf2x16(_Positions[i]);
}

glm::vec3 getPositionVec3(unsigned int i, uint16_t* tempBuffer)
{
  uint16_t* pos0 = &tempBuffer[i];
  uint16_t* pos1 = &tempBuffer[i + 1];
  uint16_t* pos2 = &tempBuffer[i + 2];

  float result0 = glm::unpackHalf1x16(*pos0);
  float result1 = glm::unpackHalf1x16(*pos1);
  float result2 = glm::unpackHalf1x16(*pos2);

  _INTR_LOG_WARNING("%f %f %f", result0, result1, result2);

  return glm::vec3(result0, result1, result2);
}

glm::vec4 getPositionVec4(unsigned int i, uint16_t* tempBuffer)
{
  uint64_t* _Positions = reinterpret_cast<uint64_t*>(tempBuffer);

  return glm::unpackHalf4x16(_Positions[i]);
}

uint32_t getPositionPacked(unsigned int i, uint16_t* tempBuffer, const char *qqqq)
{
  uint16_t* pos0 = &tempBuffer[i];
  uint16_t* pos1 = &tempBuffer[i + 1];

  uint32_t* _Position = reinterpret_cast<uint32_t*>(pos0);

  float result0 = glm::unpackHalf1x16(*pos0);
  float result1 = glm::unpackHalf1x16(*pos1);

  _INTR_LOG_WARNING("0x%X %f %f", *_Position, result0, result1);
  
  return *_Position;

  /*
  if (i % 2 == 0)
  {
    unsigned int index = i + i / 2;
    glm::vec2 tmp = glm::unpackHalf2x16(_Positions[index]);
    glm::vec2 tmp2 = glm::unpackHalf2x16(_Positions[index + 1]);

	ret = glm::vec3(tmp.x, tmp.y, tmp2.x);
  }
  else
  {
    unsigned int index = i + (i - 1) / 2;

	glm::vec2 tmp = glm::unpackHalf2x16(_Positions[index]);
    glm::vec2 tmp2 = glm::unpackHalf2x16(_Positions[index + 1]);

	ret = glm::vec3(tmp.x, tmp2.x, tmp2.y);
  }

  return ret;
  */
}

void updatePosition(unsigned int i, uint16_t* tempBuffer)
{
  uint16_t* pos0 = &tempBuffer[i];
  uint16_t* pos1 = &tempBuffer[i + 1];

  uint32_t* _Position = reinterpret_cast<uint32_t*>(pos0);

  float result0 = glm::unpackHalf1x16(*pos0);
  float result1 = glm::unpackHalf1x16(*pos1);

  result0 += 0.01f;

  *pos0 = glm::packHalf1x16(result0);

  //_INTR_LOG_WARNING("0x%X %f %f", *_Position, result0, result1);
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

  for (int i = 0; i < 24 * 3; i += 3)
  {
    //updatePosition(i, tempBuffer);
    //updatePosition(i, _vertexBufferGpuMemory);
    updatePosition(_vertexBufferGpuMemory, i, glm::vec3(0.0f, 0.01f, 0.0f));
  }

  BufferManager::updateResources(BufferManager::_dynamicBuffers,
      reinterpret_cast<void*>(_vertexBufferGpuMemory));

  _INTR_LOG_WARNING("------ Odczytuje po zapisie ----------");

  for (int i = 0; i < 24 * 3; i += 2)
  {
    //_INTR_LOG_WARNING("0x%X", tempBuffer[i]);

    uint32_t dupa = getPositionPacked(i, tempBuffer, "tempBuffer            ");
    uint32_t dupa1 =
        getPositionPacked(i, _vertexBufferGpuMemory, "_vertexBufferGpuMemory");
  }

  for (int i = 0; i < 24 * 3; i += 3)
  {
	glm::vec3 dupa = getPositionVec3(i, _vertexBufferGpuMemory);
  }

  _INTR_LOG_WARNING("------ Koniec odczytu po zapise ----------");

  /*
  */
}

} // namespace Renderer
} // namespace Intrinsic