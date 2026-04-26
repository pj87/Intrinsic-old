# Changes in branch `fixing_for_new_driver`

This branch fixes engine compatibility with new NVIDIA drivers and a newer build
environment (MSVC 14.44, Vulkan SDK 1.3+). The work falls into three broad
categories: (1) GLSL shader correctness (decals GPU hang), (2) Vulkan validation
errors caused by incorrect barrier usage that older, more lenient drivers silently
accepted, and (3) C++ undefined behavior and compiler warnings. Below is a
detailed description of each commit in chronological order.

---

## 1. `670290a1` — Fix build with new MSVC and Vulkan SDK

**Files:** `IntrinsicCore/src/stdafx.h`, `cmake/FindLuaJIT.cmake`

### `<chrono>` missing in stdafx.h

The engine uses `std::chrono` for timing. In older MSVC toolsets, including
`<thread>` also pulled in `<chrono>` as a side effect of the standard library
implementation. MSVC 14.44 cleaned up those implicit dependencies, so `<chrono>`
is no longer available unless explicitly included. The fix is minimal: add
`#include <chrono>` directly to `stdafx.h` (the precompiled header shared by all
translation units), which makes the dependency explicit and immune to future
toolchain changes.

### LuaJIT not found on MSVC

`FindLuaJIT.cmake` searched for library names in this order:
`libluajit-5.1.a`, `libluajit.a`, `lua51.lib`. The first two are GCC/MinGW naming
conventions; `lua51.lib` is the MSVC convention. Because CMake's `find_library`
stops at the first match, on a Windows MSVC build tree the GCC names were never
present and the search fell through to `lua51.lib` only if it happened to be last.
However, if the library was installed anywhere else in the search path under a
non-standard name the logic broke. Reordered the list to put `lua51.lib` first,
matching the build environment's actual output.

---

## 2. `460a12dd` — Fix decals on new NVIDIA drivers

**Files:** `app/assets/shaders/decals.frag.glsl`,
`app/assets/shaders/decals.inc.glsl`,
`IntrinsicRenderer/src/IntrinsicRendererRenderPassClustering.cpp`,
`IntrinsicRenderer/src/IntrinsicRendererResourcesPipeline.cpp`

### Root cause: illegal sampler array parameter in GLSL

The decal fragment shader passed a `sampler2D[4095]` as a function argument to
`calcDecal`. The GLSL specification requires that opaque types (samplers, images,
atomic counters) be accessed only through uniform variables, not through function
parameters that are copies of — or pointers into — an array. Passing a sampler
array as a function argument is undefined in the GLSL specification.

Older NVIDIA drivers parsed and compiled this shader without complaint. Newer
drivers enforce the specification strictly: the driver-side shader compiler either
rejects the shader outright or generates incorrect machine code for it, which
manifests as a GPU hang (the GPU stops processing commands and the driver
eventually triggers a device lost / TDR reset).

### Fix

Removed `sampler2D[4095]` from the `calcDecal` parameter list entirely. The
function now references `globalTextures` directly as a uniform, which is the
legal access pattern — the same approach already used by `lighting.inc.glsl` for
`globalCubeTextures`. Because `globalTextures` is a uniform array declared at file
scope, reading from it inside any function is legal; only passing it as a value
parameter is not.

In addition to the shader change, the Clustering render pass and Resources
Pipeline were updated to correctly create the decal pipeline and issue the draw
call. Earlier the draw code path had debug hacks left in that disabled actual
rendering; those were cleaned up and proper pipeline state was wired up.

---

## 3. `ab909002` — Remove stale debug comment in Clustering

**Files:** `IntrinsicRenderer/src/IntrinsicRendererRenderPassClustering.cpp`

Removed a single outdated `// TODO` / debug comment that referred to work
already completed in the previous commit. No logic change.

---

## 4. `cf298e55` — First Vulkan validation errors (category A)

**Files:** `IntrinsicRenderer/src/IntrinsicRendererHelper.h`,
`IntrinsicRenderer/src/IntrinsicRendererRenderSystem.cpp`

### Problem 1: non-zero access masks with `TOP_OF_PIPE`

In a Vulkan pipeline barrier, `srcAccessMask` describes what memory operations
the driver needs to flush, and `dstAccessMask` describes what operations need
to see those flushes. The pipeline stage `VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT`
is a pseudo-stage that represents "before any real work starts." By definition,
no actual memory operations have occurred yet at that point, so the only legal
value for `srcAccessMask` when `srcStage = TOP_OF_PIPE` is `0`. Similarly,
`dstAccessMask` must be `0` when `dstStage = BOTTOM_OF_PIPE` (a pseudo-stage
meaning "after all work finishes").

