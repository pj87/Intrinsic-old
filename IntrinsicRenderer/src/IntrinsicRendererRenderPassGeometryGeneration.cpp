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
struct PerInstanceData
{
  glm::mat4 projMatrix;
  glm::mat4 prevViewProjMatrix;

  glm::vec4 eyeVSVectorX;
  glm::vec4 eyeVSVectorY;
  glm::vec4 eyeVSVectorZ;

  glm::vec4 eyeWSVectorX;
  glm::vec4 eyeWSVectorY;
  glm::vec4 eyeWSVectorZ;

  glm::vec4 data0;

  glm::vec4 camPos;

  glm::mat4 shadowViewProjMatrix[_INTR_MAX_SHADOW_MAP_COUNT];

  glm::vec4 nearFar;
  glm::vec4 nearFarWidthHeight;

  glm::vec4 haltonSamples;
} _perInstanceData;

glm::mat4 prevViewProjMatrix;

BufferRef _voxelBuffer;
BufferRef _triangleConnectionBuffer;
BufferRef _positionBuffer;
BufferRef _debugBuffer; 

PipelineRef _pipelineScatteringRef;

ComputeCallRef _computeCallScatteringRef;

BufferRef bufferRef;

typedef struct Position
{
  glm::fvec3 pos;
};

Position* _positionBufferGpuMemory = nullptr;

const int N = 64;
const int SIZE = N * N * N * 3 * 5;

