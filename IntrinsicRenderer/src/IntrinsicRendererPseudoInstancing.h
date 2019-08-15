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

#pragma once

#include <random>
#include <algorithm>

#define _IS_INSTANCED_MESH(name)                                               \
  Intrinsic::Renderer::PseudoInstancing::isInstancedMesh(name)

#define _INSTANCED_MESH_SIZE(name)                                               \
  Intrinsic::Renderer::PseudoInstancing::getMeshSizes(name)

typedef Dod::Ref BufferRef;
typedef _INTR_ARRAY(BufferRef) BufferRefArray;

namespace Intrinsic
{
namespace Renderer
{

struct Voxel
{
  float x, y, z;
};

struct PseudoInstancing
{
public:
  static void addPseudoInstancingMesh(Name&&, const unsigned&, 
	  const unsigned&, const float&);

  static bool isInstancedMesh(const Name&);

  static std::tuple<
      std::unique_ptr<Name>, unsigned int, unsigned int, unsigned int, float>&
	  getMeshSizes(const Name&);

  static void addInstancedBufferRef(BufferRef bufferRef);

  static void addTempBuffer(uint16_t *tempBuffer);

  static void update();

  template <class Iter>
  static void fillWithRandomFloatValues(Iter start, Iter end, float probability)
  {
    static std::random_device rd;  // you only need to initialize it once
    static std::mt19937 mte(rd()); // this is a relative big object to create

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::generate(start, end, [&]() { return dist(mte) < probability; });
  }

  static std::vector<Voxel> voxels;
  static std::vector<Voxel> normals;

private:
  static std::vector<std::tuple<std::unique_ptr<Name>, unsigned int,
                                unsigned int, unsigned int, float>>
      meshes;

  static std::vector<std::tuple<std::unique_ptr<Name>, unsigned int,
                                unsigned int, unsigned int, float>>&
	  getMeshes();

  //static int getIndex(int x, int z, int sizeX, int numMeshVertices);

  static int getMeshIndicesRange(int x, int z);

  static BufferRef vertexBufferRef;
  static uint16_t *tempBufferRef;
};
/*
namespace RenderPass
{

// Forward declarations
typedef Dod::Ref BufferRef;
typedef _INTR_ARRAY(BufferRef) BufferRefArray;
typedef Dod::Ref ImageRef;
typedef _INTR_ARRAY(ImageRef) ImageRefArray;
typedef Dod::Ref PipelineRef;
typedef _INTR_ARRAY(PipelineRef) PipelineRefArray;
typedef Dod::Ref ComputeCallRef;
typedef _INTR_ARRAY(ComputeCallRef) ComputeCallRefArray;

struct DynamicGeneratedMesh
{
  DynamicGeneratedMesh(const int& sizeX, const int& sizeY, 
					   const int& sizeZ, const Name& meshName,
                       const Name&& voxelGenerationShader,
                       const Name&& normalGenerationShader, 
					   const Name&& geometryGenerationShader, 
					   bool isDynamic)
  {
    this->meshName = std::make_unique<Name>(meshName);
    this->isDynamic = isDynamic;

	sizes[0] = sizeX;
    sizes[1] = sizeY;
    sizes[2] = sizeZ;
    sizes[3] = 1;

    // redundant
    this->sizeX = &sizes[0];
    this->sizeY = &sizes[1];
    this->sizeZ = &sizes[2];

    shaders.push_back(std::make_unique<Name>(voxelGenerationShader));
    shaders.push_back(std::make_unique<Name>(normalGenerationShader));
    shaders.push_back(std::make_unique<Name>(geometryGenerationShader));
  }

  std::vector<std::unique_ptr<Name>> shaders;
  std::unique_ptr<Name> meshName;

  BufferRef _positionBufferRef;
  BufferRef _normalBufferRef;
  BufferRef _binormalBufferRef;
  BufferRef _tangentBufferRef;
  BufferRef _colorBufferRef;
  BufferRef _uv0BufferRef;
  BufferRef _debugBufferRef;
  BufferRef _voxelBufferRef;
  BufferRef _cubeEdgeFlagsBufferRef;
  BufferRef _triangleConnectionBufferRef;
  BufferRef _sizesBufferRef;
  BufferRef _targetBufferRef;
  BufferRef _noiseParametersRef;

  ImageRef _normalsImageRef;
  ImageRef _gradient3dImageRef;
  ImageRef _permTable2dImageRef;

  PipelineRef _pipelineScatteringRef;
  PipelineRef _pipelineSDFGenerationRef;
  PipelineRef _pipelineNormalRef;
  
  ComputeCallRef _computeCallMarchingCubesRef;
  ComputeCallRef _computeCallSDFGenerationRef;
  ComputeCallRef _computeCallNormalRef;

  int counter = 0;
  bool isCalled = false;
  bool isDynamic;
  int *sizeX, *sizeY, *sizeZ;
  int sizes[4];
};

struct DynamicGeometryGeneration
{
  static std::vector<std::unique_ptr<DynamicGeneratedMesh>>
      dynamicGenerationMeshes;

  static void addDynamicGeneradtedMesh(const int& sizeX, const int& sizeY, const int& sizeZ,
									   const Name&, const Name&&, const Name&&,
                                       const Name&&, bool isDynamic = true);

  static bool isOverridenMesh(const Name&);
  static bool isInstancedMesh(const Name&);

  static void init();
  static void onReinitRendering();

  static void postInit();
  static void destroy();

  static void render(float p_DeltaT, Components::CameraRef p_CameraRef);
};
}
*/
}
}
