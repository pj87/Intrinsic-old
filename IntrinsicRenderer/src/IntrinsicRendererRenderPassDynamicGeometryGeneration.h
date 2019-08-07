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

#define _IS_OVERRIDEN_MESH(name) Intrinsic::Renderer::RenderPass::DynamicGeometryGeneration::isOverridenMesh(name)

namespace Intrinsic
{
namespace Renderer
{
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
  BufferRef _voxelNormalBufferRef;
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

  static float getVoxel(DynamicGeneratedMesh& mesh, int x, int y, int z);
  static glm::vec3& getNormal(DynamicGeneratedMesh& mesh, int x, int y, int z);
  static void getNormal(DynamicGeneratedMesh& mesh);

  static bool isOverridenMesh(const Name&);

  static void init();
  static void onReinitRendering();

  static void postInit();
  static void destroy();

  static void render(float p_DeltaT, Components::CameraRef p_CameraRef);
};
}
}
}
