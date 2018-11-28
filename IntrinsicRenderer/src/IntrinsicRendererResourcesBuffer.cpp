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
#include "IntrinsicAlgorithms/src/MarchingCubes.h"
#include "stdafx.h"
#include <vector>

namespace Intrinsic
{
namespace Renderer
{
namespace Resources
{

// PJ: Added++
// static void readVertexValueFromRawBuffer(BufferRef bufferRef)
glm::vec3* BufferManager::readVertexValueFromRawBuffer(void* initialData, int i)
{
  uint16_t* ptr = reinterpret_cast<uint16_t*>(initialData);
  glm::vec3* pos = new glm::vec3();

  pos->x = glm::detail::toFloat32(ptr[i * 3u]);
  pos->y = glm::detail::toFloat32(ptr[i * 3u + 1u]);
  pos->z = glm::detail::toFloat32(ptr[i * 3u + 2u]);

  //_INTR_LOG_INFO("PJ: Reading: %f, %f, %f", pos->x, pos->y, pos->z);

  return pos;
}

uint16_t* BufferManager::readIndexValueFromRawBuffer(void* initialData, int i)
{
  uint16_t* ptr = reinterpret_cast<uint16_t*>(initialData);

  //_INTR_LOG_INFO("PJ: Reading: %i", ptr[i]);

  return nullptr;
}

void BufferManager::storeVertexValueToRawBuffer(void* initialData, int i,
                                                glm::vec3& pos)
{
  uint16_t* ptr = reinterpret_cast<uint16_t*>(initialData);

  uint32_t packedPosition0 = glm::packHalf2x16(glm::vec2(pos.x, pos.y));
  uint32_t packedPosition1 = glm::packHalf2x16(glm::vec2(pos.z, 0.0f));

  ptr[i * 3u] = packedPosition0;
  ptr[i * 3u + 1u] = packedPosition0 >> 16u;
  ptr[i * 3u + 2u] = packedPosition1;
}

void BufferManager::storeIndexValueToRawBuffer(void* initialData, int i,
                                               int& value)
{
  uint16_t* ptr = reinterpret_cast<uint16_t*>(initialData);
  ptr[i] = value;
}

void BufferManager::updateResources(const BufferRefArray& p_Buffers)
{
  VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

  // for (uint32_t i = 0u; i < p_Buffers.size(); i += 7*8)

  uint32_t i = 322;
  {
    //_INTR_LOG_INFO("PJ: BBBBBB: %d", i);

    BufferRef bufferRef = p_Buffers[i];

    VkBufferCreateInfo bufferCreateInfo = {};
    {
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.pNext = nullptr;
      bufferCreateInfo.usage =
          Helper::mapBufferTypeToVkUsageFlagBits(_descBufferType(bufferRef)) |
          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      bufferCreateInfo.size = _descSizeInBytes(bufferRef);
      bufferCreateInfo.queueFamilyIndexCount = 0;
      bufferCreateInfo.pQueueFamilyIndices = nullptr;
      bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      bufferCreateInfo.flags = 0u;
    }

    VkBuffer& buffer = _vkBuffer(bufferRef);
    //_INTR_ASSERT(buffer == VK_NULL_HANDLE);

    // VkResult result = vkCreateBuffer(RenderSystem::_vkDevice,
    // &bufferCreateInfo,
    //                                 nullptr, &buffer);
    //_INTR_VK_CHECK_RESULT(result);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, buffer, &memReqs);

    MemoryPoolType::Enum memoryPoolType = _descMemoryPoolType(bufferRef);
    GpuMemoryAllocationInfo& memoryAllocationInfo =
        _memoryAllocationInfo(bufferRef);

    VkResult result;
    result = vkBindBufferMemory(RenderSystem::_vkDevice, buffer,
                                memoryAllocationInfo._vkDeviceMemory,
                                memoryAllocationInfo._offset);
    _INTR_VK_CHECK_RESULT(result);

    void* initialData = _descInitialData(bufferRef);
    if (initialData)
    {
      VkBufferCreateInfo stagingBufferCreateInfo = bufferCreateInfo;
      {
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      }

      VkBuffer stagingBuffer;
      result = vkCreateBuffer(RenderSystem::_vkDevice, &stagingBufferCreateInfo,
                              nullptr, &stagingBuffer);
      _INTR_VK_CHECK_RESULT(result);

      VkMemoryRequirements stagingMemReqs;
      vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, stagingBuffer,
                                    &stagingMemReqs);

      const GpuMemoryAllocationInfo stagingGpuAllocInfo =
          GpuMemoryManager::allocateOffset(
              MemoryPoolType::kVolatileStagingBuffers,
              (uint32_t)stagingMemReqs.size, (uint32_t)stagingMemReqs.alignment,
              stagingMemReqs.memoryTypeBits);

      result = vkBindBufferMemory(RenderSystem::_vkDevice, stagingBuffer,
                                  stagingGpuAllocInfo._vkDeviceMemory,
                                  stagingGpuAllocInfo._offset);
      _INTR_VK_CHECK_RESULT(result);

      // Copy initial data to staging memory

      std::vector<Triangle>& triangles =
          Core::Resources::MeshManager::_triangles;

      for (int i = 0; i < triangles.size(); i++)
      {
        std::unique_ptr<glm::vec3> pos = std::make_unique<glm::vec3>();

        pos->x = 10.0 * triangles[i].pos.x;
        pos->y = 10.0 * triangles[i].pos.y;
        pos->z = 10.0 * triangles[i].pos.z;

        storeVertexValueToRawBuffer(initialData, i, *pos);
      }

      {

        memcpy(stagingGpuAllocInfo._mappedMemory, initialData,
               _descSizeInBytes(bufferRef));
      }

      VkBufferCopy bufferCopy = {};
      {
        bufferCopy.dstOffset = 0u;
        bufferCopy.srcOffset = 0u;
        bufferCopy.size = _descSizeInBytes(bufferRef);
      }

      // Finally copy from the staging buffer to the actual buffer
      vkCmdCopyBuffer(copyCmd, stagingBuffer, buffer, 1u, &bufferCopy);
    }
  }

