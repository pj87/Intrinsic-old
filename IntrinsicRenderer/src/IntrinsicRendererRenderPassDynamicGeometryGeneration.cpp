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
float target = 0.0;
float noiseParams[] = {0.02, 2.0, 0.5};

int cubeEdgeFlags[256] = {
    0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905,
    0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00, 0x190, 0x099, 0x393, 0x29a,
    0x596, 0x49f, 0x795, 0x69c, 0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93,
    0xf99, 0xe90, 0x230, 0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
    0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30, 0x3a0, 0x2a9,
    0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5, 0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6,
    0xfaa, 0xea3, 0xda9, 0xca0, 0x460, 0x569, 0x663, 0x76a, 0x066, 0x16f,
    0x265, 0x36c, 0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
    0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc, 0xdfc, 0xcf5,
    0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0, 0x650, 0x759, 0x453, 0x55a,
    0x256, 0x35f, 0x055, 0x15c, 0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53,
    0x859, 0x950, 0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc,
    0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0, 0x8c0, 0x9c9,
    0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc, 0x0cc, 0x1c5, 0x2cf, 0x3c6,
    0x4ca, 0x5c3, 0x6c9, 0x7c0, 0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f,
    0xf55, 0xe5c, 0x15c, 0x055, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
    0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5,
    0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0, 0xb60, 0xa69, 0x963, 0x86a,
    0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c, 0x265, 0x16f, 0x066, 0x76a, 0x663,
    0x569, 0x460, 0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
    0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0, 0xd30, 0xc39,
    0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636,
    0x13a, 0x033, 0x339, 0x230, 0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f,
    0x895, 0x99c, 0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190,
    0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605,
	0x50f, 0x406, 0x30a, 0x203, 0x109, 0x000
};

int triangleConnectionTable[4096] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1,
    3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1,
    3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1,
    3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1,
    9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1,
    1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1,
    9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1,
    2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1,
    8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1,
    9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1,
    4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1,
    3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1,
    1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1,
    4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1,
    4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1,
    9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1,
    1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
    5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1,
    2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1,
    9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
    0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1,
    2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1,
    10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1,
    4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1,
    5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1,
    5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1,
    9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1,
    0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1,
    1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1,
    10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1,
    8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1,
    2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1,
    7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1,
    9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1,
    2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1,
    11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1,
    9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1,
    5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1,
    11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1,
    11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1,
    1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1,
    9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1,
    5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1,
    2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1,
    5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1,
    6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1,
    0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1,
    3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1,
    6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1,
    5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1,
    1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1,
    10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1,
    6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1,
    1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1,
    8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1,
    7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1,
    3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1,
    5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1,
    0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1,
    9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1,
    8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1,
    5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1,
    0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1,
    6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1,
    10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1,
    10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1,
    8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1,
    1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1,
    3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1,
    0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1,
    10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1,
    0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1,
    3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1,
    6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1,
    9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1,
    8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1,
    3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1,
    6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1,
    0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1,
    10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1,
    10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1,
    1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1,
    2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1,
    7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1,
    7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1,
    2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1,
    1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1,
    11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1,
    8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1,
    0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1,
    7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1,
    10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1,
    2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1,
    6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1,
    7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1,
    2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1,
    1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1,
    10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1,
    10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1,
    0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1,
    7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1,
    6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1,
    8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1,
    9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1,
    6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1,
    1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1,
    4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1,
    10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1,
    8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1,
    0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1,
    1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1,
    8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1,
    10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1,
    4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1,
    10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1,
    5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1,
    11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1,
    9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1,
    6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1,
    7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1,
    3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1,
    7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1,
    9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1,
    3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1,
    6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1,
    9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1,
    1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1,
    4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1,
    7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1,
    6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1,
    3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1,
    0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1,
    6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1,
    1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1,
    0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1,
    11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1,
    6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1,
    5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1,
    9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1,
    1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1,
    1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1,
    10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1,
    0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1,
    5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1,
    10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1,
    11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1,
    0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1,
    9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1,
    7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1,
    2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1,
    8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1,
    9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1,
    9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1,
    1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1,
    9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1,
    9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1,
    5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1,
    0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1,
    10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1,
    2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1,
    0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1,
    0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1,
    9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1,
    5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1,
    3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1,
    5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1,
    8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1,
    0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1,
    9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1,
    0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1,
    1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1,
    3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1,
    4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1,
    9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1,
    11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1,
    11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1,
    2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1,
    9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1,
    3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1,
    1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1,
    4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1,
    4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1,
    3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1,
    3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1,
    0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1,
    9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1,
    1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

