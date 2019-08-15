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
std::vector<Voxel> PseudoInstancing::voxels;
std::vector<Voxel> PseudoInstancing::normals;

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

void transformPosition(uint16_t* srcBuffer, uint16_t* dstBuffer, unsigned int i, glm::vec3& offset)
{
  uint16_t* src0 = &srcBuffer[i];
  uint16_t* src1 = &srcBuffer[i + 1];
  uint16_t* src2 = &srcBuffer[i + 2];

  float result0 = glm::unpackHalf1x16(*src0);
  float result1 = glm::unpackHalf1x16(*src1);
  float result2 = glm::unpackHalf1x16(*src2);

  glm::vec4 pos = glm::vec4(result0 - offset.x, result1 - offset.y,
                            result2 - offset.z, 1.0);
  
  glm::mat4 rotation(glm::cos(0.5), -glm::sin(0.5), 0.0, 0.0, 
					 glm::sin(0.5),  glm::cos(0.5), 0.0, 0.0, 
					           0.0,            0.0, 1.0, 0.0, 
						   	   0.0,            0.0, 0.0, 1.0);
  
  glm::mat4 translation(1.0, 0.0, 0.0, 0.0, 
						0.0, 1.0, 0.0, 0.0, 
						0.0, 0.0, 1.0, 0.0,
						0.0, 10.0, 0.0, 1.0);

  glm::vec4 result = translation * rotation * pos;
  result += glm::vec4(offset.x, offset.y, offset.z, 0.0);

  uint16_t* dst0 = &dstBuffer[i];
  uint16_t* dst1 = &dstBuffer[i + 1];
  uint16_t* dst2 = &dstBuffer[i + 2];

  *dst0 = glm::packHalf1x16(result.x);
  *dst1 = glm::packHalf1x16(result.y);
  *dst2 = glm::packHalf1x16(result.z);
}

void transformPosition(uint16_t* srcBuffer, uint16_t* dstBuffer, unsigned int i,
                       float angle, glm::vec3& rot, glm::vec3 trans, glm::vec3& offset)
{
  uint16_t* src0 = &srcBuffer[i];
  uint16_t* src1 = &srcBuffer[i + 1];
  uint16_t* src2 = &srcBuffer[i + 2];

  float result0 = glm::unpackHalf1x16(*src0);
  float result1 = glm::unpackHalf1x16(*src1);
  float result2 = glm::unpackHalf1x16(*src2);

  glm::vec4 offset4 = glm::vec4(offset.x, offset.y, offset.z, 0.0);

  glm::vec4 pos = glm::vec4(result0, result1, result2, 1.0) - offset4;

  glm::mat4 rotation = glm::rotate(angle, rot);

  trans *= 0.1f;

  glm::mat4 translation =
      glm::translate(trans);

  glm::vec4 result = translation /* rotation */ * pos;
  result += offset4;

  uint16_t* dst0 = &dstBuffer[i];
  uint16_t* dst1 = &dstBuffer[i + 1];
  uint16_t* dst2 = &dstBuffer[i + 2];

  *dst0 = glm::packHalf1x16(result.x);
  *dst1 = glm::packHalf1x16(result.y);
  *dst2 = glm::packHalf1x16(result.z);
}

/*
void transformPosition(uint16_t* srcBuffer, uint16_t* dstBuffer, unsigned int i,
                       float angle, glm::vec3& rot, glm::vec3& trans, glm::vec3&
					   offset)
{
  uint16_t* dst0 = &dstBuffer[i];
  uint16_t* dst1 = &dstBuffer[i + 1];
  uint16_t* dst2 = &dstBuffer[i + 2];

  *dst0 = glm::packHalf1x16(trans.x);
  *dst1 = glm::packHalf1x16(trans.y);
  *dst2 = glm::packHalf1x16(trans.z);
}
*/

int getIndex(int x, int z, int sizeX, int numMeshVertices)
{
  int index = 3 * numMeshVertices * (z * sizeX + x);

  return index;
}