int triangleConnectionTable[4096] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  8,  3,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  1,  9,  -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,  8,  3,  9,  8,  1,  -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, 1,  2,  10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, 0,  8,  3,  1,  2,  10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 9,  2,  10, 0,  2,  9,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2,  8,
    3,  2,  10, 8,  10, 9,  8,  -1, -1, -1, -1, -1, -1, -1, 3,  11, 2,  -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  11, 2,  8,  11, 0,  -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, 1,  9,  0,  2,  3,  11, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, 1,  11, 2,  1,  9,  11, 9,  8,  11, -1, -1, -1, -1, -1,
    -1, -1, 3,  10, 1,  11, 10, 3,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,
    10, 1,  0,  8,  10, 8,  11, 10, -1, -1, -1, -1, -1, -1, -1, 3,  9,  0,  3,
    11, 9,  11, 10, 9,  -1, -1, -1, -1, -1, -1, -1, 9,  8,  10, 10, 8,  11, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 4,  7,  8,  -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, 4,  3,  0,  7,  3,  4,  -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 0,  1,  9,  8,  4,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4,  1,  9,  4,  7,  1,  7,  3,  1,  -1, -1, -1, -1, -1, -1, -1, 1,  2,  10,
    8,  4,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3,  4,  7,  3,  0,  4,
    1,  2,  10, -1, -1, -1, -1, -1, -1, -1, 9,  2,  10, 9,  0,  2,  8,  4,  7,
    -1, -1, -1, -1, -1, -1, -1, 2,  10, 9,  2,  9,  7,  2,  7,  3,  7,  9,  4,
    -1, -1, -1, -1, 8,  4,  7,  3,  11, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 11, 4,  7,  11, 2,  4,  2,  0,  4,  -1, -1, -1, -1, -1, -1, -1, 9,  0,
    1,  8,  4,  7,  2,  3,  11, -1, -1, -1, -1, -1, -1, -1, 4,  7,  11, 9,  4,
    11, 9,  11, 2,  9,  2,  1,  -1, -1, -1, -1, 3,  10, 1,  3,  11, 10, 7,  8,
    4,  -1, -1, -1, -1, -1, -1, -1, 1,  11, 10, 1,  4,  11, 1,  0,  4,  7,  11,
    4,  -1, -1, -1, -1, 4,  7,  8,  9,  0,  11, 9,  11, 10, 11, 0,  3,  -1, -1,
    -1, -1, 4,  7,  11, 4,  11, 9,  9,  11, 10, -1, -1, -1, -1, -1, -1, -1, 9,
    5,  4,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 9,  5,  4,  0,
    8,  3,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  5,  4,  1,  5,  0,  -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 8,  5,  4,  8,  3,  5,  3,  1,  5,  -1,
    -1, -1, -1, -1, -1, -1, 1,  2,  10, 9,  5,  4,  -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 3,  0,  8,  1,  2,  10, 4,  9,  5,  -1, -1, -1, -1, -1, -1, -1,
    5,  2,  10, 5,  4,  2,  4,  0,  2,  -1, -1, -1, -1, -1, -1, -1, 2,  10, 5,
    3,  2,  5,  3,  5,  4,  3,  4,  8,  -1, -1, -1, -1, 9,  5,  4,  2,  3,  11,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  11, 2,  0,  8,  11, 4,  9,  5,
    -1, -1, -1, -1, -1, -1, -1, 0,  5,  4,  0,  1,  5,  2,  3,  11, -1, -1, -1,
    -1, -1, -1, -1, 2,  1,  5,  2,  5,  8,  2,  8,  11, 4,  8,  5,  -1, -1, -1,
    -1, 10, 3,  11, 10, 1,  3,  9,  5,  4,  -1, -1, -1, -1, -1, -1, -1, 4,  9,
    5,  0,  8,  1,  8,  10, 1,  8,  11, 10, -1, -1, -1, -1, 5,  4,  0,  5,  0,
    11, 5,  11, 10, 11, 0,  3,  -1, -1, -1, -1, 5,  4,  8,  5,  8,  10, 10, 8,
    11, -1, -1, -1, -1, -1, -1, -1, 9,  7,  8,  5,  7,  9,  -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, 9,  3,  0,  9,  5,  3,  5,  7,  3,  -1, -1, -1, -1, -1,
    -1, -1, 0,  7,  8,  0,  1,  7,  1,  5,  7,  -1, -1, -1, -1, -1, -1, -1, 1,
    5,  3,  3,  5,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 9,  7,  8,  9,
    5,  7,  10, 1,  2,  -1, -1, -1, -1, -1, -1, -1, 10, 1,  2,  9,  5,  0,  5,
    3,  0,  5,  7,  3,  -1, -1, -1, -1, 8,  0,  2,  8,  2,  5,  8,  5,  7,  10,
    5,  2,  -1, -1, -1, -1, 2,  10, 5,  2,  5,  3,  3,  5,  7,  -1, -1, -1, -1,
    -1, -1, -1, 7,  9,  5,  7,  8,  9,  3,  11, 2,  -1, -1, -1, -1, -1, -1, -1,
    9,  5,  7,  9,  7,  2,  9,  2,  0,  2,  7,  11, -1, -1, -1, -1, 2,  3,  11,
    0,  1,  8,  1,  7,  8,  1,  5,  7,  -1, -1, -1, -1, 11, 2,  1,  11, 1,  7,
    7,  1,  5,  -1, -1, -1, -1, -1, -1, -1, 9,  5,  8,  8,  5,  7,  10, 1,  3,
    10, 3,  11, -1, -1, -1, -1, 5,  7,  0,  5,  0,  9,  7,  11, 0,  1,  0,  10,
    11, 10, 0,  -1, 11, 10, 0,  11, 0,  3,  10, 5,  0,  8,  0,  7,  5,  7,  0,
    -1, 11, 10, 5,  7,  11, 5,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 6,
    5,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  8,  3,  5,  10,
    6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 9,  0,  1,  5,  10, 6,  -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, 1,  8,  3,  1,  9,  8,  5,  10, 6,  -1, -1,
    -1, -1, -1, -1, -1, 1,  6,  5,  2,  6,  1,  -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, 1,  6,  5,  1,  2,  6,  3,  0,  8,  -1, -1, -1, -1, -1, -1, -1, 9,
    6,  5,  9,  0,  6,  0,  2,  6,  -1, -1, -1, -1, -1, -1, -1, 5,  9,  8,  5,
    8,  2,  5,  2,  6,  3,  2,  8,  -1, -1, -1, -1, 2,  3,  11, 10, 6,  5,  -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, 0,  8,  11, 2,  0,  10, 6,  5,  -1,
    -1, -1, -1, -1, -1, -1, 0,  1,  9,  2,  3,  11, 5,  10, 6,  -1, -1, -1, -1,
    -1, -1, -1, 5,  10, 6,  1,  9,  2,  9,  11, 2,  9,  8,  11, -1, -1, -1, -1,
    6,  3,  11, 6,  5,  3,  5,  1,  3,  -1, -1, -1, -1, -1, -1, -1, 0,  8,  11,
    0,  11, 5,  0,  5,  1,  5,  11, 6,  -1, -1, -1, -1, 3,  11, 6,  0,  3,  6,
    0,  6,  5,  0,  5,  9,  -1, -1, -1, -1, 6,  5,  9,  6,  9,  11, 11, 9,  8,
    -1, -1, -1, -1, -1, -1, -1, 5,  10, 6,  4,  7,  8,  -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, 4,  3,  0,  4,  7,  3,  6,  5,  10, -1, -1, -1, -1, -1, -1,
    -1, 1,  9,  0,  5,  10, 6,  8,  4,  7,  -1, -1, -1, -1, -1, -1, -1, 10, 6,
    5,  1,  9,  7,  1,  7,  3,  7,  9,  4,  -1, -1, -1, -1, 6,  1,  2,  6,  5,
    1,  4,  7,  8,  -1, -1, -1, -1, -1, -1, -1, 1,  2,  5,  5,  2,  6,  3,  0,
    4,  3,  4,  7,  -1, -1, -1, -1, 8,  4,  7,  9,  0,  5,  0,  6,  5,  0,  2,
    6,  -1, -1, -1, -1, 7,  3,  9,  7,  9,  4,  3,  2,  9,  5,  9,  6,  2,  6,
    9,  -1, 3,  11, 2,  7,  8,  4,  10, 6,  5,  -1, -1, -1, -1, -1, -1, -1, 5,
    10, 6,  4,  7,  2,  4,  2,  0,  2,  7,  11, -1, -1, -1, -1, 0,  1,  9,  4,
    7,  8,  2,  3,  11, 5,  10, 6,  -1, -1, -1, -1, 9,  2,  1,  9,  11, 2,  9,
    4,  11, 7,  11, 4,  5,  10, 6,  -1, 8,  4,  7,  3,  11, 5,  3,  5,  1,  5,
    11, 6,  -1, -1, -1, -1, 5,  1,  11, 5,  11, 6,  1,  0,  11, 7,  11, 4,  0,
    4,  11, -1, 0,  5,  9,  0,  6,  5,  0,  3,  6,  11, 6,  3,  8,  4,  7,  -1,
    6,  5,  9,  6,  9,  11, 4,  7,  9,  7,  11, 9,  -1, -1, -1, -1, 10, 4,  9,
    6,  4,  10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4,  10, 6,  4,  9,  10,
    0,  8,  3,  -1, -1, -1, -1, -1, -1, -1, 10, 0,  1,  10, 6,  0,  6,  4,  0,
    -1, -1, -1, -1, -1, -1, -1, 8,  3,  1,  8,  1,  6,  8,  6,  4,  6,  1,  10,
    -1, -1, -1, -1, 1,  4,  9,  1,  2,  4,  2,  6,  4,  -1, -1, -1, -1, -1, -1,
    -1, 3,  0,  8,  1,  2,  9,  2,  4,  9,  2,  6,  4,  -1, -1, -1, -1, 0,  2,
    4,  4,  2,  6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8,  3,  2,  8,  2,
    4,  4,  2,  6,  -1, -1, -1, -1, -1, -1, -1, 10, 4,  9,  10, 6,  4,  11, 2,
    3,  -1, -1, -1, -1, -1, -1, -1, 0,  8,  2,  2,  8,  11, 4,  9,  10, 4,  10,
    6,  -1, -1, -1, -1, 3,  11, 2,  0,  1,  6,  0,  6,  4,  6,  1,  10, -1, -1,
    -1, -1, 6,  4,  1,  6,  1,  10, 4,  8,  1,  2,  1,  11, 8,  11, 1,  -1, 9,
    6,  4,  9,  3,  6,  9,  1,  3,  11, 6,  3,  -1, -1, -1, -1, 8,  11, 1,  8,
    1,  0,  11, 6,  1,  9,  1,  4,  6,  4,  1,  -1, 3,  11, 6,  3,  6,  0,  0,
    6,  4,  -1, -1, -1, -1, -1, -1, -1, 6,  4,  8,  11, 6,  8,  -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, 7,  10, 6,  7,  8,  10, 8,  9,  10, -1, -1, -1, -1,
    -1, -1, -1, 0,  7,  3,  0,  10, 7,  0,  9,  10, 6,  7,  10, -1, -1, -1, -1,
    10, 6,  7,  1,  10, 7,  1,  7,  8,  1,  8,  0,  -1, -1, -1, -1, 10, 6,  7,
    10, 7,  1,  1,  7,  3,  -1, -1, -1, -1, -1, -1, -1, 1,  2,  6,  1,  6,  8,
    1,  8,  9,  8,  6,  7,  -1, -1, -1, -1, 2,  6,  9,  2,  9,  1,  6,  7,  9,
    0,  9,  3,  7,  3,  9,  -1, 7,  8,  0,  7,  0,  6,  6,  0,  2,  -1, -1, -1,
    -1, -1, -1, -1, 7,  3,  2,  6,  7,  2,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 2,  3,  11, 10, 6,  8,  10, 8,  9,  8,  6,  7,  -1, -1, -1, -1, 2,  0,
    7,  2,  7,  11, 0,  9,  7,  6,  7,  10, 9,  10, 7,  -1, 1,  8,  0,  1,  7,
    8,  1,  10, 7,  6,  7,  10, 2,  3,  11, -1, 11, 2,  1,  11, 1,  7,  10, 6,
    1,  6,  7,  1,  -1, -1, -1, -1, 8,  9,  6,  8,  6,  7,  9,  1,  6,  11, 6,
    3,  1,  3,  6,  -1, 0,  9,  1,  11, 6,  7,  -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, 7,  8,  0,  7,  0,  6,  3,  11, 0,  11, 6,  0,  -1, -1, -1, -1, 7,
    11, 6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 7,  6,  11, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3,  0,  8,  11, 7,  6,  -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  1,  9,  11, 7,  6,  -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, 8,  1,  9,  8,  3,  1,  11, 7,  6,  -1, -1, -1, -1,
    -1, -1, -1, 10, 1,  2,  6,  11, 7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1,  2,  10, 3,  0,  8,  6,  11, 7,  -1, -1, -1, -1, -1, -1, -1, 2,  9,  0,
    2,  10, 9,  6,  11, 7,  -1, -1, -1, -1, -1, -1, -1, 6,  11, 7,  2,  10, 3,
    10, 8,  3,  10, 9,  8,  -1, -1, -1, -1, 7,  2,  3,  6,  2,  7,  -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, 7,  0,  8,  7,  6,  0,  6,  2,  0,  -1, -1, -1,
    -1, -1, -1, -1, 2,  7,  6,  2,  3,  7,  0,  1,  9,  -1, -1, -1, -1, -1, -1,
    -1, 1,  6,  2,  1,  8,  6,  1,  9,  8,  8,  7,  6,  -1, -1, -1, -1, 10, 7,
    6,  10, 1,  7,  1,  3,  7,  -1, -1, -1, -1, -1, -1, -1, 10, 7,  6,  1,  7,
    10, 1,  8,  7,  1,  0,  8,  -1, -1, -1, -1, 0,  3,  7,  0,  7,  10, 0,  10,
    9,  6,  10, 7,  -1, -1, -1, -1, 7,  6,  10, 7,  10, 8,  8,  10, 9,  -1, -1,
    -1, -1, -1, -1, -1, 6,  8,  4,  11, 8,  6,  -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, 3,  6,  11, 3,  0,  6,  0,  4,  6,  -1, -1, -1, -1, -1, -1, -1, 8,
    6,  11, 8,  4,  6,  9,  0,  1,  -1, -1, -1, -1, -1, -1, -1, 9,  4,  6,  9,
    6,  3,  9,  3,  1,  11, 3,  6,  -1, -1, -1, -1, 6,  8,  4,  6,  11, 8,  2,
    10, 1,  -1, -1, -1, -1, -1, -1, -1, 1,  2,  10, 3,  0,  11, 0,  6,  11, 0,
    4,  6,  -1, -1, -1, -1, 4,  11, 8,  4,  6,  11, 0,  2,  9,  2,  10, 9,  -1,
    -1, -1, -1, 10, 9,  3,  10, 3,  2,  9,  4,  3,  11, 3,  6,  4,  6,  3,  -1,
    8,  2,  3,  8,  4,  2,  4,  6,  2,  -1, -1, -1, -1, -1, -1, -1, 0,  4,  2,
    4,  6,  2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,  9,  0,  2,  3,  4,
    2,  4,  6,  4,  3,  8,  -1, -1, -1, -1, 1,  9,  4,  1,  4,  2,  2,  4,  6,
    -1, -1, -1, -1, -1, -1, -1, 8,  1,  3,  8,  6,  1,  8,  4,  6,  6,  10, 1,
    -1, -1, -1, -1, 10, 1,  0,  10, 0,  6,  6,  0,  4,  -1, -1, -1, -1, -1, -1,
    -1, 4,  6,  3,  4,  3,  8,  6,  10, 3,  0,  3,  9,  10, 9,  3,  -1, 10, 9,
    4,  6,  10, 4,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4,  9,  5,  7,  6,
    11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  8,  3,  4,  9,  5,  11, 7,
    6,  -1, -1, -1, -1, -1, -1, -1, 5,  0,  1,  5,  4,  0,  7,  6,  11, -1, -1,
    -1, -1, -1, -1, -1, 11, 7,  6,  8,  3,  4,  3,  5,  4,  3,  1,  5,  -1, -1,
    -1, -1, 9,  5,  4,  10, 1,  2,  7,  6,  11, -1, -1, -1, -1, -1, -1, -1, 6,
    11, 7,  1,  2,  10, 0,  8,  3,  4,  9,  5,  -1, -1, -1, -1, 7,  6,  11, 5,
    4,  10, 4,  2,  10, 4,  0,  2,  -1, -1, -1, -1, 3,  4,  8,  3,  5,  4,  3,
    2,  5,  10, 5,  2,  11, 7,  6,  -1, 7,  2,  3,  7,  6,  2,  5,  4,  9,  -1,
    -1, -1, -1, -1, -1, -1, 9,  5,  4,  0,  8,  6,  0,  6,  2,  6,  8,  7,  -1,
    -1, -1, -1, 3,  6,  2,  3,  7,  6,  1,  5,  0,  5,  4,  0,  -1, -1, -1, -1,
    6,  2,  8,  6,  8,  7,  2,  1,  8,  4,  8,  5,  1,  5,  8,  -1, 9,  5,  4,
    10, 1,  6,  1,  7,  6,  1,  3,  7,  -1, -1, -1, -1, 1,  6,  10, 1,  7,  6,
    1,  0,  7,  8,  7,  0,  9,  5,  4,  -1, 4,  0,  10, 4,  10, 5,  0,  3,  10,
    6,  10, 7,  3,  7,  10, -1, 7,  6,  10, 7,  10, 8,  5,  4,  10, 4,  8,  10,
    -1, -1, -1, -1, 6,  9,  5,  6,  11, 9,  11, 8,  9,  -1, -1, -1, -1, -1, -1,
    -1, 3,  6,  11, 0,  6,  3,  0,  5,  6,  0,  9,  5,  -1, -1, -1, -1, 0,  11,
    8,  0,  5,  11, 0,  1,  5,  5,  6,  11, -1, -1, -1, -1, 6,  11, 3,  6,  3,
    5,  5,  3,  1,  -1, -1, -1, -1, -1, -1, -1, 1,  2,  10, 9,  5,  11, 9,  11,
    8,  11, 5,  6,  -1, -1, -1, -1, 0,  11, 3,  0,  6,  11, 0,  9,  6,  5,  6,
    9,  1,  2,  10, -1, 11, 8,  5,  11, 5,  6,  8,  0,  5,  10, 5,  2,  0,  2,
    5,  -1, 6,  11, 3,  6,  3,  5,  2,  10, 3,  10, 5,  3,  -1, -1, -1, -1, 5,
    8,  9,  5,  2,  8,  5,  6,  2,  3,  8,  2,  -1, -1, -1, -1, 9,  5,  6,  9,
    6,  0,  0,  6,  2,  -1, -1, -1, -1, -1, -1, -1, 1,  5,  8,  1,  8,  0,  5,
    6,  8,  3,  8,  2,  6,  2,  8,  -1, 1,  5,  6,  2,  1,  6,  -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, 1,  3,  6,  1,  6,  10, 3,  8,  6,  5,  6,  9,  8,
    9,  6,  -1, 10, 1,  0,  10, 0,  6,  9,  5,  0,  5,  6,  0,  -1, -1, -1, -1,
    0,  3,  8,  5,  6,  10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 5,  6,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, 5,  10, 7,  5,  11,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, 5,  10, 11, 7,  5,  8,  3,  0,
    -1, -1, -1, -1, -1, -1, -1, 5,  11, 7,  5,  10, 11, 1,  9,  0,  -1, -1, -1,
    -1, -1, -1, -1, 10, 7,  5,  10, 11, 7,  9,  8,  1,  8,  3,  1,  -1, -1, -1,
    -1, 11, 1,  2,  11, 7,  1,  7,  5,  1,  -1, -1, -1, -1, -1, -1, -1, 0,  8,
    3,  1,  2,  7,  1,  7,  5,  7,  2,  11, -1, -1, -1, -1, 9,  7,  5,  9,  2,
    7,  9,  0,  2,  2,  11, 7,  -1, -1, -1, -1, 7,  5,  2,  7,  2,  11, 5,  9,
    2,  3,  2,  8,  9,  8,  2,  -1, 2,  5,  10, 2,  3,  5,  3,  7,  5,  -1, -1,
    -1, -1, -1, -1, -1, 8,  2,  0,  8,  5,  2,  8,  7,  5,  10, 2,  5,  -1, -1,
    -1, -1, 9,  0,  1,  5,  10, 3,  5,  3,  7,  3,  10, 2,  -1, -1, -1, -1, 9,
    8,  2,  9,  2,  1,  8,  7,  2,  10, 2,  5,  7,  5,  2,  -1, 1,  3,  5,  3,
    7,  5,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  8,  7,  0,  7,  1,  1,
    7,  5,  -1, -1, -1, -1, -1, -1, -1, 9,  0,  3,  9,  3,  5,  5,  3,  7,  -1,
    -1, -1, -1, -1, -1, -1, 9,  8,  7,  5,  9,  7,  -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 5,  8,  4,  5,  10, 8,  10, 11, 8,  -1, -1, -1, -1, -1, -1, -1,
    5,  0,  4,  5,  11, 0,  5,  10, 11, 11, 3,  0,  -1, -1, -1, -1, 0,  1,  9,
    8,  4,  10, 8,  10, 11, 10, 4,  5,  -1, -1, -1, -1, 10, 11, 4,  10, 4,  5,
    11, 3,  4,  9,  4,  1,  3,  1,  4,  -1, 2,  5,  1,  2,  8,  5,  2,  11, 8,
    4,  5,  8,  -1, -1, -1, -1, 0,  4,  11, 0,  11, 3,  4,  5,  11, 2,  11, 1,
    5,  1,  11, -1, 0,  2,  5,  0,  5,  9,  2,  11, 5,  4,  5,  8,  11, 8,  5,
    -1, 9,  4,  5,  2,  11, 3,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2,  5,
    10, 3,  5,  2,  3,  4,  5,  3,  8,  4,  -1, -1, -1, -1, 5,  10, 2,  5,  2,
    4,  4,  2,  0,  -1, -1, -1, -1, -1, -1, -1, 3,  10, 2,  3,  5,  10, 3,  8,
    5,  4,  5,  8,  0,  1,  9,  -1, 5,  10, 2,  5,  2,  4,  1,  9,  2,  9,  4,
    2,  -1, -1, -1, -1, 8,  4,  5,  8,  5,  3,  3,  5,  1,  -1, -1, -1, -1, -1,
    -1, -1, 0,  4,  5,  1,  0,  5,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8,
    4,  5,  8,  5,  3,  9,  0,  5,  0,  3,  5,  -1, -1, -1, -1, 9,  4,  5,  -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4,  11, 7,  4,  9,  11, 9,
    10, 11, -1, -1, -1, -1, -1, -1, -1, 0,  8,  3,  4,  9,  7,  9,  11, 7,  9,
    10, 11, -1, -1, -1, -1, 1,  10, 11, 1,  11, 4,  1,  4,  0,  7,  4,  11, -1,
    -1, -1, -1, 3,  1,  4,  3,  4,  8,  1,  10, 4,  7,  4,  11, 10, 11, 4,  -1,
    4,  11, 7,  9,  11, 4,  9,  2,  11, 9,  1,  2,  -1, -1, -1, -1, 9,  7,  4,
    9,  11, 7,  9,  1,  11, 2,  11, 1,  0,  8,  3,  -1, 11, 7,  4,  11, 4,  2,
    2,  4,  0,  -1, -1, -1, -1, -1, -1, -1, 11, 7,  4,  11, 4,  2,  8,  3,  4,
    3,  2,  4,  -1, -1, -1, -1, 2,  9,  10, 2,  7,  9,  2,  3,  7,  7,  4,  9,
    -1, -1, -1, -1, 9,  10, 7,  9,  7,  4,  10, 2,  7,  8,  7,  0,  2,  0,  7,
    -1, 3,  7,  10, 3,  10, 2,  7,  4,  10, 1,  10, 0,  4,  0,  10, -1, 1,  10,
    2,  8,  7,  4,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4,  9,  1,  4,  1,
    7,  7,  1,  3,  -1, -1, -1, -1, -1, -1, -1, 4,  9,  1,  4,  1,  7,  0,  8,
    1,  8,  7,  1,  -1, -1, -1, -1, 4,  0,  3,  7,  4,  3,  -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, 4,  8,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, 9,  10, 8,  10, 11, 8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3,
    0,  9,  3,  9,  11, 11, 9,  10, -1, -1, -1, -1, -1, -1, -1, 0,  1,  10, 0,
    10, 8,  8,  10, 11, -1, -1, -1, -1, -1, -1, -1, 3,  1,  10, 11, 3,  10, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,  2,  11, 1,  11, 9,  9,  11, 8,  -1,
    -1, -1, -1, -1, -1, -1, 3,  0,  9,  3,  9,  11, 1,  2,  9,  2,  11, 9,  -1,
    -1, -1, -1, 0,  2,  11, 8,  0,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    3,  2,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2,  3,  8,
    2,  8,  10, 10, 8,  9,  -1, -1, -1, -1, -1, -1, -1, 9,  10, 2,  0,  9,  2,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2,  3,  8,  2,  8,  10, 0,  1,  8,
    1,  10, 8,  -1, -1, -1, -1, 1,  10, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, 1,  3,  8,  9,  1,  8,  -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 0,  9,  1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  3,
    8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