The engine's barrier helper originally forwarded whatever access masks the caller
provided, without sanitizing them. This worked on older drivers that silently
ignored the invalid masks. New NVIDIA drivers validate barrier correctness
strictly and reported these as errors. The fix adds logic inside the helper to
force access masks to `0` whenever the stage is `TOP_OF_PIPE` or `BOTTOM_OF_PIPE`,
regardless of what the caller passed.

### Problem 2: `VK_EXT_debug_marker` without `VK_EXT_debug_report`

`VK_EXT_debug_marker` (which enables labelling of Vulkan objects and command
buffer regions) has `VK_EXT_debug_report` as a prerequisite. The engine
unconditionally requested `debug_marker` without first checking whether both
extensions were available in the current environment. When running without a
debug-capable Vulkan runtime, device creation failed or validation produced
errors. Added an availability check: both extensions are only requested when
both are present in the enumerated device extension list.

---

## 5. `fa3d8e07` — Detect and enable `VK_KHR_maintenance2`

**Files:** `IntrinsicRenderer/src/IntrinsicRendererRenderSystem.cpp`

`VK_KHR_maintenance2` (promoted to core in Vulkan 1.1) adds several small
corrections to the spec, but the one relevant here is the introduction of
`VkImageViewUsageCreateInfo`. This structure, chained into `VkImageViewCreateInfo`
via the `pNext` pointer, lets you declare which subset of the parent image's
usage flags this particular view should be restricted to. It is needed in the
next commit to fix SRGB image view validation errors.

Because the engine targets Vulkan 1.0 at the API level, `maintenance2` must be
explicitly requested as a device extension. Added detection of the extension in
the enumerated device extension list and conditional inclusion in the
`VkDeviceCreateInfo::ppEnabledExtensionNames` array.

---

## 6. `e0b33ce6` — Fix pre-present and post-present barriers

**Files:** `IntrinsicRenderer/src/IntrinsicRendererRenderSystem.h`

The swapchain image transitions around `vkQueuePresentKHR` require very specific
barrier configurations because the presentation engine operates outside the
graphics queue's usual pipeline stages.

### Pre-present barrier (before `vkQueuePresentKHR`)

The image must transition from `COLOR_ATTACHMENT_OPTIMAL` to
`PRESENT_SRC_KHR`. The correct stage for the destination is
`BOTTOM_OF_PIPE` (the presentation engine picks it up after the queue is done),
and the Vulkan spec explicitly requires `dstAccessMask = 0` for
`BOTTOM_OF_PIPE`. The code had `dstAccessMask = VK_ACCESS_MEMORY_READ_BIT`,
which is invalid. Fixed to `0`.

### Post-present barrier (after `vkAcquireNextImageKHR`)

The image must transition from `PRESENT_SRC_KHR` (or `UNDEFINED`) back to
`COLOR_ATTACHMENT_OPTIMAL` so the graphics pipeline can write to it again. The
source stage should be `BOTTOM_OF_PIPE` (no GPU work has touched it yet since
present), and the destination stage must be `COLOR_ATTACHMENT_OUTPUT` — the
stage that writes to color attachments. The code had `dstStage = BOTTOM_OF_PIPE`
with `dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT`, which is
contradictory: `BOTTOM_OF_PIPE` cannot have a write access mask. Fixed
`dstStage` to `COLOR_ATTACHMENT_OUTPUT`.

---

## 7. `fd363ee7` — Fix barriers for `TOP_OF_PIPE` and `BOTTOM_OF_PIPE`

**Files:** `IntrinsicRenderer/src/IntrinsicRendererHelper.h`

This is a follow-up to commit `cf298e55`, extending the same access-mask
zeroing logic to the buffer barrier helper and ensuring it covers all call sites
consistently. After the fix in `cf298e55`, further validation runs revealed
additional barriers in the codebase that still passed non-zero access masks
with `TOP_OF_PIPE` or `BOTTOM_OF_PIPE` stages, because they went through a
different code path in the helper. Both image and buffer barrier helpers now
unconditionally clamp access masks to `0` at both pseudo-stages, making the fix
systematic rather than per-call-site.

This commit also serves as the foundation for commit `c5ddc856`: barriers that
previously "worked" with wrong stages and non-zero masks were silently converted
into no-ops by this zeroing logic. That exposed a second class of problem where
barriers were no-ops when they needed to do real synchronization work.