_INTR_INLINE ComputeCallRef createComputeCallPolygonization(
    std::unique_ptr<DynamicGeneratedMesh>& mesh, glm::vec3 p_Dim)
{
  const Name& name = *(mesh->meshName);
  const uint32_t index = BufferManager::_nameToInitlialBufferMap[name];

  mesh->_positionBufferRef = BufferManager::_dynamicBuffers[index];
  mesh->_uv0BufferRef = BufferManager::_dynamicBuffers[index + 1];
  mesh->_normalBufferRef = BufferManager::_dynamicBuffers[index + 2];
  mesh->_tangentBufferRef = BufferManager::_dynamicBuffers[index + 3];
  mesh->_binormalBufferRef = BufferManager::_dynamicBuffers[index + 4];
  mesh->_colorBufferRef = BufferManager::_dynamicBuffers[index + 5];
  
  ComputeCallRef computeCallPloygonizationRef =
      ComputeCallManager::createComputeCall(_N(DynamicGeometryGeneration));
  {
    ComputeCallManager::resetToDefault(computeCallPloygonizationRef);
    ComputeCallManager::addResourceFlags(
        computeCallPloygonizationRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallPloygonizationRef) =
        glm::uvec3(p_Dim);
    ComputeCallManager::_descPipeline(computeCallPloygonizationRef) =
        mesh->_pipelineScatteringRef;

    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_PositionBuffer), GpuProgramType::kCompute,
        mesh->_positionBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_positionBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_NormalBuffer), GpuProgramType::kCompute,
        mesh->_normalBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_normalBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_BinormalBuffer), GpuProgramType::kCompute,
        mesh->_binormalBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_binormalBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_TangentBuffer), GpuProgramType::kCompute,
        mesh->_tangentBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_tangentBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_Uv0Buffer), GpuProgramType::kCompute,
        mesh->_uv0BufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_uv0BufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_ColorBuffer), GpuProgramType::kCompute,
        mesh->_colorBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_colorBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_CubeEdgeBuffer), GpuProgramType::kCompute,
        mesh->_cubeEdgeFlagsBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_cubeEdgeFlagsBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_TriangleConnectionBuffer),
        GpuProgramType::kCompute, mesh->_triangleConnectionBufferRef,
        UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_triangleConnectionBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_VoxelBuffer), GpuProgramType::kCompute,
        mesh->_voxelBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_voxelBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_DebugBuffer), GpuProgramType::kCompute,
        mesh->_debugBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_debugBufferRef));
    ComputeCallManager::bindImage(
        computeCallPloygonizationRef, _N(_NormalsTex), GpuProgramType::kCompute,
        mesh->_normalsImageRef, Samplers::kNearestRepeat);
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_SizesBuffer), GpuProgramType::kCompute,
        mesh->_sizesBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_sizesBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallPloygonizationRef, _N(_TargetBuffer), GpuProgramType::kCompute,
        mesh->_targetBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_targetBufferRef));
  }

  return computeCallPloygonizationRef;
}

_INTR_INLINE ComputeCallRef createComputeCallSDFGeneration(
    std::unique_ptr<DynamicGeneratedMesh>& mesh, glm::vec3 p_Dim)
{
  ComputeCallRef computeCallSDFGenerationRef =
      ComputeCallManager::createComputeCall(_N(SDFGenerationNoiseGeneration));
  {
    ComputeCallManager::resetToDefault(computeCallSDFGenerationRef);
    ComputeCallManager::addResourceFlags(
        computeCallSDFGenerationRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallSDFGenerationRef) =
        glm::uvec3(p_Dim);
    ComputeCallManager::_descPipeline(computeCallSDFGenerationRef) =
        mesh->_pipelineSDFGenerationRef;

    ComputeCallManager::bindBuffer(
        computeCallSDFGenerationRef, _N(_VoxelBuffer), GpuProgramType::kCompute,
        mesh->_voxelBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_voxelBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallSDFGenerationRef, _N(_VoxelNormalBuffer), GpuProgramType::kCompute,
        mesh->_voxelNormalBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_voxelNormalBufferRef));
    ComputeCallManager::bindImage(
        computeCallSDFGenerationRef, _N(_Gradient3D), GpuProgramType::kCompute,
        mesh->_gradient3dImageRef, Samplers::kNearestRepeat);
    ComputeCallManager::bindImage(
        computeCallSDFGenerationRef, _N(_PermTable2D), GpuProgramType::kCompute,
        mesh->_permTable2dImageRef, Samplers::kNearestRepeat);
    ComputeCallManager::bindBuffer(
        computeCallSDFGenerationRef, _N(_SizeBuffer), GpuProgramType::kCompute,
        mesh->_sizesBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_sizesBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallSDFGenerationRef, _N(_ParametersBuffer), GpuProgramType::kCompute,
        mesh->_noiseParametersRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_noiseParametersRef));
  }

  return computeCallSDFGenerationRef;
}