float voxelTable[N * N * N];

_INTR_INLINE void
updatePerInstanceData(CameraRef p_CameraRef,
                      ComputeCallRef p_CurrentAccumComputeCallRef)
{
  NodeRef camNodeRef =
      NodeManager::getComponentForEntity(CameraManager::_entity(p_CameraRef));

  // Post effect data
  {
    const glm::vec2 scattering =
        PostEffectManager::_descVolumetricLightingScatteringDayNight(
            PostEffectManager::_blendTargetRef);

    _perInstanceData.data0.x =
        glm::mix(scattering.y, scattering.x, World::_currentDayNightFactor) *
        GeometryGeneration::_globalScatteringFactor;
    _perInstanceData.data0.z = Clustering::_globalIrradianceFactor;
  }

  _perInstanceData.haltonSamples =
      RenderProcess::UniformManager::_uniformDataSource.haltonSamples32;

  _perInstanceData.prevViewProjMatrix = prevViewProjMatrix;
  prevViewProjMatrix = CameraManager::_viewProjectionMatrix(p_CameraRef);
  _perInstanceData.projMatrix = CameraManager::_projectionMatrix(p_CameraRef);

  _perInstanceData.camPos = glm::vec4(NodeManager::_worldPosition(camNodeRef),
                                      TaskManager::_frameCounter);

  _perInstanceData.eyeVSVectorX = glm::vec4(
      glm::vec3(1.0 / _perInstanceData.projMatrix[0][0], 0.0, 0.0), 0.0);
  _perInstanceData.eyeVSVectorY = glm::vec4(
      glm::vec3(0.0, 1.0 / _perInstanceData.projMatrix[1][1], 0.0), 0.0);
  _perInstanceData.eyeVSVectorZ = glm::vec4(glm::vec3(0.0, 0.0, -1.0), 0.0);
  _perInstanceData.eyeWSVectorX =
      CameraManager::_inverseViewMatrix(p_CameraRef) *
      _perInstanceData.eyeVSVectorX;
  _perInstanceData.eyeWSVectorX.w = TaskManager::_totalTimePassed;
  _perInstanceData.eyeWSVectorY =
      CameraManager::_inverseViewMatrix(p_CameraRef) *
      _perInstanceData.eyeVSVectorY;
  _perInstanceData.eyeWSVectorZ =
      CameraManager::_inverseViewMatrix(p_CameraRef) *
      _perInstanceData.eyeVSVectorZ;

  _perInstanceData.nearFar =
      glm::vec4(CameraManager::_descNearPlane(p_CameraRef),
                CameraManager::_descFarPlane(p_CameraRef), 0.0f, 0.0f);

  Math::FrustumCorners viewSpaceCorners;
  Math::extractFrustumsCorners(
      CameraManager::_inverseProjectionMatrix(p_CameraRef), viewSpaceCorners);

  _perInstanceData.nearFarWidthHeight = glm::vec4(
      viewSpaceCorners.c[3].x - viewSpaceCorners.c[2].x /* Near Width */,
      viewSpaceCorners.c[2].y - viewSpaceCorners.c[1].y /* Near Height */,
      viewSpaceCorners.c[7].x - viewSpaceCorners.c[6].x /* Far Width */,
      viewSpaceCorners.c[6].y - viewSpaceCorners.c[5].y /* Far Height */);

  const _INTR_ARRAY(FrustumRef)& shadowFrustums =
      RenderProcess::Default::_shadowFrustums[p_CameraRef];

  for (uint32_t i = 0u; i < shadowFrustums.size(); ++i)
  {
    FrustumRef shadowFrustumRef = shadowFrustums[i];

    // Transform from camera view space => light proj. space
    _perInstanceData.shadowViewProjMatrix[i] =
        FrustumManager::_viewProjectionMatrix(shadowFrustumRef) *
        CameraManager::_inverseViewMatrix(p_CameraRef);
  }

  ComputeCallManager::updateUniformMemory({p_CurrentAccumComputeCallRef},
                                          &_perInstanceData,
                                          sizeof(PerInstanceData));
}