---

## 8. `dd8082e4` — Add `STORAGE_BIT` flag to vertex buffers

**Files:** `IntrinsicRenderer/src/IntrinsicRendererHelper.h`

The marching cubes and procedural geometry generation passes use compute shaders
that write their output (vertex positions, normals, texture coordinates, etc.)
into GPU buffers, which are then consumed as vertex buffers in the subsequent
draw pass. In Vulkan, a buffer that is accessed by a compute shader as an SSBO
(shader storage buffer object) must have `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT`
set at creation time. A buffer that is then read as vertex input must have
`VK_BUFFER_USAGE_VERTEX_BUFFER_BIT` set. Both flags must be present on the same
buffer.

The buffer creation helper was only setting `VERTEX_BUFFER_BIT` for these
buffers. New drivers validate usage flags at descriptor binding time and reported
an error when a buffer without `STORAGE_BUFFER_BIT` was bound as an SSBO.
Added `STORAGE_BUFFER_BIT` to the default set of usage flags in the helper so all
dynamically generated vertex buffers receive both flags.

---

## 9. `689f3ea7` — Guard against double `vkBindBufferMemory`

**Files:** `IntrinsicRenderer/src/IntrinsicRendererResourcesBuffer.cpp`

`vkBindBufferMemory` must be called exactly once per `VkBuffer` object, at the
time the buffer is first given its backing memory. Calling it again on the same
buffer is explicitly illegal per the Vulkan specification.

The engine's buffer creation code allocated memory and bound it, but the function
could be called on a buffer that had already been allocated in a previous frame
or after a resize event. Old drivers tolerated the redundant call. New drivers
treat it as a validation error. Added a boolean check: `vkBindBufferMemory` is
called only when the allocation path actually ran (i.e., a new `VkDeviceMemory`
handle was produced), not on subsequent calls for a buffer that already has
memory bound.

---

## 10. `73af179b` — `VkImageViewUsageCreateInfo` for SRGB image views

**Files:** `IntrinsicRenderer/src/IntrinsicRendererResourcesImage.cpp`

### Background: two views for SRGB textures

The engine creates SRGB textures with two image views:
- A **SRGB view** (`VK_FORMAT_*_SRGB`) used by fragment shaders for gamma-correct
  sampling.
- A **UNORM view** (`VK_FORMAT_*_UNORM`) used by compute shaders for storage
  writes (storage images require a linear format).

The parent `VkImage` is created with both `VK_IMAGE_USAGE_SAMPLED_BIT` and
`VK_IMAGE_USAGE_STORAGE_BIT`. This is legal because the mutable format extension
allows re-interpreting the image through a view of a compatible format.

### The validation error

New drivers enforce a rule introduced by `VK_KHR_maintenance2`: when a
`VkImageView` is created over an image that has `STORAGE_BIT` in its usage flags,
the view itself is implicitly assumed to also support storage use — even if the
view's format (SRGB) is not supported for storage. The validation layer reports
this as a conflict.

### Fix

Chained a `VkImageViewUsageCreateInfo` into the `pNext` of the SRGB view's
`VkImageViewCreateInfo`, specifying only `VK_IMAGE_USAGE_SAMPLED_BIT` (without
`STORAGE_BIT`). This explicitly restricts the SRGB view to sampling-only use,
regardless of what the parent image supports. The UNORM view does not need this
restriction because it legitimately has `STORAGE_BIT`. The `maintenance2`
extension enabled in commit `fa3d8e07` is what makes `VkImageViewUsageCreateInfo`
available.

---

## 11. `1d7e94ab` — Fix image barriers and buffer updates in geometry/texture generation

**Files:** `IntrinsicRendererRenderPassDynamicGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassDynamicTextureGeneration.cpp`,
`IntrinsicRendererRenderPassGeometryGeneration.cpp`

### Problem 1 (critical): self-copy via `vkCmdCopyBuffer`

`updateDataMemory` was called to upload new data into a host-visible buffer.
The implementation issued `vkCmdCopyBuffer(cmdBuf, buffer, buffer, ...)`, using
the same `VkBuffer` handle as both source and destination. The Vulkan spec
prohibits this: source and destination regions must not overlap, and copying a
buffer to itself is always an overlap. The result is undefined behavior — on some
drivers the data was silently corrupted, on others the copy simply didn't happen.