_INTR_INLINE ComputeCallRef createComputeCallNormal(
    std::unique_ptr<DynamicGeneratedMesh>& mesh, glm::vec3 p_Dim)
{
  ComputeCallRef computeCallNormalRef =
      ComputeCallManager::createComputeCall(_N(NormalGeneration));
  {
    ComputeCallManager::resetToDefault(computeCallNormalRef);
    ComputeCallManager::addResourceFlags(
        computeCallNormalRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallNormalRef) =
        glm::uvec3(p_Dim);
    ComputeCallManager::_descPipeline(computeCallNormalRef) =
        mesh->_pipelineNormalRef;

    ComputeCallManager::bindImage(
        computeCallNormalRef, _N(_NormalTex), GpuProgramType::kCompute,
        mesh->_normalsImageRef, Samplers::kNearestRepeat);
    ComputeCallManager::bindBuffer(
        computeCallNormalRef, _N(_NoiseBuffer), GpuProgramType::kCompute,
        mesh->_voxelBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_voxelBufferRef));
    ComputeCallManager::bindBuffer(
        computeCallNormalRef, _N(_SizeBuffer), GpuProgramType::kCompute,
        mesh->_sizesBufferRef, UboType::kPerInstanceCompute,
        BufferManager::_descSizeInBytes(mesh->_sizesBufferRef));
  }

  return computeCallNormalRef;
}

} // namespace

// Static members

std::vector<std::unique_ptr<DynamicGeneratedMesh>>
    DynamicGeometryGeneration::dynamicGenerationMeshes;

std::vector<std::unique_ptr<Name>> pseudoInstancedMeshes;