_INTR_INLINE void createVoxels(float* voxels)
{

  float lower_bound = -1.0;
  float upper_bound = 1.0;
  std::uniform_real_distribution<float> unif(lower_bound, upper_bound);
  std::default_random_engine re;

  memset(voxels, 0.0f, N * N * N * sizeof(float));
}

_INTR_INLINE ComputeCallRef createComputeCallScattering(glm::vec3 p_Dim)
{
  const Name& name = _N(house);
  const uint32_t index = BufferManager::_nameToInitlialBufferMap[name];

  // BufferRef bufferRef = p_Buffers[index];
  bufferRef = BufferManager::_buffersToCreate[index];

  ComputeCallRef computeCallScatteringRef =
      ComputeCallManager::createComputeCall(_N(GeometryGeneration));
  {
    ComputeCallManager::resetToDefault(computeCallScatteringRef);
    ComputeCallManager::addResourceFlags(
        computeCallScatteringRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallScatteringRef) =
        glm::uvec3(8u, 8u, 8u);
    ComputeCallManager::_descPipeline(computeCallScatteringRef) =
        _pipelineScatteringRef;

    ComputeCallManager::bindBuffer(
        computeCallScatteringRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceData));
    ComputeCallManager::bindBuffer(computeCallScatteringRef, _N(positionBuffer),
                                   GpuProgramType::kCompute, bufferRef,
                                   UboType::kPerInstanceCompute,
                                   BufferManager::_descSizeInBytes(bufferRef));
    ComputeCallManager::bindBuffer(
        computeCallScatteringRef, _N(triangleConnectionBuffer),
        GpuProgramType::kCompute, _triangleConnectionBuffer,
        UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(_triangleConnectionBuffer));
    ComputeCallManager::bindBuffer(
        computeCallScatteringRef, _N(voxelBuffer), GpuProgramType::kCompute,
        _voxelBuffer, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(_voxelBuffer));
    ComputeCallManager::bindBuffer(
        computeCallScatteringRef, _N(debugBuffer), GpuProgramType::kCompute,
        _debugBuffer, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(_debugBuffer));
  }

  return computeCallScatteringRef;
}
} // namespace