The fix replaces `vkCmdCopyBuffer` with a direct `memcpy` into the buffer's
host-visible mapped pointer. The buffer was already host-coherent (created with
`HOST_VISIBLE | HOST_COHERENT`), so no explicit flush is needed. This is also
more efficient: it eliminates a GPU command altogether.

### Problem 2: missing `UNDEFINED→GENERAL` transitions at init

`normalsTex` and several procedurally generated textures are used as storage
images (`image2D` in GLSL). When they are first created their layout is
`VK_IMAGE_LAYOUT_UNDEFINED`. The compute shaders that write to them require
`VK_IMAGE_LAYOUT_GENERAL`. Without an explicit transition the shaders operated
on images in an undefined layout, producing garbage output or validation errors.
Added `UNDEFINED→GENERAL` barriers in the initialization path for all affected
images.

### Problem 3: wrong barrier types for compute-written buffers

Several barriers around compute writes were issued as `TRANSFER_SRC/DST`
barriers, a leftover from an older implementation that used transfer commands
for the same operations. Compute shader writes require `GENERAL` layout (for
images) and `STORAGE_BUFFER` access flags (for buffers), not transfer access
flags. Updated the barrier types and pipeline stages accordingly.

### Problem 4: missing `GENERAL→SHADER_READ_ONLY` before marching cubes

The marching cubes dispatch reads from a voxel normal texture. This texture must
be in `SHADER_READ_ONLY_OPTIMAL` for the read to be valid. The texture was left
in `GENERAL` after its generation pass. Added the required transition before
the marching cubes dispatch.

---

## 12. `bd927c75` — Fix barrier stages in Bloom and VolumetricLighting

**Files:** `IntrinsicRendererRenderPassBloom.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`

### Background: why `srcStage` matters

`srcStage` in a pipeline barrier tells the driver: "wait until all commands
before this barrier that touch `srcStage` have finished before proceeding."
If `srcStage = TOP_OF_PIPE`, the barrier provides no meaningful wait — nothing
has been executed yet at the top of the pipe, so the "wait" is instantaneous and
empty. The driver is then free to reorder subsequent accesses before the data
is actually ready.

Both Bloom and VolumetricLighting had barriers after their compute dispatches
with `srcStage = TOP_OF_PIPE` instead of `COMPUTE_SHADER_BIT`. This meant the
barrier did not actually wait for the compute shader to finish writing. On older
drivers the accesses happened to be serialized by other means (implicit
synchronization or driver-side conservatism). New drivers exploit the declared
parallelism and access the images before the compute writes land.

### Bloom fix