  RenderSystem::flushTemporaryCommandBuffer();
  GpuMemoryManager::resetPool(MemoryPoolType::kVolatileStagingBuffers);
}

void BufferManager::updateResourcesNormal(const BufferRefArray& p_Buffers)
{
  VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

  uint32_t i = 324;
  {

    BufferRef bufferRef = p_Buffers[i];

    VkBufferCreateInfo bufferCreateInfo = {};
    {
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.pNext = nullptr;
      bufferCreateInfo.usage =
          Helper::mapBufferTypeToVkUsageFlagBits(_descBufferType(bufferRef)) |
          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      bufferCreateInfo.size = _descSizeInBytes(bufferRef);
      bufferCreateInfo.queueFamilyIndexCount = 0;
      bufferCreateInfo.pQueueFamilyIndices = nullptr;
      bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      bufferCreateInfo.flags = 0u;
    }

    VkBuffer& buffer = _vkBuffer(bufferRef);
    //_INTR_ASSERT(buffer == VK_NULL_HANDLE);

    // VkResult result = vkCreateBuffer(RenderSystem::_vkDevice,
    // &bufferCreateInfo,
    //                                 nullptr, &buffer);
    //_INTR_VK_CHECK_RESULT(result);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, buffer, &memReqs);

    MemoryPoolType::Enum memoryPoolType = _descMemoryPoolType(bufferRef);
    GpuMemoryAllocationInfo& memoryAllocationInfo =
        _memoryAllocationInfo(bufferRef);

    VkResult result;
    result = vkBindBufferMemory(RenderSystem::_vkDevice, buffer,
                                memoryAllocationInfo._vkDeviceMemory,
                                memoryAllocationInfo._offset);
    _INTR_VK_CHECK_RESULT(result);

    void* initialData = _descInitialData(bufferRef);
    if (initialData)
    {
      VkBufferCreateInfo stagingBufferCreateInfo = bufferCreateInfo;
      {
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      }

      VkBuffer stagingBuffer;
      result = vkCreateBuffer(RenderSystem::_vkDevice, &stagingBufferCreateInfo,
                              nullptr, &stagingBuffer);
      _INTR_VK_CHECK_RESULT(result);

      VkMemoryRequirements stagingMemReqs;
      vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, stagingBuffer,
                                    &stagingMemReqs);

      const GpuMemoryAllocationInfo stagingGpuAllocInfo =
          GpuMemoryManager::allocateOffset(
              MemoryPoolType::kVolatileStagingBuffers,
              (uint32_t)stagingMemReqs.size, (uint32_t)stagingMemReqs.alignment,
              stagingMemReqs.memoryTypeBits);

      result = vkBindBufferMemory(RenderSystem::_vkDevice, stagingBuffer,
                                  stagingGpuAllocInfo._vkDeviceMemory,
                                  stagingGpuAllocInfo._offset);
      _INTR_VK_CHECK_RESULT(result);

      // Copy initial data to staging memory

      std::vector<Triangle>& triangles =
          Core::Resources::MeshManager::_triangles;

      for (int i = 0; i < triangles.size(); i++)
      {
        std::unique_ptr<glm::vec3> pos = std::make_unique<glm::vec3>();

        pos->x = triangles[i].normal.x;
        pos->y = triangles[i].normal.y;
        pos->z = triangles[i].normal.z;

        storeVertexValueToRawBuffer(initialData, i, *pos);
      }

      {

        memcpy(stagingGpuAllocInfo._mappedMemory, initialData,
               _descSizeInBytes(bufferRef));
      }

      VkBufferCopy bufferCopy = {};
      {
        bufferCopy.dstOffset = 0u;
        bufferCopy.srcOffset = 0u;
        bufferCopy.size = _descSizeInBytes(bufferRef);
      }

      // Finally copy from the staging buffer to the actual buffer
      vkCmdCopyBuffer(copyCmd, stagingBuffer, buffer, 1u, &bufferCopy);
    }
  }

