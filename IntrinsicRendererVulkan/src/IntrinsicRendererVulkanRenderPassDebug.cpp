// Copyright 2016 Benjamin Glatzel
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
#include "stdafx_vulkan.h"
#include "stdafx.h"

#define MAX_LINE_COUNT 256000

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace RenderPass
{
namespace
{
struct DebugLineVertex
{
  glm::vec3 pos;
  uint32_t color;
};

struct PerInstanceDataDebugLineVertex
{
  glm::mat4 worldViewProjMatrix;
  glm::mat4 normalMatrix;
};

Resources::ImageRef _albedoImageRef;
Resources::ImageRef _normalImageRef;
Resources::ImageRef _parameter0ImageRef;
Resources::ImageRef _depthImageRef;
Resources::FramebufferRef _framebufferRef;

Resources::PipelineLayoutRef _debugLinePipelineLayout;
Resources::PipelineRef _debugLinePipelineRef;
Resources::DrawCallRef _debugLineDrawCallRef;
Resources::BufferRef _debugLineVertexBufferRef;

Resources::RenderPassRef _renderPassRef;

DebugLineVertex* _mappedLineMemory = nullptr;
uint32_t _currentLineVertexCount = 0u;

void displayWorldBoundingSpheres()
{
  for (uint32_t i = 0u; i < Components::NodeManager::_activeRefs.size(); ++i)
  {
    Components::NodeRef nodeRef = Components::NodeManager::_activeRefs[i];
  }
}

// <-

void displayDebugLineGeometryForSelectedObject()
{
  if (GameStates::Editing::_currentlySelectedEntity.isValid())
  {
    Components::NodeRef nodeRef =
        Components::NodeManager::getComponentForEntity(
            GameStates::Editing::_currentlySelectedEntity);

    if (!nodeRef.isValid())
    {
      return;
    }

    Components::PostEffectVolumeRef postVolumeRef =
        Components::PostEffectVolumeManager::getComponentForEntity(
            GameStates::Editing::_currentlySelectedEntity);
  }
}
}

void Debug::init()
{
  using namespace Resources;
    _renderPassRef = RenderPassManager::createRenderPass(_N(Debug));
}

}
}
}
}
