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

void PseudoInstancing::addPseudoInstancingMesh(Name&& name, 
	const unsigned& sizeX, const unsigned& sizeY, const float& probability)
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

} // namespace Renderer
} // namespace Intrinsic