  RenderSystem::flushTemporaryCommandBuffer();
  GpuMemoryManager::resetPool(MemoryPoolType::kVolatileStagingBuffers);
}

void BufferManager::updateResourcesIndices(const BufferRefArray& p_Buffers)
{
  VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

  uint32_t index = 328;
  {
    BufferRef bufferRef = p_Buffers[index];
    uint32_t bufSize = _descSizeInBytes(bufferRef) / 8;

    VkBufferCreateInfo bufferCreateInfo = {};
    {
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.pNext = nullptr;
      bufferCreateInfo.usage =
          Helper::mapBufferTypeToVkUsageFlagBits(_descBufferType(bufferRef)) |
          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      bufferCreateInfo.size = _descSizeInBytes(bufferRef);
      bufferCreateInfo.queueFamilyIndexCount = 0;
      bufferCreateInfo.pQueueFamilyIndices = nullptr;
      bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      bufferCreateInfo.flags = 0u;
    }

    VkBuffer& buffer = _vkBuffer(bufferRef);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, buffer, &memReqs);

    MemoryPoolType::Enum memoryPoolType = _descMemoryPoolType(bufferRef);
    GpuMemoryAllocationInfo& memoryAllocationInfo =
        _memoryAllocationInfo(bufferRef);

    VkResult result;
    result = vkBindBufferMemory(RenderSystem::_vkDevice, buffer,
                                memoryAllocationInfo._vkDeviceMemory,
                                memoryAllocationInfo._offset);
    _INTR_VK_CHECK_RESULT(result);

    void* initialData = _descInitialData(bufferRef);
    if (initialData)
    {
      VkBufferCreateInfo stagingBufferCreateInfo = bufferCreateInfo;
      {
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      }

      VkBuffer stagingBuffer;
      result = vkCreateBuffer(RenderSystem::_vkDevice, &stagingBufferCreateInfo,
                              nullptr, &stagingBuffer);
      _INTR_VK_CHECK_RESULT(result);

      VkMemoryRequirements stagingMemReqs;
      vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, stagingBuffer,
                                    &stagingMemReqs);

      const GpuMemoryAllocationInfo stagingGpuAllocInfo =
          GpuMemoryManager::allocateOffset(
              MemoryPoolType::kVolatileStagingBuffers,
              (uint32_t)stagingMemReqs.size, (uint32_t)stagingMemReqs.alignment,
              stagingMemReqs.memoryTypeBits);

      result = vkBindBufferMemory(RenderSystem::_vkDevice, stagingBuffer,
                                  stagingGpuAllocInfo._vkDeviceMemory,
                                  stagingGpuAllocInfo._offset);
      _INTR_VK_CHECK_RESULT(result);

      std::vector<Triangle>& triangles =
          Core::Resources::MeshManager::_triangles;

      for (int i = 0; i < triangles.size() * 3; i++)
        storeIndexValueToRawBuffer(initialData, i, i);

      // Copy initial data to staging memory
      {

        memcpy(stagingGpuAllocInfo._mappedMemory, initialData,
               _descSizeInBytes(bufferRef));
      }

      VkBufferCopy bufferCopy = {};
      {
        bufferCopy.dstOffset = 0u;
        bufferCopy.srcOffset = 0u;
        bufferCopy.size = _descSizeInBytes(bufferRef);
      }

      // Finally copy from the staging buffer to the actual buffer
      vkCmdCopyBuffer(copyCmd, stagingBuffer, buffer, 1u, &bufferCopy);
    }
  }

  RenderSystem::flushTemporaryCommandBuffer();
  GpuMemoryManager::resetPool(MemoryPoolType::kVolatileStagingBuffers);
}