// Static members
// float GeometryGeneration::_globalScatteringFactor = 1.0f;

void GeometryGeneration::postInit()
{
  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;

  // Pipeline layouts
  PipelineLayoutRef pipelineLayoutScattering;
  {
    {
      pipelineLayoutScattering =
          PipelineLayoutManager::createPipelineLayout(_N(GeometryGeneration));
      PipelineLayoutManager::resetToDefault(pipelineLayoutScattering);

      GpuProgramManager::reflectPipelineLayout(
          8u,
          {GpuProgramManager::getResourceByName("geometry_generation.comp")},
          pipelineLayoutScattering);
    }
    pipelineLayoutsToCreate.push_back(pipelineLayoutScattering);
  }

  // Pipeline
  {
    {
      _pipelineScatteringRef =
          PipelineManager::createPipeline(_N(GeometryGeneration));
      PipelineManager::resetToDefault(_pipelineScatteringRef);

      PipelineManager::_descComputeProgram(_pipelineScatteringRef) =
          GpuProgramManager::getResourceByName("geometry_generation.comp");
      PipelineManager::_descPipelineLayout(_pipelineScatteringRef) =
          pipelineLayoutScattering;
    }
    pipelinesToCreate.push_back(_pipelineScatteringRef);
  }

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  PipelineManager::createResources(pipelinesToCreate);

  ComputeCallRefArray computeCallsToCreate;
  BufferRefArray buffersToCreate;

  const glm::uvec3 computeDim = glm::uvec3(8u, 8u, 8u);

  // Compute calls
  {
    // Scattering
    _computeCallScatteringRef = createComputeCallScattering(computeDim);

    computeCallsToCreate.push_back(_computeCallScatteringRef);
  }
  ComputeCallManager::createResources(computeCallsToCreate);
}

