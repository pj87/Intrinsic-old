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

#include <array>
#include <iterator>

// PhysX includes
#include "PxPhysics.h"
#include "PxScene.h"
#include "cooking/PxCooking.h"
#include "common/PxTolerancesScale.h"
#include "extensions/PxDefaultStreams.h"
#include "geometry/PxTriangleMesh.h"
#include "geometry/PxConvexMesh.h"

using namespace RResources;

namespace Intrinsic
{
namespace Core
{
namespace Resources
{
namespace
{

_INTR_INLINE void createOrLoadPhysicsMeshes(MeshRef p_MeshRef)
{
  const _INTR_STRING meshFilePath =
      "media/physics_meshes/" +
      CResources::MeshManager::_name(p_MeshRef).getString() + ".pm";

  if (Util::fileExists(meshFilePath.c_str()))
  {
    physx::PxDefaultFileInputData fileInput =
        physx::PxDefaultFileInputData(meshFilePath.c_str());
    MeshManager::_pxTriangleMesh(p_MeshRef) =
        Physics::System::_pxPhysics->createTriangleMesh(fileInput);
  }

  const _INTR_STRING convexMeshFilePath =
      "media/physics_meshes/" +
      CResources::MeshManager::_name(p_MeshRef).getString() + ".pcm";

  if (Util::fileExists(convexMeshFilePath.c_str()))
  {
    physx::PxDefaultFileInputData fileInput =
        physx::PxDefaultFileInputData(convexMeshFilePath.c_str());
    MeshManager::_pxConvexMesh(p_MeshRef) =
        Physics::System::_pxPhysics->createConvexMesh(fileInput);
  }
}
}

void MeshManager::init()
{
  _INTR_LOG_INFO("Inititializing Mesh Manager...");

  Dod::Resources::ResourceManagerBase<
      MeshData, _INTR_MAX_MESH_COUNT>::_initResourceManager();

  Dod::Resources::ResourceManagerEntry managerEntry;
  {
    managerEntry.createFunction = MeshManager::createMesh;
    managerEntry.destroyFunction = MeshManager::destroyMesh;
    managerEntry.createResourcesFunction = MeshManager::createResources;
    managerEntry.destroyResourcesFunction = MeshManager::destroyResources;
    managerEntry.getActiveResourceAtIndexFunction =
        MeshManager::getActiveResourceAtIndex;
    managerEntry.getActiveResourceCountFunction =
        MeshManager::getActiveResourceCount;
    managerEntry.loadFromMultipleFilesFunction =
        MeshManager::loadFromMultipleFiles;
    managerEntry.saveToMultipleFilesFunction = MeshManager::saveToMultipleFiles;
    managerEntry.getResourceFlagsFunction = MeshManager::_resourceFlags;
    managerEntry.onPropertyUpdateFinishedFunction =
        MeshManager::updateDependentResources;

    Application::_resourceManagerMapping[_N(Mesh)] = managerEntry;
  }

  Dod::PropertyCompilerEntry propertyEntry;
  {
    propertyEntry.compileFunction = MeshManager::compileDescriptor;
    propertyEntry.initFunction = MeshManager::initFromDescriptor;
    propertyEntry.ref = Dod::Ref();

    Application::_resourcePropertyCompilerMapping[_N(Mesh)] = propertyEntry;
  }

  _defaultResourceName = _N(cube);
}

// <-

_INTR_INLINE void MeshManager::updateDependentResources(MeshRef p_Ref)
{
  Components::MeshRefArray componentsToRecreate;

  // Update mesh components using this mesh
  for (uint32_t i = 0u; i < CComponents::MeshManager::getActiveResourceCount();
       ++i)
  {
    Components::MeshRef meshCompRef =
        CComponents::MeshManager::getActiveResourceAtIndex(i);

    if (CComponents::MeshManager::_descMeshName(meshCompRef) ==
        MeshManager::_name(p_Ref))
    {
      componentsToRecreate.push_back(meshCompRef);
    }
  }

  CComponents::MeshManager::destroyResources(componentsToRecreate);
  CComponents::MeshManager::createResources(componentsToRecreate);
}

int32_t MeshManager::getMeshInitialBufferNumber(const MeshRefArray& p_Meshes,
                                                Name& name)
{
  for (uint32_t meshIdx = 0u; meshIdx < p_Meshes.size(); ++meshIdx)
  {
    MeshRef meshRef = p_Meshes[meshIdx];

    Name& meshName = MeshManager::_name(meshRef);
    _INTR_LOG_INFO("PJ: Mesh Name: %s", meshName.getString().c_str());

    if (meshName == name)
      return meshIdx;
  }
  return -1;
}

void MeshManager::createGeneratedResources(const MeshRefArray& p_Meshes)
{
  // Create vertex/index buffers - we're using a separate buffer for each vertex
  // attribute
  BufferRefArray buffersToCreate;
  _INTR_ARRAY(void*) tempBuffersToRelease;

  for (uint32_t meshIdx = 0u; meshIdx < p_Meshes.size(); ++meshIdx)
  {
    MeshRef meshRef = p_Meshes[meshIdx];

	const Name& name = _name(meshRef);        
    if (!_IS_OVERRIDEN_MESH(name))
      continue;

    const PositionsPerSubMeshArray& positions =
        _descPositionsPerSubMesh(meshRef);
    const UVsPerSubMeshArray& uv0s = _descUV0sPerSubMesh(meshRef);
    const IndicesPerSubMeshArray& indices = _descIndicesPerSubMesh(meshRef);
    const NormalsPerSubMeshArray& normals = _descNormalsPerSubMesh(meshRef);
    const TangentsPerSubMeshArray& tangents = _descTangentsPerSubMesh(meshRef);
    const BinormalsPerSubMeshArray& binormals =
        _descBinormalsPerSubMesh(meshRef);
    const VertexColorsPerSubMeshArray& vtxColors =
        _descVertexColorsPerSubMesh(meshRef);
    VertexBuffersPerSubMeshArray& vertexBuffers =
        _vertexBuffersPerSubMesh(meshRef);
    IndexBufferPerSubMeshArray& indexBuffers = _indexBufferPerSubMesh(meshRef);

    void* tempIndexBuffer = nullptr;
    const uint32_t subMeshCount = (uint32_t)positions.size();
    vertexBuffers.resize(subMeshCount);
    indexBuffers.resize(subMeshCount);
    _aabbPerSubMesh(meshRef).resize(subMeshCount);

    _INTR_LOG_INFO("%s", name.getString().c_str());

    for (uint32_t subMeshIdx = 0u; subMeshIdx < subMeshCount; ++subMeshIdx)
    {
      // Build AABB
      {
        Math::AABB& aabb = _aabbPerSubMesh(meshRef)[subMeshIdx];
        Math::initAABB(aabb);

        for (uint32_t posIdx = 0u; posIdx < positions[subMeshIdx].size();
             ++posIdx)
        {
          Math::mergePointToAABB(aabb, positions[subMeshIdx][posIdx]);
        }
      }

      BufferRef posVertexBuffer =
          BufferManager::createBuffer(_N(MeshPositionVb));
      {
        BufferManager::resetToDefault(posVertexBuffer);

        BufferManager::addResourceFlags(
            posVertexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(posVertexBuffer) =
            R::BufferType::kVertex;

        BufferManager::_nameToInitlialBufferMap[name] =
            BufferManager::_dynamicBuffers.size();
        BufferManager::_descSizeInBytes(posVertexBuffer) =
            20000000 * sizeof(uint32_t);
        BufferManager::_dynamicBuffers.push_back(posVertexBuffer);
        
        vertexBuffers[subMeshIdx].push_back(posVertexBuffer);
      }

      BufferRef uv0VertexBuffer = BufferManager::createBuffer(_N(MeshUv0Vb));
      {
        BufferManager::resetToDefault(uv0VertexBuffer);

        BufferManager::addResourceFlags(
            uv0VertexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(uv0VertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(uv0VertexBuffer) =
            20000000 * sizeof(uint32_t);
        BufferManager::_dynamicBuffers.push_back(uv0VertexBuffer);
        
        vertexBuffers[subMeshIdx].push_back(uv0VertexBuffer);
      }

      BufferRef normalVertexBuffer =
          BufferManager::createBuffer(_N(MeshNormalVb));
      {
        BufferManager::resetToDefault(normalVertexBuffer);

        BufferManager::addResourceFlags(
            normalVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(normalVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(normalVertexBuffer) =
            20000000 * sizeof(uint32_t);
        BufferManager::_dynamicBuffers.push_back(normalVertexBuffer);
        
        vertexBuffers[subMeshIdx].push_back(normalVertexBuffer);
      }

      BufferRef tangentVertexBuffer =
          BufferManager::createBuffer(_N(MeshTangentVb));
      {
        BufferManager::resetToDefault(tangentVertexBuffer);

        BufferManager::addResourceFlags(
            tangentVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(tangentVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(tangentVertexBuffer) =
            20000000 * sizeof(uint32_t);
        BufferManager::_dynamicBuffers.push_back(tangentVertexBuffer);
        
        vertexBuffers[subMeshIdx].push_back(tangentVertexBuffer);
      }

      BufferRef binormalVertexBuffer =
          BufferManager::createBuffer(_N(MeshBinormalVb));
      {
        BufferManager::resetToDefault(binormalVertexBuffer);

        BufferManager::addResourceFlags(
            binormalVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(binormalVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(binormalVertexBuffer) =
            20000000 * sizeof(uint32_t);
        BufferManager::_dynamicBuffers.push_back(binormalVertexBuffer);
        
        vertexBuffers[subMeshIdx].push_back(binormalVertexBuffer);
      }

      BufferRef vtxColorVertexBuffer =
          BufferManager::createBuffer(_N(MeshVtxColorVb));
      {
        BufferManager::resetToDefault(vtxColorVertexBuffer);

        BufferManager::addResourceFlags(
            vtxColorVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(vtxColorVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(vtxColorVertexBuffer) =
            20000000 * sizeof(uint32_t);
        BufferManager::_dynamicBuffers.push_back(vtxColorVertexBuffer);

        vertexBuffers[subMeshIdx].push_back(vtxColorVertexBuffer);
      }
    }

    createOrLoadPhysicsMeshes(meshRef);
  }

  BufferManager::createResources(BufferManager::_dynamicBuffers);
}

void MeshManager::createInstancedResources(const MeshRefArray& p_Meshes)
{
  // Create vertex/index buffers - we're using a separate buffer for each vertex
  // attribute
  
  BufferRefArray buffersToCreate;
  _INTR_ARRAY(void*) tempBuffersToRelease;

  // iteruj po wszystkich meshach (tak jak jest teraz)
  // jeœli mesh jest na liœcie (czyli jest zapisana jego nazwa)
  // pobierz odpowiednie wartosci rozmiarow dla danego mesha

  float posX = 0.0f;
  float posZ = 0.0f;

  for (uint32_t meshIdx = 0u; meshIdx < p_Meshes.size(); ++meshIdx)
  {
    MeshRef meshRef = p_Meshes[meshIdx];

    const Name& name = _name(meshRef);
    if (!_IS_INSTANCED_MESH(name))
      continue;

	const std::tuple<std::unique_ptr<Name>, unsigned int, unsigned int, float>&
        instancedMesh = _INSTANCED_MESH_SIZE(name);

	const unsigned sizeX = std::get<1>(instancedMesh);
    const unsigned sizeY = std::get<2>(instancedMesh);
    const float probability = std::get<3>(instancedMesh);

    std::vector<int> a(sizeX * sizeY);

    Intrinsic::Renderer::PseudoInstancing::fillWithRandomFloatValues(a.begin(), a.end(), probability);

    for (auto& i : a)
      _INTR_LOG_WARNING("%d", i);

	_INTR_LOG_INFO("instanced mesh: x: %d y: %d", sizeX, sizeY);

	const PositionsPerSubMeshArray& positions =
        _descPositionsPerSubMesh(meshRef);
    const UVsPerSubMeshArray& uv0s = _descUV0sPerSubMesh(meshRef);
    const IndicesPerSubMeshArray& indices = _descIndicesPerSubMesh(meshRef);
    const NormalsPerSubMeshArray& normals = _descNormalsPerSubMesh(meshRef);
    const TangentsPerSubMeshArray& tangents = _descTangentsPerSubMesh(meshRef);
    const BinormalsPerSubMeshArray& binormals =
        _descBinormalsPerSubMesh(meshRef);
    const VertexColorsPerSubMeshArray& vtxColors =
        _descVertexColorsPerSubMesh(meshRef);
    VertexBuffersPerSubMeshArray& vertexBuffers =
        _vertexBuffersPerSubMesh(meshRef);
    IndexBufferPerSubMeshArray& indexBuffers = _indexBufferPerSubMesh(meshRef);

    void* tempIndexBuffer = nullptr;
    const uint32_t subMeshCount = (uint32_t)positions.size();
    vertexBuffers.resize(subMeshCount);
    indexBuffers.resize(subMeshCount);
    _aabbPerSubMesh(meshRef).resize(subMeshCount);

    _INTR_LOG_INFO("%s", name.getString().c_str());

    // Tree_Tall_01_8

    for (uint32_t subMeshIdx = 0u; subMeshIdx < subMeshCount; ++subMeshIdx)
    {
      // Build AABB
      {
        Math::AABB& aabb = _aabbPerSubMesh(meshRef)[subMeshIdx];
        Math::initAABB(aabb);

        for (uint32_t posIdx = 0u; posIdx < positions[subMeshIdx].size();
             ++posIdx)
        {
          Math::mergePointToAABB(aabb, positions[subMeshIdx][posIdx]);
        }
      }

      BufferRef posVertexBuffer =
          BufferManager::createBuffer(_N(MeshPositionVb));
      {
        BufferManager::resetToDefault(posVertexBuffer);

        BufferManager::addResourceFlags(
            posVertexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(posVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(posVertexBuffer) =
            (uint32_t)positions[subMeshIdx].size() * sizeof(uint16_t) * 4u *
            sizeX * sizeY;

		Intrinsic::Renderer::PseudoInstancing::addInstancedBufferRef(
            posVertexBuffer);

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(posVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

		_INTR_LOG_WARNING("Positions: %d", positions[subMeshIdx].size());

		uint32_t len = positions[subMeshIdx].size();

		_INTR_LOG_WARNING("len: %d", len);
        _INTR_LOG_WARNING("len * 2u: %d", len * 2u);		

        for (uint32_t i = 0u; i < positions[subMeshIdx].size(); ++i)
        {
          for (uint32_t j = 0; j < sizeX * sizeY; ++j)
          {
            //posZ = positions[subMeshIdx][i].z + static_cast<float>(j % 10) * 5.0;
            posZ = static_cast<float>(j / sizeX) * 5.0;
            posX = static_cast<float>(j % sizeY) * 5.0;
            //posZ = positions[subMeshIdx][i].z;
            //+static_cast<float>(j) * 15.0;
            /*
            if (j % 10 == 0)
            {
              posZ = positions[subMeshIdx][i].z;
            }
			else
			{
              posZ += static_cast<float>(j) * 5.0;
			}
			*/

            uint32_t packedPosition0 = glm::packHalf2x16(glm::vec2(
                positions[subMeshIdx][i].x + posX,
                positions[subMeshIdx][i].y));
            uint32_t packedPosition1 = glm::packHalf2x16(glm::vec2(
				positions[subMeshIdx][i].z + posZ, 0.0f));
            /*
			_INTR_LOG_WARNING("x: %f, y: %f, z: %f",
                              positions[subMeshIdx][i].x + posX, 
							  positions[subMeshIdx][i].y, 
							  positions[subMeshIdx][i].z + posZ);
			*/
            tempBuffer[(len * j + i) * 3u] = packedPosition0;
            tempBuffer[(len * j + i) * 3u + 1u] = packedPosition0 >> 16u;
            tempBuffer[(len * j + i) * 3u + 2u] = packedPosition1;
          }
        }
        BufferManager::_descInitialData(posVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(posVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(posVertexBuffer);
      }

      BufferRef uv0VertexBuffer = BufferManager::createBuffer(_N(MeshUv0Vb));
      {
        BufferManager::resetToDefault(uv0VertexBuffer);

        BufferManager::addResourceFlags(
            uv0VertexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(uv0VertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(uv0VertexBuffer) =
            (uint32_t)uv0s[subMeshIdx].size() * sizeof(uint16_t) * 2u * sizeX * sizeY;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(uv0VertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

		uint32_t len = uv0s[subMeshIdx].size();

        for (uint32_t i = 0u; i < uv0s[subMeshIdx].size(); ++i)
        {
          uint32_t packedUv = glm::packHalf2x16(uv0s[subMeshIdx][i]);

          for (uint32_t j = 0; j < sizeX * sizeY; ++j)
          {
            tempBuffer[(len * j + i) * 2u] = packedUv;
            tempBuffer[(len * j + i) * 2u + 1u] = packedUv >> 16u;
          }
        }
        BufferManager::_descInitialData(uv0VertexBuffer) = tempBuffer;

        buffersToCreate.push_back(uv0VertexBuffer);
        vertexBuffers[subMeshIdx].push_back(uv0VertexBuffer);
      }

      BufferRef normalVertexBuffer =
          BufferManager::createBuffer(_N(MeshNormalVb));
      {
        BufferManager::resetToDefault(normalVertexBuffer);

        BufferManager::addResourceFlags(
            normalVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(normalVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(normalVertexBuffer) =
            (uint32_t)normals[subMeshIdx].size() * sizeof(uint16_t) * 4u * sizeX * sizeY;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(normalVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

		uint32_t len = normals[subMeshIdx].size();

        for (uint32_t i = 0u; i < normals[subMeshIdx].size(); ++i)
        {
          uint32_t packedNormal0 = glm::packHalf2x16(
              glm::vec2(normals[subMeshIdx][i].x, normals[subMeshIdx][i].y));
          uint32_t packedNormal1 =
              glm::packHalf2x16(glm::vec2(normals[subMeshIdx][i].z, 0.0f));

          for (uint32_t j = 0; j < sizeX * sizeY; ++j)
          {
            tempBuffer[(len * j + i) * 3u] = packedNormal0;
            tempBuffer[(len * j + i) * 3u + 1u] = packedNormal0 >> 16u;
            tempBuffer[(len * j + i) * 3u + 2u] = packedNormal1;
          }
        }
        BufferManager::_descInitialData(normalVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(normalVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(normalVertexBuffer);
      }

      BufferRef tangentVertexBuffer =
          BufferManager::createBuffer(_N(MeshTangentVb));
      {
        BufferManager::resetToDefault(tangentVertexBuffer);

        BufferManager::addResourceFlags(
            tangentVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(tangentVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(tangentVertexBuffer) =
            (uint32_t)tangents[subMeshIdx].size() * sizeof(uint16_t) * 4u *
            sizeX * sizeY;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(tangentVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

		uint32_t len = tangents[subMeshIdx].size();

        for (uint32_t i = 0u; i < tangents[subMeshIdx].size(); ++i)
        {
          uint32_t packedTangent0 = glm::packHalf2x16(
              glm::vec2(tangents[subMeshIdx][i].x, tangents[subMeshIdx][i].y));
          uint32_t packedTangent1 =
              glm::packHalf2x16(glm::vec2(tangents[subMeshIdx][i].z, 0.0f));

          for (uint32_t j = 0; j < sizeX * sizeY; ++j)
          {
            tempBuffer[(len * j + i) * 3u] = packedTangent0;
            tempBuffer[(len * j + i) * 3u + 1u] = packedTangent0 >> 16u;
            tempBuffer[(len * j + i) * 3u + 2u] = packedTangent1;
		  }
        }
        BufferManager::_descInitialData(tangentVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(tangentVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(tangentVertexBuffer);
      }

      BufferRef binormalVertexBuffer =
          BufferManager::createBuffer(_N(MeshBinormalVb));
      {
        BufferManager::resetToDefault(binormalVertexBuffer);

        BufferManager::addResourceFlags(
            binormalVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(binormalVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(binormalVertexBuffer) =
            (uint32_t)binormals[subMeshIdx].size() * sizeof(uint16_t) * 4u *
            sizeX * sizeY;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(binormalVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

		uint32_t len = binormals[subMeshIdx].size();

        for (uint32_t i = 0u; i < binormals[subMeshIdx].size(); ++i)
        {
          uint32_t packedBinormal0 = glm::packHalf2x16(glm::vec2(
              binormals[subMeshIdx][i].x, binormals[subMeshIdx][i].y));
          uint32_t packedBinormal1 =
              glm::packHalf2x16(glm::vec2(binormals[subMeshIdx][i].z, 0.0f));

		  for (uint32_t j = 0; j < sizeX * sizeY; ++j)
          {
            tempBuffer[(len * j + i) * 3u] = packedBinormal0;
            tempBuffer[(len * j + i) * 3u + 1u] = packedBinormal0 >> 16u;
            tempBuffer[(len * j + i) * 3u + 2u] = packedBinormal1;
		  }
        }
        BufferManager::_descInitialData(binormalVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(binormalVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(binormalVertexBuffer);
      }

      BufferRef vtxColorVertexBuffer =
          BufferManager::createBuffer(_N(MeshVtxColorVb));
      {
        BufferManager::resetToDefault(vtxColorVertexBuffer);

        BufferManager::addResourceFlags(
            vtxColorVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(vtxColorVertexBuffer) =
            R::BufferType::kVertex;

        BufferManager::_descSizeInBytes(vtxColorVertexBuffer) =
            (uint32_t)vtxColors[subMeshIdx].size() * sizeof(uint32_t) * sizeX * sizeY;

        // Convert color
        uint32_t* tempBuffer = (uint32_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(vtxColorVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

		uint32_t len = vtxColors[subMeshIdx].size();

        for (uint32_t i = 0u; i < vtxColors[subMeshIdx].size(); ++i)
        {
          for (uint32_t j = 0; j < sizeX * sizeY; ++j)
          {
            tempBuffer[i + len * j] =
                Math::convertColorToBGRA(vtxColors[subMeshIdx][i]);
          }
        }
        BufferManager::_descInitialData(vtxColorVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(vtxColorVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(vtxColorVertexBuffer);
      }

      BufferRef indexBuffer = BufferManager::createBuffer(_N(MeshIb));
      {
        BufferManager::resetToDefault(indexBuffer);

        BufferManager::addResourceFlags(
            indexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

        if (indices[subMeshIdx].size() * sizeX * sizeY <= 0xFFFF)
        {
          uint32_t indexBufferSizeInBytes =
              (uint16_t)indices[subMeshIdx].size() * sizeof(uint16_t) * sizeX * sizeY;
          uint16_t* tempIndexBuffer =
              (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
                  indexBufferSizeInBytes);
          tempBuffersToRelease.push_back(tempIndexBuffer);

          uint32_t len = positions[subMeshIdx].size();

          for (uint32_t i = 0u; i < indices[subMeshIdx].size(); ++i)
          {
            for (int j = 0; j < sizeX * sizeY; ++j)
            {
              tempIndexBuffer[i + indices[subMeshIdx].size() * j] =
                  (uint16_t)indices[subMeshIdx][i] + len * j;
            }
          }

          BufferManager::_descBufferType(indexBuffer) = R::BufferType::kIndex16;
          BufferManager::_descSizeInBytes(indexBuffer) = indexBufferSizeInBytes;
          BufferManager::_descInitialData(indexBuffer) = tempIndexBuffer;
        }
        else
        {
          uint32_t indexBufferSizeInBytes =
              (uint32_t)indices[subMeshIdx].size() * sizeof(uint32_t) * sizeX * sizeY;
          uint32_t* tempIndexBuffer =
              (uint32_t*)Memory::Tlsf::MainAllocator::allocate(
                  indexBufferSizeInBytes);
          tempBuffersToRelease.push_back(tempIndexBuffer);

          uint32_t len = positions[subMeshIdx].size();

          for (uint32_t i = 0u; i < indices[subMeshIdx].size(); ++i)
          {
            for (int j = 0; j < sizeX * sizeY; ++j)
            {
              if (a[j] == 0)
              {
                tempIndexBuffer[i + indices[subMeshIdx].size() * j] = 0;
                continue;
              }

              tempIndexBuffer[i + indices[subMeshIdx].size() * j] =
                  (uint32_t)indices[subMeshIdx][i] + len * j;
            }
          }

          BufferManager::_descBufferType(indexBuffer) = R::BufferType::kIndex32;
          BufferManager::_descSizeInBytes(indexBuffer) = indexBufferSizeInBytes;
          BufferManager::_descInitialData(indexBuffer) = tempIndexBuffer;
        }
        buffersToCreate.push_back(indexBuffer);
        indexBuffers[subMeshIdx] = indexBuffer;
      }
    }

    createOrLoadPhysicsMeshes(meshRef);
  }

  BufferManager::createResources(buffersToCreate);

  for (uint32_t i = 0u; i < tempBuffersToRelease.size(); ++i)
  {
    Memory::Tlsf::MainAllocator::free(tempBuffersToRelease[i]);
  }
  tempBuffersToRelease.clear();
}


void MeshManager::createResources(const MeshRefArray& p_Meshes)
{
  // Create vertex/index buffers - we're using a separate buffer for each vertex
  // attribute
  BufferRefArray buffersToCreate;
  _INTR_ARRAY(void*) tempBuffersToRelease;

  for (uint32_t meshIdx = 0u; meshIdx < p_Meshes.size(); ++meshIdx)
  {
    MeshRef meshRef = p_Meshes[meshIdx];

	const Name& name = _name(meshRef);
    if (_IS_OVERRIDEN_MESH(name))
      continue;
    if (_IS_INSTANCED_MESH(name))
      continue;

    const PositionsPerSubMeshArray& positions =
        _descPositionsPerSubMesh(meshRef);
    const UVsPerSubMeshArray& uv0s = _descUV0sPerSubMesh(meshRef);
    const IndicesPerSubMeshArray& indices = _descIndicesPerSubMesh(meshRef);
    const NormalsPerSubMeshArray& normals = _descNormalsPerSubMesh(meshRef);
    const TangentsPerSubMeshArray& tangents = _descTangentsPerSubMesh(meshRef);
    const BinormalsPerSubMeshArray& binormals =
        _descBinormalsPerSubMesh(meshRef);
    const VertexColorsPerSubMeshArray& vtxColors =
        _descVertexColorsPerSubMesh(meshRef);
    VertexBuffersPerSubMeshArray& vertexBuffers =
        _vertexBuffersPerSubMesh(meshRef);
    IndexBufferPerSubMeshArray& indexBuffers = _indexBufferPerSubMesh(meshRef);

    void* tempIndexBuffer = nullptr;
    const uint32_t subMeshCount = (uint32_t)positions.size();
    vertexBuffers.resize(subMeshCount);
    indexBuffers.resize(subMeshCount);
    _aabbPerSubMesh(meshRef).resize(subMeshCount);

    _INTR_LOG_INFO("%s", name.getString().c_str());

	//Tree_Tall_01_8

    for (uint32_t subMeshIdx = 0u; subMeshIdx < subMeshCount; ++subMeshIdx)
    {
      // Build AABB
      {
        Math::AABB& aabb = _aabbPerSubMesh(meshRef)[subMeshIdx];
        Math::initAABB(aabb);

        for (uint32_t posIdx = 0u; posIdx < positions[subMeshIdx].size();
             ++posIdx)
        {
          Math::mergePointToAABB(aabb, positions[subMeshIdx][posIdx]);
        }
      }

      BufferRef posVertexBuffer =
          BufferManager::createBuffer(_N(MeshPositionVb));
      {
        BufferManager::resetToDefault(posVertexBuffer);

        BufferManager::addResourceFlags(
            posVertexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(posVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(posVertexBuffer) =
            (uint32_t)positions[subMeshIdx].size() * sizeof(uint16_t) * 4u;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(posVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < positions[subMeshIdx].size(); ++i)
        {
          uint32_t packedPosition0 = glm::packHalf2x16(glm::vec2(
              positions[subMeshIdx][i].x, positions[subMeshIdx][i].y));
          uint32_t packedPosition1 =
              glm::packHalf2x16(glm::vec2(positions[subMeshIdx][i].z, 0.0f));

          tempBuffer[i * 3u] = packedPosition0;
          tempBuffer[i * 3u + 1u] = packedPosition0 >> 16u;
          tempBuffer[i * 3u + 2u] = packedPosition1;
        }
        BufferManager::_descInitialData(posVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(posVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(posVertexBuffer);
      }

      BufferRef uv0VertexBuffer = BufferManager::createBuffer(_N(MeshUv0Vb));
      {
        BufferManager::resetToDefault(uv0VertexBuffer);

        BufferManager::addResourceFlags(
            uv0VertexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(uv0VertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(uv0VertexBuffer) =
            (uint32_t)uv0s[subMeshIdx].size() * sizeof(uint16_t) * 2u;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(uv0VertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < uv0s[subMeshIdx].size(); ++i)
        {
          uint32_t packedUv = glm::packHalf2x16(uv0s[subMeshIdx][i]);

          tempBuffer[i * 2u] = packedUv;
          tempBuffer[i * 2u + 1u] = packedUv >> 16u;
        }
        BufferManager::_descInitialData(uv0VertexBuffer) = tempBuffer;

        buffersToCreate.push_back(uv0VertexBuffer);
        vertexBuffers[subMeshIdx].push_back(uv0VertexBuffer);
      }

      BufferRef normalVertexBuffer =
          BufferManager::createBuffer(_N(MeshNormalVb));
      {
        BufferManager::resetToDefault(normalVertexBuffer);

        BufferManager::addResourceFlags(
            normalVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(normalVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(normalVertexBuffer) =
            (uint32_t)normals[subMeshIdx].size() * sizeof(uint16_t) * 4u;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(normalVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < normals[subMeshIdx].size(); ++i)
        {
          uint32_t packedNormal0 = glm::packHalf2x16(
              glm::vec2(normals[subMeshIdx][i].x, normals[subMeshIdx][i].y));
          uint32_t packedNormal1 =
              glm::packHalf2x16(glm::vec2(normals[subMeshIdx][i].z, 0.0f));

          tempBuffer[i * 3u] = packedNormal0;
          tempBuffer[i * 3u + 1u] = packedNormal0 >> 16u;
          tempBuffer[i * 3u + 2u] = packedNormal1;
        }
        BufferManager::_descInitialData(normalVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(normalVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(normalVertexBuffer);
      }

      BufferRef tangentVertexBuffer =
          BufferManager::createBuffer(_N(MeshTangentVb));
      {
        BufferManager::resetToDefault(tangentVertexBuffer);

        BufferManager::addResourceFlags(
            tangentVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(tangentVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(tangentVertexBuffer) =
            (uint32_t)tangents[subMeshIdx].size() * sizeof(uint16_t) * 4u;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(tangentVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < tangents[subMeshIdx].size(); ++i)
        {
          uint32_t packedTangent0 = glm::packHalf2x16(
              glm::vec2(tangents[subMeshIdx][i].x, tangents[subMeshIdx][i].y));
          uint32_t packedTangent1 =
              glm::packHalf2x16(glm::vec2(tangents[subMeshIdx][i].z, 0.0f));

          tempBuffer[i * 3u] = packedTangent0;
          tempBuffer[i * 3u + 1u] = packedTangent0 >> 16u;
          tempBuffer[i * 3u + 2u] = packedTangent1;
        }
        BufferManager::_descInitialData(tangentVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(tangentVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(tangentVertexBuffer);
      }

      BufferRef binormalVertexBuffer =
          BufferManager::createBuffer(_N(MeshBinormalVb));
      {
        BufferManager::resetToDefault(binormalVertexBuffer);

        BufferManager::addResourceFlags(
            binormalVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(binormalVertexBuffer) =
            R::BufferType::kVertex;
        BufferManager::_descSizeInBytes(binormalVertexBuffer) =
            (uint32_t)binormals[subMeshIdx].size() * sizeof(uint16_t) * 4u;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(binormalVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < binormals[subMeshIdx].size(); ++i)
        {
          uint32_t packedBinormal0 = glm::packHalf2x16(glm::vec2(
              binormals[subMeshIdx][i].x, binormals[subMeshIdx][i].y));
          uint32_t packedBinormal1 =
              glm::packHalf2x16(glm::vec2(binormals[subMeshIdx][i].z, 0.0f));

          tempBuffer[i * 3u] = packedBinormal0;
          tempBuffer[i * 3u + 1u] = packedBinormal0 >> 16u;
          tempBuffer[i * 3u + 2u] = packedBinormal1;
        }
        BufferManager::_descInitialData(binormalVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(binormalVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(binormalVertexBuffer);
      }

      BufferRef vtxColorVertexBuffer =
          BufferManager::createBuffer(_N(MeshVtxColorVb));
      {
        BufferManager::resetToDefault(vtxColorVertexBuffer);

        BufferManager::addResourceFlags(
            vtxColorVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(vtxColorVertexBuffer) =
            R::BufferType::kVertex;

        BufferManager::_descSizeInBytes(vtxColorVertexBuffer) =
            (uint32_t)vtxColors[subMeshIdx].size() * sizeof(uint32_t);

        // Convert color
        uint32_t* tempBuffer = (uint32_t*)Memory::Tlsf::MainAllocator::allocate(
            BufferManager::_descSizeInBytes(vtxColorVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < vtxColors[subMeshIdx].size(); ++i)
        {
          tempBuffer[i] = Math::convertColorToBGRA(vtxColors[subMeshIdx][i]);
        }
        BufferManager::_descInitialData(vtxColorVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(vtxColorVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(vtxColorVertexBuffer);
      }

      BufferRef indexBuffer = BufferManager::createBuffer(_N(MeshIb));
      {
        BufferManager::resetToDefault(indexBuffer);

        BufferManager::addResourceFlags(
            indexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

        if (indices[subMeshIdx].size() <= 0xFFFF)
        {
          uint32_t indexBufferSizeInBytes =
              (uint16_t)indices[subMeshIdx].size() * sizeof(uint16_t) * 2u;
          uint16_t* tempIndexBuffer =
              (uint16_t*)Memory::Tlsf::MainAllocator::allocate(
                  indexBufferSizeInBytes);
          tempBuffersToRelease.push_back(tempIndexBuffer);

          for (uint32_t i = 0u; i < indices[subMeshIdx].size(); ++i)
          {
            tempIndexBuffer[i] = (uint16_t)indices[subMeshIdx][i];
          }

          BufferManager::_descBufferType(indexBuffer) = R::BufferType::kIndex16;
          BufferManager::_descSizeInBytes(indexBuffer) = indexBufferSizeInBytes;
          BufferManager::_descInitialData(indexBuffer) = tempIndexBuffer;
        }
        else
        {
          BufferManager::_descBufferType(indexBuffer) = R::BufferType::kIndex32;
          BufferManager::_descSizeInBytes(indexBuffer) =
              (uint32_t)indices[subMeshIdx].size() * sizeof(uint32_t) * 2u;
          BufferManager::_descInitialData(indexBuffer) =
              (void*)indices[subMeshIdx].data();
        }

        buffersToCreate.push_back(indexBuffer);
        indexBuffers[subMeshIdx] = indexBuffer;
      }
    }

    createOrLoadPhysicsMeshes(meshRef);
  }

  BufferManager::createResources(buffersToCreate);

  for (uint32_t i = 0u; i < tempBuffersToRelease.size(); ++i)
  {
    Memory::Tlsf::MainAllocator::free(tempBuffersToRelease[i]);
  }
  tempBuffersToRelease.clear();
}

// <-

void MeshManager::destroyResources(const MeshRefArray& p_Meshes)
{
  BufferRefArray buffersToDestroy;

  for (uint32_t i = 0u; i < p_Meshes.size(); ++i)
  {
    MeshRef meshRef = p_Meshes[i];

    VertexBuffersPerSubMeshArray& subMeshVtxBuffers =
        _vertexBuffersPerSubMesh(meshRef);
    IndexBufferPerSubMeshArray& subMeshIdxBuffers =
        _indexBufferPerSubMesh(meshRef);

    for (uint32_t i = 0u; i < subMeshVtxBuffers.size(); ++i)
    {
      for (uint32_t j = 0u; j < subMeshVtxBuffers[i].size(); ++j)
      {
        BufferRef bufferRef = subMeshVtxBuffers[i][j];
        _INTR_ASSERT(bufferRef.isValid());

        buffersToDestroy.push_back(bufferRef);
      }
    }

    for (uint32_t i = 0u; i < subMeshIdxBuffers.size(); ++i)
    {
      BufferRef bufferRef = subMeshIdxBuffers[i];
      _INTR_ASSERT(bufferRef.isValid());

      buffersToDestroy.push_back(bufferRef);
    }

    _vertexBuffersPerSubMesh(meshRef).clear();
    _indexBufferPerSubMesh(meshRef).clear();

    if (_pxTriangleMesh(meshRef) != nullptr)
    {
      _pxTriangleMesh(meshRef)->release();
      _pxTriangleMesh(meshRef) = nullptr;
    }

    if (_pxConvexMesh(meshRef) != nullptr)
    {
      _pxConvexMesh(meshRef)->release();
      _pxConvexMesh(meshRef) = nullptr;
    }
  }

  BufferManager::destroyResources(buffersToDestroy);

  for (uint32_t i = 0u; i < buffersToDestroy.size(); ++i)
  {
    BufferManager::destroyBuffer(buffersToDestroy[i]);
  }
}
}
}
}
