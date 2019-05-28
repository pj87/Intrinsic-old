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
	/*
	std::vector<std::unique_ptr<Name>>& PseudoInstancing::getInstancedMesh()
	{
	}
	*/

bool PseudoInstancing::isInstancedMesh(const Name& meshName)
{
  /*
  for (auto& mesh : pseudoInstancedMeshes)
  {
    if ((*mesh) == meshName)
      return true;
  }
  return false;
  */

  if (
      /*
	  meshName == _N(Tree_Tall_01_8) || meshName == _N(Tree_Tall_02_10) || 
      meshName == _N(Tree_Tall_04_12) || */ meshName == _N(Tree_Tall_05_14) || 
      //meshName == _N(Tree_Trunk_01_70) || 
      meshName == _N(cube))
    return true;

  return false;
}
} // namespace Renderer
} // namespace Intrinsic