// PJ: Added--
void BufferManager::createResources(const BufferRefArray& p_Buffers)
{
  VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

  _INTR_ARRAY(VkBuffer) stagingBuffersToDestroy;
  stagingBuffersToDestroy.reserve(p_Buffers.size());

  for (uint32_t i = 0u; i < p_Buffers.size(); ++i)
  {
    BufferRef bufferRef = p_Buffers[i];

    VkBufferCreateInfo bufferCreateInfo = {};
    {
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.pNext = nullptr;
      bufferCreateInfo.usage =
          Helper::mapBufferTypeToVkUsageFlagBits(_descBufferType(bufferRef)) |
          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      bufferCreateInfo.size = _descSizeInBytes(bufferRef);
      bufferCreateInfo.queueFamilyIndexCount = 0;
      bufferCreateInfo.pQueueFamilyIndices = nullptr;
      bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      bufferCreateInfo.flags = 0u;
    }

    VkBuffer& buffer = _vkBuffer(bufferRef);
    _INTR_ASSERT(buffer == VK_NULL_HANDLE);

    VkResult result = vkCreateBuffer(RenderSystem::_vkDevice, &bufferCreateInfo,
                                     nullptr, &buffer);
    _INTR_VK_CHECK_RESULT(result);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, buffer, &memReqs);

    MemoryPoolType::Enum memoryPoolType = _descMemoryPoolType(bufferRef);
    GpuMemoryAllocationInfo& memoryAllocationInfo =
        _memoryAllocationInfo(bufferRef);

    bool needsAlloc = true;

    // Try to keep memory for static buffers
    if (memoryPoolType >= MemoryPoolType::kRangeStartStatic &&
        memoryPoolType <= MemoryPoolType::kRangeEndStatic)
    {
      if (memoryAllocationInfo._memoryPoolType == memoryPoolType &&
          memReqs.size <= memoryAllocationInfo._sizeInBytes &&
          memReqs.alignment == memoryAllocationInfo._alignmentInBytes)
      {
        needsAlloc = false;
      }
    }

    if (needsAlloc)
    {
      memoryAllocationInfo = GpuMemoryManager::allocateOffset(
          memoryPoolType, (uint32_t)memReqs.size, (uint32_t)memReqs.alignment,
          memReqs.memoryTypeBits);
    }

    result = vkBindBufferMemory(RenderSystem::_vkDevice, buffer,
                                memoryAllocationInfo._vkDeviceMemory,
                                memoryAllocationInfo._offset);
    _INTR_VK_CHECK_RESULT(result);

    void* initialData = _descInitialData(bufferRef);
    if (initialData)
    {
      VkBufferCreateInfo stagingBufferCreateInfo = bufferCreateInfo;
      {
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      }

      VkBuffer stagingBuffer;
      result = vkCreateBuffer(RenderSystem::_vkDevice, &stagingBufferCreateInfo,
                              nullptr, &stagingBuffer);
      _INTR_VK_CHECK_RESULT(result);

      VkMemoryRequirements stagingMemReqs;
      vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, stagingBuffer,
                                    &stagingMemReqs);

      const GpuMemoryAllocationInfo stagingGpuAllocInfo =
          GpuMemoryManager::allocateOffset(
              MemoryPoolType::kVolatileStagingBuffers,
              (uint32_t)stagingMemReqs.size, (uint32_t)stagingMemReqs.alignment,
              stagingMemReqs.memoryTypeBits);

      result = vkBindBufferMemory(RenderSystem::_vkDevice, stagingBuffer,
                                  stagingGpuAllocInfo._vkDeviceMemory,
                                  stagingGpuAllocInfo._offset);
      _INTR_VK_CHECK_RESULT(result);

      // Copy initial data to staging memory
      {
        memcpy(stagingGpuAllocInfo._mappedMemory, initialData,
               _descSizeInBytes(bufferRef));
      }

      VkBufferCopy bufferCopy = {};
      {
        bufferCopy.dstOffset = 0u;
        bufferCopy.srcOffset = 0u;
        bufferCopy.size = _descSizeInBytes(bufferRef);
      }

      // Finally copy from the staging buffer to the actual buffer
      vkCmdCopyBuffer(copyCmd, stagingBuffer, buffer, 1u, &bufferCopy);
    }
  }

  RenderSystem::flushTemporaryCommandBuffer();

  for (uint32_t i = 0u; i < stagingBuffersToDestroy.size(); ++i)
  {
    vkDestroyBuffer(RenderSystem::_vkDevice, stagingBuffersToDestroy[i],
                    nullptr);
  }

  GpuMemoryManager::resetPool(MemoryPoolType::kVolatileStagingBuffers);
}
}
}
}