glm::vec3 getOffset(uint16_t* srcBuffer, int i1, int i2)
{
  uint16_t* src0 = &srcBuffer[i1];
  uint16_t* src1 = &srcBuffer[i1 + 1];
  uint16_t* src2 = &srcBuffer[i1 + 2];

  float srcX = glm::unpackHalf1x16(*src0);
  float srcY = glm::unpackHalf1x16(*src1);
  float srcZ = glm::unpackHalf1x16(*src2);

  uint16_t* dst0 = &srcBuffer[i2];
  uint16_t* dst1 = &srcBuffer[i2 + 1];
  uint16_t* dst2 = &srcBuffer[i2 + 2];

  float dstX = glm::unpackHalf1x16(*dst0);
  float dstY = glm::unpackHalf1x16(*dst1);
  float dstZ = glm::unpackHalf1x16(*dst2);

  return glm::vec3(dstX - srcX, dstY - srcY, dstZ - srcZ);
}

void transformMesh(int x, int y, int sizeX, int numMeshVertices,
                   uint16_t* srcBuffer, uint16_t* dstBuffer, float angle,
                   glm::vec3& rot, glm::vec3& trans)
{
  int ref = getIndex(0, y, sizeX, numMeshVertices);
  int start = getIndex(x, y, sizeX, numMeshVertices);
  int end = getIndex(x + 1, y, sizeX, numMeshVertices);

  glm::vec3 offset = getOffset(srcBuffer, ref, start);

  for (int i = start; i < end; i += 3)
  {
    transformPosition(srcBuffer, dstBuffer, i, angle, rot, trans, offset);
  }
}
/*
void transformMesh(int x, int y, int sizeX, int numMeshVertices,
                   uint16_t* srcBuffer, uint16_t* dstBuffer, float angle,
                   glm::vec3& rot, glm::vec3& trans)
{
  int ref = getIndex(0, y, sizeX, numMeshVertices);
  int start = getIndex(x, y, sizeX, numMeshVertices);
  int end = getIndex(x + 1, y, sizeX, numMeshVertices);

  glm::vec3 offset = getOffset(srcBuffer, ref, start);

  for (int i = start; i < end; i += 3)
  {
    transformPosition(srcBuffer, dstBuffer, i, angle, rot, trans, offset);
  }
}
*/
void PseudoInstancing::update()
{
  uint16_t* _vertexBufferGpuMemory =
      (uint16_t*)BufferManager::getGpuMemory(vertexBufferRef);
  /*
  //int reference = getIndex(0, 0, 10, 24);
  int index = getIndex(1, 0, 10, 24);
  int index1 = getIndex(2, 0, 10, 24);

  glm::vec3 offset = getOffset(tempBufferRef, _vertexBufferGpuMemory, index, index1);

  _INTR_LOG_WARNING("PJ: offset %f %f %f", offset.x, offset.y, offset.z);

  for (int i = index; i < index1; i += 3)
  {
    //updatePosition(_vertexBufferGpuMemory, i, glm::vec3(0.0f, 0.01f, 0.0f));
    //transformPosition(tempBufferRef, _vertexBufferGpuMemory, i,
    //                  offset);
    transformPosition(tempBufferRef, _vertexBufferGpuMemory, i, 0.5,
                      glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 10.0, 0.0),
                      offset);
  }
  */
  
  if (voxels.size() == 0)
    return;

  for (int x = 0; x < 5; x++)
    for (int z = 0; z < 5; z++)
    {
      transformMesh(x, z, 5, 1092, tempBufferRef, _vertexBufferGpuMemory, 0.0,
          glm::vec3(0.0, 0.0, 0.0),
		  glm::vec3(static_cast<float>(x - 2) * 50.0, 0.0,
			  static_cast<float>(z - 2) * 50.0));
    }
  
  /*
  int i = 0;

  for (int x = 0; x < 100; x++)
    for (int z = 0; z < 100; z++)
    {
      transformMesh(x, z, 100, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                    glm::vec3(0.0, 0.0, 0.0),
                    glm::vec3(voxels[i].x, voxels[i].y, voxels[i].z));

	  i++;

      if (i >= voxels.size())
	  {
        x = z = 100;
        break;
	  }
    }
  */
  char buffer[256];

  int j = 0;
  for (int i = 0; i < 500; i ++)
  {
    sprintf(buffer, "PJTerrain%d", i);
    Name name = std::move(buffer);

	//_INTR_LOG_WARNING("%s", name.getString().c_str());

    Entity::EntityRef entityRef = Entity::EntityManager::getEntityByName(name);
    NodeRef nodeRef = NodeManager::getComponentForEntity(entityRef);

    if (nodeRef.isValid())
    {
	  //NodeManager::setSize(nodeRef, glm::vec3(0.025, 0.025, 0.025));
      NodeManager::setSize(nodeRef, glm::vec3(0.01, 0.01, 0.01));

	  glm::vec3 position =
              glm::vec3(127.0 * (voxels[j].x - 32.0),
                        127.0 * (voxels[j].y - 0.5 * (1.0 - normals[j].y)),
                        127.0 * (voxels[j].z - 32.0));

	  /*
	  glm::vec3 position =
              glm::vec3(127.0 * (voxels[j].x - 32.0),
                        127.0 * (voxels[j].y - 0.5 * (1.0 - normals[j].y)),
                        127.0 * (voxels[j].z - 32.0));
	  */

	  NodeManager::setPosition(nodeRef, position);

	  const glm::vec3 euler = glm::vec3(
              normals[j].x * 0.5, normals[j].y * 0.5, normals[j].z * 0.5);

	  NodeManager::setOrientation(nodeRef, glm::quat(euler));

      Components::NodeManager::rebuildTreeAndUpdateTransforms();

	  j += 8;
    }
  }
  
  /*
  transformMesh(0, 0, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0));

  transformMesh(0, 1, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 2.0, 1.0));

  transformMesh(0, 2, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 3.0, 1.0));
  */

  /*
  transformMesh(0, 0, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0));
  
  transformMesh(0, 1, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 2.0));

  transformMesh(0, 2, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 3.0));


  transformMesh(0, 3, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 2.0, 1.0));

  transformMesh(0, 4, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 2.0, 2.0));

  transformMesh(0, 5, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 2.0, 3.0));


  transformMesh(0, 6, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 3.0, 1.0));

  transformMesh(0, 7, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 3.0, 2.0));

  transformMesh(0, 8, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 3.0, 3.0));


  transformMesh(2, 4, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(2.0, 1.0, 1.0));

  transformMesh(2, 5, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(2.0, 1.0, 2.0));

  transformMesh(2, 6, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(2.0, 1.0, 3.0));


  transformMesh(0, 9, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(2.0, 2.0, 1.0));
  
  transformMesh(1, 0, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(2.0, 2.0, 2.0));

  transformMesh(1, 1, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(2.0, 2.0, 3.0));


  transformMesh(1, 2, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(2.0, 3.0, 1.0));

  transformMesh(1, 3, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(2.0, 3.0, 2.0));

  transformMesh(1, 4, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(2.0, 3.0, 3.0));


  transformMesh(1, 5, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(3.0, 1.0, 1.0));

  transformMesh(1, 6, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(3.0, 1.0, 2.0));

  transformMesh(1, 7, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(3.0, 1.0, 3.0));


  transformMesh(1, 8, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(3.0, 2.0, 1.0));

  transformMesh(1, 9, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(3.0, 2.0, 2.0));

  transformMesh(2, 0, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(3.0, 2.0, 3.0));


  transformMesh(2, 1, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(3.0, 3.0, 1.0));

  transformMesh(2, 2, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(3.0, 3.0, 2.0));

  transformMesh(2, 3, 10, 24, tempBufferRef, _vertexBufferGpuMemory, 0.0,
                glm::vec3(0.0, 0.0, 0.0), glm::vec3(3.0, 3.0, 3.0));
  */

  BufferManager::updateResources(
      vertexBufferRef, reinterpret_cast<void*>(_vertexBufferGpuMemory));

  //getMeshIndicesRange(1, 1);
}

} // namespace Renderer
} // namespace Intrinsic