Changed `srcStage` on the post-lum-dispatch barrier to `COMPUTE_SHADER_BIT`.
Also corrected `dstStage` on the final `_lumImageRef` barrier to
`FRAGMENT_SHADER_BIT` (it was previously `TOP_OF_PIPE`, again a no-op). Added a
first-frame guard flag `_bloomRendered` so the initial barrier uses
`UNDEFINED` as `oldLayout` and subsequent barriers use `SHADER_READ_ONLY_OPTIMAL`
(the layout left by the previous frame's compute pass).

### VolumetricLighting fix

Same `srcStage` fix for the post-accumulation-dispatch barriers. Fixed `dstStage`
of the scatter buffer barrier to `FRAGMENT_SHADER_BIT`. Added the
`_volLightingRendered` first-frame guard.

---

## 13. `fb5bb76a` — Fix `UNDEFINED` layout in Shadow, Clustering, VolumetricLighting, PerPixelPicking

**Files:** `IntrinsicRendererRenderPassShadow.cpp`,
`IntrinsicRendererRenderPassClustering.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`,
`IntrinsicRendererRenderPassPerPixelPicking.cpp`

### Background: Vulkan layout tracking

The Vulkan validation layer tracks the current layout of every image
sub-resource. When a barrier declares `oldLayout = VK_IMAGE_LAYOUT_UNDEFINED`,
the validation layer accepts it on any image (the spec says `UNDEFINED` is always
a legal source layout — the driver may discard the image content). However, if
the image is currently in `SHADER_READ_ONLY_OPTIMAL` and the barrier specifies
`oldLayout = UNDEFINED`, new drivers (and strict validation layers) treat this
as a potential correctness issue: the caller is claiming it doesn't care about
the existing content, which would be wrong if the image held data from the
previous frame that should have been preserved.

### Fix

For each affected render pass, introduced a boolean flag
(`_shadowRendered`, `_clusteringRendered`, `_volLightingRendered`,
`_pickingRendered`). On the first frame the flag is `false`, so barriers use
`UNDEFINED` as `oldLayout`. After the first render call sets the flag to `true`,
subsequent frames supply the concrete layout that was left at the end of the
previous frame:

- Shadow map depth images: `DEPTH_STENCIL_ATTACHMENT_OPTIMAL`
- Clustering lighting buffer: `COLOR_ATTACHMENT_OPTIMAL`
- Volumetric lighting / ESM images: `SHADER_READ_ONLY_OPTIMAL`
- PerPixelPicking color image: `TRANSFER_SRC_OPTIMAL`

All flags are reset to `false` in `onReinitRendering()` because reinit destroys
and recreates the images, putting them back into `UNDEFINED`.

---

## 14. `c5ddc856` — Fix buffer barrier stages in geometry/marching cubes

**Files:** `IntrinsicRendererRenderPassDynamicGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassMarchingCubes.cpp`

### The no-op barrier problem

After the helper fix in `fd363ee7`, any barrier with `srcStage = TOP_OF_PIPE`
or `dstStage = TOP_OF_PIPE` automatically has its access masks zeroed to `0`.
A barrier with both stages set to a pseudo-stage and both access masks set to `0`
is a valid Vulkan barrier — but it provides no synchronization at all. It is a
no-op.

The geometry generation and marching cubes passes had exactly this pattern for
their compute-to-compute and compute-to-vertex buffer barriers: they used
`TOP_OF_PIPE` as a placeholder stage, which after the helper fix became no-ops.
This meant the GPU could start the vertex stage before the compute writes were
visible, resulting in rendering from stale or uninitialized buffer data.

### Fix

- **Voxel data buffers** (written by geometry gen compute, read by marching cubes
  compute): `srcStage = COMPUTE_SHADER_BIT`, `dstStage = COMPUTE_SHADER_BIT`,
  `srcAccessMask = SHADER_WRITE_BIT`, `dstAccessMask = SHADER_READ_BIT`.

- **Output vertex buffers** (position, normal, UV, etc. written by marching cubes
  compute, consumed as vertex attributes by the draw pass):
  `srcStage = COMPUTE_SHADER_BIT`, `dstStage = VERTEX_INPUT_BIT`,
  `srcAccessMask = SHADER_WRITE_BIT`,
  `dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT`.

This ensures the GPU serializes compute writes before vertex fetch reads.

---

## 15. `9bfb146d` — Fix barrier stages in texture upload and PerPixelPicking

**Files:** `IntrinsicRendererResourcesImage.cpp`,
`IntrinsicRendererRenderPassPerPixelPicking.cpp`

### Texture upload path

Texture uploads use a staging buffer copy (`vkCmdCopyBufferToImage`) followed by
a layout transition to `SHADER_READ_ONLY_OPTIMAL`. Both barriers in this sequence
had incorrect stages:

- `UNDEFINED→TRANSFER_DST_OPTIMAL`: needs `srcStage = TOP_OF_PIPE` (nothing to
  wait for, image is new) and `dstStage = TRANSFER_BIT`. Was using wrong masks.
- `TRANSFER_DST_OPTIMAL→SHADER_READ_ONLY_OPTIMAL`: needs `srcStage = TRANSFER_BIT`
  (wait for the copy to finish) and `dstStage = FRAGMENT_SHADER_BIT` (the texture
  will be sampled in fragment shaders). Was using `TOP_OF_PIPE` as `srcStage`,
  making it a no-op and allowing the fragment shader to start sampling before the
  copy completed.

### PerPixelPicking

The picking pass reads back a rendered image over a CPU-readable path:
render to color attachment → transition to `TRANSFER_SRC` → `vkCmdCopyImageToBuffer`
→ CPU reads the buffer. The transitions needed correct stage pairs:

- `TRANSFER_SRC→COLOR_ATTACHMENT_OPTIMAL` (at start of frame): needed
  `srcStage = TRANSFER_BIT`, `dstStage = COLOR_ATTACHMENT_OUTPUT_BIT`.
- `COLOR_ATTACHMENT_OPTIMAL→TRANSFER_SRC` (after render): needed
  `srcStage = COLOR_ATTACHMENT_OUTPUT_BIT`, `dstStage = TRANSFER_BIT`.

The depth image transition was also removed for subsequent frames, because the
render pass itself handles that transition via its attachment load/store ops.

---

## 16. `750aa2fd` — Fix first-frame layout errors in VolumetricLighting

**Files:** `IntrinsicRendererRenderPassVolumetricLighting.cpp`,
`IntrinsicRendererRenderSystem.cpp`, `IntrinsicRendererRenderProcess.cpp`,
`IntrinsicCoreApplication.cpp`

### The problem with array descriptors on the first frame

The engine binds a large array of textures as a single descriptor (bindless
texturing). This array includes the volumetric lighting images and all ESM
(exponential shadow map) layers. When `vkUpdateDescriptorSets` writes this
array, the validation layer checks that every image in the array is in the layout
specified in the descriptor (`SHADER_READ_ONLY_OPTIMAL`).

On the very first frame, freshly created images are in `UNDEFINED` layout. Even
though the volumetric lighting and shadow passes issue per-frame barriers to
transition these images, those barriers run *after* the descriptor set update —
so the update sees `UNDEFINED` images that are declared as `SHADER_READ_ONLY`.
This triggers validation error `VUID-VkDescriptorImageInfo-imageLayout-00344`.

### Fix

Added a one-time command buffer submission at the end of `init()` that
transitions all ESM layers and volumetric lighting images from `UNDEFINED` to
`SHADER_READ_ONLY_OPTIMAL` before the first frame ever starts. This ensures the
global descriptor array sees a consistent, valid layout for all images from the
very first `vkUpdateDescriptorSets` call. The `_volLightingRendered` flag is
also set to `true` in `init()` so the per-frame barriers begin with
`SHADER_READ_ONLY_OPTIMAL` as `oldLayout`, not `UNDEFINED`.

---

## 17. `d8793ced` — Transition images to `SHADER_READ_ONLY` after reinit

**Files:** `IntrinsicRendererRenderProcess.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`,
`IntrinsicRendererResourcesImage.cpp`

### Why reinit is a separate problem from init

Commit `750aa2fd` solved the first-frame descriptor problem at startup. But the
engine supports hot-reloading the renderer configuration (e.g., changing the
render resolution). During `loadRendererConfig()`, all resolution-dependent
images are destroyed and recreated. The newly created images start in `UNDEFINED`
layout again — reproducing exactly the same descriptor array validation error as
before, but now on the first frame after a reinit rather than on the first frame
after startup.

### Fix

After `loadRendererConfig()` recreates all images, a temporary command buffer is
submitted that transitions every resolution-dependent image from `UNDEFINED` to
`SHADER_READ_ONLY_OPTIMAL`. This mirrors the init-time transition from `750aa2fd`
but is triggered by the reinit event.

Additionally, `VolumetricLighting::onReinitRendering()` was not properly
reinitializing its own static (non-resolution-dependent) images to
`SHADER_READ_ONLY` after the reinit. Fixed by adding the same one-time command
buffer submission pattern there.

---

## 18. `57e8245a` — Fix `VUID-VkDescriptorImageInfo-imageLayout-00344` in DynamicTextureGeneration

**Files:** `IntrinsicRendererRenderPassDynamicTextureGeneration.cpp`,
`IntrinsicRendererRenderSystem.cpp`, `IntrinsicCoreApplication.cpp`

### The validation error

`VUID-VkDescriptorImageInfo-imageLayout-00344` fires when a descriptor set
update writes an image with a layout that does not match the layout the image is
actually in at update time. In this case the texture generation compute shaders
declare their output images (`_TextureTex`) and source images (`_SourceTex`) as
`image2D` (storage images). Storage images must be in `VK_IMAGE_LAYOUT_GENERAL`.

The texture generation pass was issuing a barrier with `oldLayout = UNDEFINED`
before each dispatch. The validation layer, which tracks the actual image layout,
knew the images were in `SHADER_READ_ONLY_OPTIMAL` (left by the previous frame's
sampling pass). Using `UNDEFINED` as `oldLayout` when the image is in a known
layout tells the driver it can discard the content — which might be valid for
write-only outputs, but the validation layer flagged it as suspicious given the
image was already in a tracked state.

### Fix

Changed all barriers to use the actual current layout as `oldLayout`:

- Output textures (written by compute): `SHADER_READ_ONLY_OPTIMAL → GENERAL`
  before dispatch, `GENERAL → SHADER_READ_ONLY_OPTIMAL` after.
- Source textures (`hasSourceTex = true`, e.g. `house_01_E_GEN` used as input
  for the NRM generation pass): explicit `SHADER_READ_ONLY → GENERAL` barrier
  before dispatch, `GENERAL → SHADER_READ_ONLY` barrier after, so the image is
  in the correct layout when the compute shader reads from it as a storage image.

---

## 19. `0b7ae636` — Fix `VUID-VkWriteDescriptorSet-descriptorType-00339`

**Files:** `IntrinsicRendererRenderSystem.cpp`, `IntrinsicCoreResourcesMesh.cpp`,
`IntrinsicRendererRenderProcess.cpp`, `IntrinsicRendererResourcesImage.cpp`

### Background: bindless texture system and fallback

The engine registers all runtime-generated textures in a global bindless array.
When a texture generation pass for asset `X` runs, it writes its output into a
slot in that array. The descriptor for each slot specifies the image and its
required layout. For storage images the type must be
`VK_DESCRIPTOR_TYPE_STORAGE_IMAGE` and the image must have
`VK_IMAGE_USAGE_STORAGE_BIT`.

If `getResourceByName("X")` cannot find a texture, it returns a fallback:
the `checkerboard` debug texture. This texture is a compressed BC1 SRGB image.
Compressed formats cannot have `VK_IMAGE_USAGE_STORAGE_BIT` because the Vulkan
spec does not allow storage access to block-compressed images.

### The specific failure

The entry for `concrete_GEN` (the base colour texture for the concrete material,
needed as a source by `concrete_NRM_GEN` and `concrete_PBR_GEN` generation
shaders) had been commented out in `RenderSystem.cpp` during earlier debugging.
With that entry missing, `getResourceByName("concrete_GEN")` returned the
`checkerboard` fallback. The generation passes for `concrete_NRM_GEN` (compute
call index 14) and `concrete_PBR_GEN` (index 15) then tried to write the
fallback into a `STORAGE_IMAGE` descriptor slot, triggering
`VUID-VkWriteDescriptorSet-descriptorType-00339` — the validation error that
fires when a `STORAGE_IMAGE` descriptor is given an image without `STORAGE_BIT`.

This was a multi-step failure that required tracing the descriptor write error
back through the resource lookup system to the commented-out entry.

### Fix

Uncommented the `concrete_GEN` registration line. Also replaced the reference to
`texture_concrete_generation.comp` (a shader that does not exist in the asset
directory) with the existing and functionally equivalent
`texture_bricks_generation.comp`. Removed debug logging that was added during
the investigation (several `_INTR_LOG_WARNING` calls in the resource lookup path).

---

## 20. `21964455` — Remove `/* PJ: */` markers from RenderProcess.cpp

**Files:** `IntrinsicRendererRenderProcess.cpp`

Removed 4 stale `/* PJ: */` markers from `_renderStepTypeMapping` and
`_renderStepFunctionMapping` that were left from development-time bookkeeping.
No logic change.

---

## 21. `21816ab3` — Remove `_*Rendered` flags from render passes

**Files:** `IntrinsicRendererRenderPassBloom.cpp`,
`IntrinsicRendererRenderPassClustering.cpp`,
`IntrinsicRendererRenderPassPerPixelPicking.cpp`,
`IntrinsicRendererRenderPassShadow.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`

### Why the flags were added

Commits `bd927c75` and `fb5bb76a` introduced five boolean flags
(`_bloomRendered`, `_clusteringRendered`, `_pickingRendered`, `_shadowRendered`,
`_volLightingRendered`) as guards for first-frame vs. subsequent-frame image
layout selection. The reasoning at the time was: "On the first frame the image is
in `UNDEFINED`; on subsequent frames it is in some known layout, so we must tell
the barrier the actual layout."

### Why the flags are unnecessary

The reasoning was correct but overlooked a key Vulkan guarantee: specifying
`VK_IMAGE_LAYOUT_UNDEFINED` as `oldLayout` in a barrier is **always** legal,
regardless of what the image's actual current layout is. The spec explicitly
states that `UNDEFINED` as `oldLayout` means "I don't care about the existing
content" — the driver may discard it. The validation layer accepts this without
error because it is valid by spec.

More importantly, each render pass has an `init()` function (and
`onReinitRendering()` for reinit events) that runs before the first `render()`
call. These init functions already issue barriers that transition images from
`UNDEFINED` into their operational layouts. By the time `render()` runs for the
first time, the images are already in the correct layouts. Using `UNDEFINED` as
`oldLayout` in the per-frame barriers is therefore safe for every frame, not just
the first one: the driver discards the content and transitions to the new layout,
which is exactly what the barrier needs to do anyway.

After removing all five flags and simplifying all barriers to unconditionally use
`VK_IMAGE_LAYOUT_UNDEFINED` as `oldLayout`, the engine ran with zero validation
warnings. The flags are removed along with their reset calls in
`onReinitRendering()` and their set calls at the end of `render()`.

---

## 22. `d3116793` — Fix UB: dangling reference and missing return path

**Files:** `IntrinsicRendererPseudoInstancing.cpp`,
`IntrinsicRendererRenderPassDynamicGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassDynamicGeometryGeneration.h`

### Problem 1 (C4172): dangling reference in `getNormal`

`DynamicGeometryGeneration::getNormal()` was declared as:

```cpp
static glm::vec3& getNormal(DynamicGeneratedMesh& mesh, int x, int y, int z);
```

The function body computed a `glm::vec3` value locally and returned a reference
to it. In C++ a temporary object constructed inside a function is destroyed when
the function returns. A reference to that temporary is immediately dangling — it
points to memory that has been reclaimed. Accessing it is undefined behavior:
the value may appear correct (stack memory hasn't been overwritten yet), or it
may be garbage, or it may crash. MSVC C4172 warns about this pattern.

Fixed by changing the return type to `glm::vec3` (return by value). The function
signature in the header was updated to match.

### Problem 2 (C4715): missing return in `getMeshSizes`

`PseudoInstancing::getMeshSizes()` iterates through the `meshes` list looking
for a mesh by name and returns a reference to the matching entry. If the loop
exits without finding a match, the function falls off the end with no `return`
statement:

```cpp
// Before fix:
for (auto& i : meshes) {
    if (meshName == *(i->name)) return i;
}
// control reaches here — no return — UB
```

MSVC C4715 warns: "not all control paths return a value." Reaching the end of a
non-void function without returning is undefined behavior in C++. In practice
on MSVC x64 the function would return whatever garbage happened to be in the
`rax` register.

In practice `getMeshSizes` is only ever called after `isInstancedMesh` returned
`true`, which guarantees the mesh exists. The fix therefore adds an assertion
to document the invariant, followed by a fallback return that satisfies the
compiler without introducing false logic:

```cpp
_INTR_ASSERT(false && "getMeshSizes: mesh not found");
return meshes.front();
```

---

## 23. `ba9758bb` — Fix C4267/C4018/C4305 compiler warnings

**Files:** `IntrinsicCoreResourcesMesh.cpp`, `IntrinsicCoreWorld.cpp`,
`IntrinsicRendererRenderPassDynamicTextureGeneration.cpp`,
`IntrinsicRendererRenderPassGeometryGeneration.cpp`,
`IntrinsicRendererRenderSystem.cpp`

These are not logic bugs but are worth fixing: some can silently truncate values
at runtime, and on MSVC with `/W4` they produce warnings that obscure genuinely
important diagnostics in the build log.

### C4267 — implicit narrowing from `size_t` to `uint32_t`

`std::vector::size()` returns `size_t` (64-bit on x64). Assigning it to a
`uint32_t` without a cast silently truncates if the container ever exceeds 4
billion elements (unlikely in practice, but the implicit conversion is
semantically misleading). Fixed by adding explicit `(uint32_t)` casts in 9
places across `ResourcesMesh.cpp` and `World.cpp`.

### C4018 — signed/unsigned mismatch in loop comparisons

`for (int j = 0; j < sizeX * sizeZ; ++j)` where `sizeX` and `sizeZ` are
`uint32_t` forces a signed/unsigned comparison. On platforms where
`sizeX * sizeZ` overflows a signed integer, the comparison result is
implementation-defined. Fixed by changing loop variables to `uint32_t` and
using a `(uint32_t)` cast on the limit expression.

### C4305 — truncation from `double` to `float`

Floating-point literals without the `f` suffix (e.g. `0.02`, `2.0`, `0.5`) are
`double` in C++. Assigning or passing them to `float` parameters triggers an
implicit narrowing conversion. While the precision loss is negligible for the
values used here, the warnings were noisy. Fixed by adding the `f` suffix to all
affected literals (`0.02f`, `2.0f`, `0.5f`, `0.1f`, `1.01f`, `5.0f`, `-5.0f`,
`2.0f * 3.1415f`, etc.) across several geometry and texture generation files.

---

## Summary

| Category | Commits |
|----------|---------|
| Build / environment | 1 |
| Decal fix (GLSL) | 1 |
| Vulkan validation errors (barriers, layouts, extensions) | 16 |
| Code cleanup | 2 |
| UB fixes and compiler warnings | 2 |
| **Total** | **22** |