void DynamicGeometryGeneration::postInit()
{
  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  ComputeCallRefArray computeCallsToCreate;

  for (auto& mesh : dynamicGenerationMeshes)
  {
    // Pipeline layouts
    PipelineLayoutRef pipelineLayoutSDFGeneration;
    {
      {
        pipelineLayoutSDFGeneration = PipelineLayoutManager::createPipelineLayout(
            _N(SDFGenerationNoiseGeneration));
        PipelineLayoutManager::resetToDefault(pipelineLayoutSDFGeneration);

        GpuProgramManager::reflectPipelineLayout(
            1u, {GpuProgramManager::getResourceByName(*(mesh->shaders[0]))},
            pipelineLayoutSDFGeneration);
      }
      pipelineLayoutsToCreate.push_back(pipelineLayoutSDFGeneration);
    }

    // Pipeline
    {
      PipelineRef _pipelineSDFGenerationRef =
          PipelineManager::createPipeline(_N(SDFGenerationNoiseGeneration));
      {
		  PipelineManager::resetToDefault(_pipelineSDFGenerationRef);
		  PipelineManager::_descComputeProgram(_pipelineSDFGenerationRef) =
			  GpuProgramManager::getResourceByName(*(mesh->shaders[0]));
		  PipelineManager::_descPipelineLayout(_pipelineSDFGenerationRef) =
			  pipelineLayoutSDFGeneration;
	  }
      mesh->_pipelineSDFGenerationRef = _pipelineSDFGenerationRef;
      pipelinesToCreate.push_back(_pipelineSDFGenerationRef);
    }

    // Pipeline layouts
    PipelineLayoutRef pipelineLayoutNormal;
    {
      {
        pipelineLayoutNormal =
            PipelineLayoutManager::createPipelineLayout(_N(NormalGeneration));
        PipelineLayoutManager::resetToDefault(pipelineLayoutNormal);

        GpuProgramManager::reflectPipelineLayout(
            1u, {GpuProgramManager::getResourceByName(*(mesh->shaders[1]))},
            pipelineLayoutNormal);
      }
      pipelineLayoutsToCreate.push_back(pipelineLayoutNormal);
    }

    // Pipeline
    {
        PipelineRef _pipelineNormalRef =
            PipelineManager::createPipeline(_N(NormalGeneration));
      {
        PipelineManager::resetToDefault(_pipelineNormalRef);

        PipelineManager::_descComputeProgram(_pipelineNormalRef) =
            GpuProgramManager::getResourceByName(*(mesh->shaders[1]));
        PipelineManager::_descPipelineLayout(_pipelineNormalRef) =
            pipelineLayoutNormal;
      }
      mesh->_pipelineNormalRef = _pipelineNormalRef;
      pipelinesToCreate.push_back(_pipelineNormalRef);
    }

    // Pipeline layouts
    PipelineLayoutRef pipelineLayoutScattering;
    {
      {
        pipelineLayoutScattering = PipelineLayoutManager::createPipelineLayout(
            _N(DynamicGeometryGeneration));
        PipelineLayoutManager::resetToDefault(pipelineLayoutScattering);

        GpuProgramManager::reflectPipelineLayout(
            1u, {GpuProgramManager::getResourceByName(*(mesh->shaders[2]))},
            pipelineLayoutScattering);
      }
      pipelineLayoutsToCreate.push_back(pipelineLayoutScattering);
    }

    // Pipeline
    {
        PipelineRef _pipelineScatteringRef =
            PipelineManager::createPipeline(_N(DynamicGeometryGeneration));
      {
        PipelineManager::resetToDefault(_pipelineScatteringRef);

        PipelineManager::_descComputeProgram(_pipelineScatteringRef) =
            GpuProgramManager::getResourceByName(*(mesh->shaders[2]));
        PipelineManager::_descPipelineLayout(_pipelineScatteringRef) =
            pipelineLayoutScattering;
      }
      mesh->_pipelineScatteringRef = _pipelineScatteringRef;
      pipelinesToCreate.push_back(_pipelineScatteringRef);
    }

    PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
    PipelineManager::createResources(pipelinesToCreate);

    const glm::uvec3 computeDim = 
		glm::uvec3(sqrt(mesh->sizes[0]), 
				   sqrt(mesh->sizes[1]), 
				   sqrt(mesh->sizes[2]));

    {
      // SDF generation
      ComputeCallRef _computeCallSDFGenerationRef =
          createComputeCallSDFGeneration(mesh, computeDim);

	  mesh->_computeCallSDFGenerationRef = _computeCallSDFGenerationRef;
      computeCallsToCreate.push_back(_computeCallSDFGenerationRef);
    }

    {
      // Normal
      ComputeCallRef _computeCallNormalRef =
          createComputeCallNormal(mesh, computeDim);

	  mesh->_computeCallNormalRef = _computeCallNormalRef;
      computeCallsToCreate.push_back(_computeCallNormalRef);
    }

    // Compute calls
    {
      // Polygonization
      ComputeCallRef _computeCallMarchingCubesRef =
          createComputeCallPolygonization(mesh, computeDim);

	  mesh->_computeCallMarchingCubesRef = _computeCallMarchingCubesRef;
      computeCallsToCreate.push_back(_computeCallMarchingCubesRef);
    }

  }

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  PipelineManager::createResources(pipelinesToCreate);
  ComputeCallManager::createResources(computeCallsToCreate);
}

void DynamicGeometryGeneration::addDynamicGeneradtedMesh(
    const int& sizeX, const int& sizeY, const int& sizeZ, 
	const Name& meshName, const Name&& voxelGenerationShadera,
    const Name&& normalGenerationShader, const Name&& geometryGenerationShader, 
	bool isDynamic)
{
  std::unique_ptr<DynamicGeneratedMesh> dynamicGenerationMesh =
      std::make_unique<DynamicGeneratedMesh>(
          sizeX, sizeY, sizeZ, 
          std::move(meshName), std::move(voxelGenerationShadera),
          std::move(normalGenerationShader),
          std::move(geometryGenerationShader), 
		  isDynamic);

  dynamicGenerationMeshes.push_back(std::move(dynamicGenerationMesh));
}