void GeometryGeneration::init()
{
  std::ifstream infile("data.txt");
  float a;
  int i = 0;
  while (infile >> a)
  {
    // process pair (a,b)
    //std::cout << a < std::endl;
    //_INTR_LOG_INFO("%f", a);
    voxelTable[i] = a;
    i++;
    if (i > N * N * N)
      break;
  }

  //createVoxels(voxelTable);

  // Buffers
  BufferRefArray buffersToCreate;
  {
    uint32_t indexBufferSizeInBytes = 4096 * sizeof(int);

    _triangleConnectionBuffer =
        BufferManager::createBuffer(_N(_TriangleConnectionTable));
    {
      BufferManager::resetToDefault(_triangleConnectionBuffer);
      BufferManager::addResourceFlags(
          _triangleConnectionBuffer,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      BufferManager::_descBufferType(_triangleConnectionBuffer) =
          BufferType::kStorage;
      BufferManager::_descSizeInBytes(_triangleConnectionBuffer) =
          indexBufferSizeInBytes;
      BufferManager::_descInitialData(_triangleConnectionBuffer) =
          triangleConnectionTable;
    }
    buffersToCreate.push_back(_triangleConnectionBuffer);

    _voxelBuffer = BufferManager::createBuffer(_N(_Voxels));
    {
      BufferManager::resetToDefault(_voxelBuffer);
      BufferManager::addResourceFlags(
          _voxelBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

      BufferManager::_descBufferType(_voxelBuffer) = BufferType::kStorage;
      BufferManager::_descSizeInBytes(_voxelBuffer) = N * N * N * sizeof(float);
      BufferManager::_descInitialData(_voxelBuffer) = voxelTable;
    }
    buffersToCreate.push_back(_voxelBuffer);

	_debugBuffer = BufferManager::createBuffer(_N(_DebugBuffer));
    {
      BufferManager::resetToDefault(_debugBuffer);
      BufferManager::addResourceFlags(
          _debugBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

      BufferManager::_descBufferType(_debugBuffer) = BufferType::kStorage;
      BufferManager::_descSizeInBytes(_debugBuffer) =
          150000 * 8 * sizeof(float);
      // BufferManager::_descInitialData(_debugBuffer) = voxelTable;
    }
    buffersToCreate.push_back(_debugBuffer);
  }

  BufferManager::createResources(buffersToCreate);
}

// <-

void GeometryGeneration::onReinitRendering() {}

// <-

void GeometryGeneration::destroy() {}

// <-

void GeometryGeneration::render(float p_DeltaT, CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Render Volumetric Lighting");
  _INTR_PROFILE_GPU("Render Volumetric Lighting");

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  ComputeCallRef scatteringComputeCalltoUse = _computeCallScatteringRef;

  // Maybe it will be useful in the future...
  /*
  {
    // Update per instance data
    updatePerInstanceData(p_CameraRef, scatteringComputeCalltoUse);
  }
  */

  // ImageManager::insertImageMemoryBarrier(
  //    _volLightingScatteringBufferImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
  //    VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  //    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  {
    RenderSystem::dispatchComputeCall(scatteringComputeCalltoUse,
                                      primaryCmdBuffer);
  }
  // ImageManager::insertImageMemoryBarrier(
  //    _volLightingScatteringBufferImageRef, VK_IMAGE_LAYOUT_GENERAL,
  //    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  //    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  BufferManager::insertBufferMemoryBarrier(
      bufferRef, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

  // W PassClusteting.cpp jest opis jak dostac sie do pamieci GPU!!!!!

  //_INTR_LOG_WARNING("%d", BufferManager::_descSizeInBytes(bufferRef));

  /*
  _positionBufferGpuMemory =
  (Position*)BufferManager::getGpuMemory(bufferRef);
  _INTR_LOG_WARNING("%f %f %f", _positionBufferGpuMemory->pos[0],
                                                        _positionBufferGpuMemory->pos[1],
                                                            _positionBufferGpuMemory->pos[2]);
  */
}
} // namespace RenderPass
} // namespace Renderer
} // namespace Intrinsic