bool DynamicGeometryGeneration::isOverridenMesh(const Name& meshName)
{
  for (auto& mesh : dynamicGenerationMeshes)
  {
    if ((*mesh->meshName) == meshName)
      return true;
  }
  return false;
}

void DynamicGeometryGeneration::init()
{
  // Buffers
  BufferRefArray buffersToCreate;
  ImageRefArray imgsToCreate;
    
  for (auto& mesh : dynamicGenerationMeshes)
  {
      BufferRef _noiseParametersRef =
          BufferManager::createBuffer(_N(_ParametersBuffer));
      {
        BufferManager::resetToDefault(_noiseParametersRef);
        BufferManager::addResourceFlags(
            _noiseParametersRef,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(_noiseParametersRef) =
            BufferType::kStorage;
        BufferManager::_descMemoryPoolType(_noiseParametersRef) =
            MemoryPoolType::kStaticStagingBuffers;
        BufferManager::_descSizeInBytes(_noiseParametersRef) =
            sizeof(noiseParams);
        BufferManager::_descInitialData(_noiseParametersRef) = 
			noiseParams;
      }
      mesh->_noiseParametersRef = _noiseParametersRef;
      buffersToCreate.push_back(_noiseParametersRef);

      BufferRef _voxelBufferRef = 
		  BufferManager::createBuffer(_N(_Voxels));
      {
        BufferManager::resetToDefault(_voxelBufferRef);
        BufferManager::addResourceFlags(
            _voxelBufferRef, 
			Dod::Resources::ResourceFlags::kResourceVolatile);
        ///// PJ: only for tests
        BufferManager::_descMemoryPoolType(_voxelBufferRef) =
            MemoryPoolType::kStaticStagingBuffers;
        ///// PJ: only for tests
        BufferManager::_descBufferType(_voxelBufferRef) = 
			BufferType::kStorage;
        BufferManager::_descSizeInBytes(_voxelBufferRef) = 
			(*mesh->sizeX) * (*mesh->sizeY) * (*mesh->sizeZ) * sizeof(float);
      }
      mesh->_voxelBufferRef = _voxelBufferRef;
      buffersToCreate.push_back(_voxelBufferRef);

	  BufferRef _voxelNormalBufferRef =
		  BufferManager::createBuffer(_N(_VoxelNormals));
      {
        BufferManager::resetToDefault(_voxelNormalBufferRef);
        BufferManager::addResourceFlags(
            _voxelNormalBufferRef,
			Dod::Resources::ResourceFlags::kResourceVolatile);
        ///// PJ: only for tests
        BufferManager::_descMemoryPoolType(_voxelNormalBufferRef) =
            MemoryPoolType::kStaticStagingBuffers;
        ///// PJ: only for tests
        BufferManager::_descBufferType(_voxelNormalBufferRef) = BufferType::kStorage;
        BufferManager::_descSizeInBytes(_voxelNormalBufferRef) =
            (*mesh->sizeX) * (*mesh->sizeY) * (*mesh->sizeZ) * sizeof(float) * 4;
      }
      mesh->_voxelNormalBufferRef = _voxelNormalBufferRef;
      buffersToCreate.push_back(_voxelNormalBufferRef);

      BufferRef _sizesBufferRef = 
		  BufferManager::createBuffer(_N(_SizeBuffer));
      {
        BufferManager::resetToDefault(_sizesBufferRef);
        BufferManager::addResourceFlags(
            _sizesBufferRef, 
			Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(_sizesBufferRef) = 
			BufferType::kStorage;
        BufferManager::_descSizeInBytes(_sizesBufferRef) = 
			sizeof(mesh->sizes);
        BufferManager::_descInitialData(_sizesBufferRef) = 
			mesh->sizes;
      }
      mesh->_sizesBufferRef = _sizesBufferRef;
      buffersToCreate.push_back(_sizesBufferRef);

      BufferRef _targetBufferRef =
          BufferManager::createBuffer(_N(_TargetBuffer));
      {
        BufferManager::resetToDefault(_targetBufferRef);
        BufferManager::addResourceFlags(
            _targetBufferRef, 
			Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(_targetBufferRef) = 
			BufferType::kStorage;
        BufferManager::_descSizeInBytes(_targetBufferRef) = 
			sizeof(float);
        BufferManager::_descInitialData(_targetBufferRef) = 
			&target;
      }
      mesh->_targetBufferRef = _targetBufferRef;
      buffersToCreate.push_back(_targetBufferRef);

      BufferRef _cubeEdgeFlagsBufferRef =
          BufferManager::createBuffer(_N(_CubeEdgeFlags));
      {
        BufferManager::resetToDefault(_cubeEdgeFlagsBufferRef);
        BufferManager::addResourceFlags(
            _cubeEdgeFlagsBufferRef,
            Dod::Resources::ResourceFlags::kResourceVolatile);

		///// PJ: only for tests
        BufferManager::_descMemoryPoolType(_cubeEdgeFlagsBufferRef) =
            MemoryPoolType::kStaticStagingBuffers;
        ///// PJ: only for tests

        BufferManager::_descBufferType(_cubeEdgeFlagsBufferRef) =
            BufferType::kStorage;
        BufferManager::_descSizeInBytes(_cubeEdgeFlagsBufferRef) =
            sizeof(cubeEdgeFlags);
        BufferManager::_descInitialData(_cubeEdgeFlagsBufferRef) =
            cubeEdgeFlags;
      }
      mesh->_cubeEdgeFlagsBufferRef = _cubeEdgeFlagsBufferRef;
      buffersToCreate.push_back(_cubeEdgeFlagsBufferRef);

      BufferRef _triangleConnectionBufferRef =
          BufferManager::createBuffer(_N(_TriangleConnectionTable));
      {
        BufferManager::resetToDefault(_triangleConnectionBufferRef);
        BufferManager::addResourceFlags(
            _triangleConnectionBufferRef,
            Dod::Resources::ResourceFlags::kResourceVolatile);

        BufferManager::_descBufferType(_triangleConnectionBufferRef) =
            BufferType::kStorage;
        BufferManager::_descSizeInBytes(_triangleConnectionBufferRef) =
            sizeof(triangleConnectionTable);
        BufferManager::_descInitialData(_triangleConnectionBufferRef) =
            triangleConnectionTable;
      }
      mesh->_triangleConnectionBufferRef = _triangleConnectionBufferRef;
      buffersToCreate.push_back(_triangleConnectionBufferRef);

      BufferRef _debugBufferRef = 
		  BufferManager::createBuffer(_N(_DebugBuffer));
      {
        BufferManager::resetToDefault(_debugBufferRef);
        BufferManager::addResourceFlags(
            _debugBufferRef, 
			Dod::Resources::ResourceFlags::kResourceVolatile);
        BufferManager::_descBufferType(_debugBufferRef) = 
			BufferType::kStorage;
        BufferManager::_descSizeInBytes(_debugBufferRef) =
            150000 * 8 * sizeof(float);
      }
      mesh->_debugBufferRef = _debugBufferRef;
      buffersToCreate.push_back(_debugBufferRef);

	  // Images
      mesh->_gradient3dImageRef =
          ImageManager::getResourceByName(_N(gradient3d));

      mesh->_permTable2dImageRef =
          ImageManager::getResourceByName(_N(perm_table2d));

	  ImageRef _normalsImageRef =
          ImageManager::getResourceByName(_N(terrain_rock));

	  /*
	  if (*mesh->shaders[1] != _N(normal_generation.comp))
	  {
         _normalsImageRef = 
			 ImageManager::getResourceByName(_N(terrain_rock));
	  }
	  else
	  {
         _normalsImageRef = 
			 ImageManager::createImage(_N(normalsTex));
	  }
	  */
      {
        //ImageManager::resetToDefault(_normalsImageRef);
        ImageManager::addResourceFlags(
            _normalsImageRef, 
			Dod::Resources::ResourceFlags::kResourceVolatile);

		// hack (for the demo): sqrt in normals texture are only for the
        // fractals
        const Name& name = *(mesh->meshName);

        if (name != _N(pbr_test_0125) && name != _N(pbr_test_025) && name != _N(house))
        {
          //ImageManager::_descDimensions(_normalsImageRef) = glm::uvec3(
          //    sqrt(mesh->sizes[0]), sqrt(mesh->sizes[1]), sqrt(mesh->sizes[2]));
        }
        else
        {
          //ImageManager::_descDimensions(_normalsImageRef) =
          //    glm::uvec3(mesh->sizes[0], mesh->sizes[1], mesh->sizes[2]);
        }
        // end of hack (for the demo)

        ImageManager::_descImageFormat(_normalsImageRef) =
            // Format::kR16G16B16A16Float;
            // Format::kB10G11R11UFloat;
            // Format::kR8UNorm;
            // Format::kR16G16Float;
            // Format::kR16G16Float;
            Format::kB8G8R8A8UNorm;
        ImageManager::_descImageType(_normalsImageRef) = 
			ImageType::kTexture;
        ImageManager::_descImageFlags(_normalsImageRef) =
            ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
      }
      mesh->_normalsImageRef = _normalsImageRef;
      imgsToCreate.push_back(_normalsImageRef);
  }
  
  BufferManager::createResources(buffersToCreate);
  ImageManager::createResources(imgsToCreate);
}

// <-

void DynamicGeometryGeneration::onReinitRendering() {}

// <-

void DynamicGeometryGeneration::destroy() {}

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

void DynamicGeometryGeneration::render(float p_DeltaT, CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Render Dynamic Geometry Generation");
  _INTR_PROFILE_GPU("Dynamic Geometry Generation");

  ImageRef dupa = ImageManager::getResourceByName(_N(concrete));

  noiseParams[0] += p_DeltaT;

  for (auto& mesh : dynamicGenerationMeshes)
  {
    if (mesh->isDynamic)
	{
	  BufferRef buffer = mesh->_noiseParametersRef;
      updateDataMemory(noiseParams, buffer,
                       BufferManager::_descSizeInBytes(buffer), 0);
	}
	else
	{
	  if (mesh->isCalled && mesh->counter > 2)
		continue;
	}

    VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

    {
      RenderSystem::dispatchComputeCall(mesh->_computeCallNormalRef,
                                        primaryCmdBuffer);
    }

    ImageManager::insertImageMemoryBarrier(mesh->_normalsImageRef, 
										   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
										   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    {
      RenderSystem::dispatchComputeCall(mesh->_computeCallSDFGenerationRef,
                                        primaryCmdBuffer);
    }

    BufferManager::insertBufferMemoryBarrier(mesh->_voxelBufferRef,
                                             VK_ACCESS_SHADER_WRITE_BIT,
                                             VK_ACCESS_SHADER_READ_BIT);

	BufferManager::insertBufferMemoryBarrier(mesh->_voxelNormalBufferRef,
                                             VK_ACCESS_SHADER_WRITE_BIT,
                                             VK_ACCESS_SHADER_READ_BIT);

    {
      RenderSystem::dispatchComputeCall(mesh->_computeCallMarchingCubesRef,
                                        primaryCmdBuffer);
    }

    BufferManager::insertBufferMemoryBarrier(mesh->_positionBufferRef,
                                             VK_ACCESS_SHADER_WRITE_BIT,
                                             VK_ACCESS_SHADER_READ_BIT);

    BufferManager::insertBufferMemoryBarrier(mesh->_normalBufferRef,
                                             VK_ACCESS_SHADER_WRITE_BIT,
                                             VK_ACCESS_SHADER_READ_BIT);

    BufferManager::insertBufferMemoryBarrier(mesh->_binormalBufferRef,
                                             VK_ACCESS_SHADER_WRITE_BIT,
                                             VK_ACCESS_SHADER_READ_BIT);

    BufferManager::insertBufferMemoryBarrier(mesh->_tangentBufferRef,
                                             VK_ACCESS_SHADER_WRITE_BIT,
                                             VK_ACCESS_SHADER_READ_BIT);

    BufferManager::insertBufferMemoryBarrier(mesh->_uv0BufferRef,
                                             VK_ACCESS_SHADER_WRITE_BIT,
                                             VK_ACCESS_SHADER_READ_BIT);

    BufferManager::insertBufferMemoryBarrier(mesh->_colorBufferRef, 
											 VK_ACCESS_SHADER_WRITE_BIT, 
											 VK_ACCESS_SHADER_READ_BIT);

	mesh->isCalled = true;
    mesh->counter++;

	const Name& name = *(mesh->meshName);

    // hack (for the demo): generate the normals and voxels only for the terrain
	
	if (name != _N(terrain))
		continue;

	// end of hack (for the demo)

	// Uncommet the following lines only when generating a new terrain with new trees

    //PseudoInstancing::voxels.clear();
	//PseudoInstancing::normals.clear();

    //aquireVoxelsAndNormals(*mesh);

    //PseudoInstancing::populateMeshes();
    PseudoInstancing::generateInstances();
  }
}

void DynamicGeometryGeneration::aquireVoxelsAndNormals(
    DynamicGeneratedMesh& mesh)
{
  for (int x = 0; x < 64; x += 1)
    for (int y = 0; y < 64; y += 1)
      for (int z = 0; z < 64; z += 1)
      {
			float voxel = getVoxel(mesh, x, y, z);
			float voxel1 = getVoxel(mesh, x, y + 1, z);

			if (voxel > 0.0 && voxel1 < 0.0)
			{
				  Voxel voxel;
				  voxel.x = static_cast<float>(x);
				  voxel.y = static_cast<float>(y);
				  voxel.z = static_cast<float>(64 - z);

				  glm::vec3 nor = getNormal(mesh, x, y + 1, z);
				  Voxel normal;
				  normal.x = nor.x;
				  normal.y = nor.y;
				  normal.z = nor.z;

				  PseudoInstancing::voxels.push_back(voxel);
				  PseudoInstancing::normals.push_back(normal);
			}
      }
}

float DynamicGeometryGeneration::getVoxel(DynamicGeneratedMesh& mesh, int x,
                                          int y, int z)
{ /*
	https://stackoverflow.com/questions/3613429/algorithm-to-convert-a-multi-dimensional-array-to-a-one-dimensional-array
    https://stackoverflow.com/questions/29022714/java-mapping-multi-dimensional-arrays-to-single
      m0,m1,.. are dimensions
      A(i,j,k,...) -> A0[i + j*m0 + k*m0*m1 + ...]
      */
  //int index = x + y * (*mesh.sizeX) + z * (*mesh.sizeX) * (*mesh.sizeY);
  //int[dimX][dimY][dimZ] : 1 - D array index[i * dimY * dimZ + j * dimZ + k]

  int index = x * (*mesh.sizeY) * (*mesh.sizeZ) + y * (*mesh.sizeZ) + z;

  float* _voxelBufferGpuMemory =
      (float*)BufferManager::getGpuMemory(mesh._voxelBufferRef);

  return *(_voxelBufferGpuMemory + index);
}

glm::vec3& DynamicGeometryGeneration::getNormal(
    DynamicGeneratedMesh& mesh, int x, int y, int z)
{ /*
    https://stackoverflow.com/questions/3613429/algorithm-to-convert-a-multi-dimensional-array-to-a-one-dimensional-array
    https://stackoverflow.com/questions/29022714/java-mapping-multi-dimensional-arrays-to-single
      m0,m1,.. are dimensions
      A(i,j,k,...) -> A0[i + j*m0 + k*m0*m1 + ...]
      */
  // int index = x + y * (*mesh.sizeX) + z * (*mesh.sizeX) * (*mesh.sizeY);
  // int[dimX][dimY][dimZ] : 1 - D array index[i * dimY * dimZ + j * dimZ + k]

  int index = 4 * (x * (*mesh.sizeY) * (*mesh.sizeZ) + y * (*mesh.sizeZ) + z);

  float* srcBuffer =
      (float*)BufferManager::getGpuMemory(mesh._voxelNormalBufferRef);

  float* srcX = &srcBuffer[index];
  float* srcY = &srcBuffer[index + 1];
  float* srcZ = &srcBuffer[index + 2];
  float* srcW = &srcBuffer[index + 3];

  /*
  float srcX = glm::unpackHalf1x16(*src0);
  float srcY = glm::unpackHalf1x16(*src1);
  float srcZ = glm::unpackHalf1x16(*src2);
  */
  //if (srcX > 0.0 || srcY > 0.0 || srcZ > 0.0)
  //_INTR_LOG_WARNING("_normalVertexBufferGpuMemory: %f %f %f %f", *srcX, *srcY, *srcZ, *srcW);

  //return *(_normalBufferGpuMemory + index);
  return glm::vec3(*srcX, *srcY, *srcZ);
}

} // namespace RenderPass
} // namespace Renderer
} // namespace Intrinsic
