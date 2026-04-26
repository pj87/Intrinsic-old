# Changes in branch `fixing_for_new_driver`

This branch fixes engine compatibility with new NVIDIA drivers and a newer build
environment (MSVC 14.44, Vulkan SDK 1.3+). Below is a detailed description of
each commit in chronological order.

---

## 1. `670290a1` — Fix build with new MSVC and Vulkan SDK

**Files:** `IntrinsicCore/src/stdafx.h`, `cmake/FindLuaJIT.cmake`

MSVC 14.44 stopped including `<chrono>` transitively through `<thread>`, causing
a compilation error. Added an explicit `#include <chrono>` to `stdafx.h`.

`FindLuaJIT.cmake` searched for `libluajit.a` (GCC format) first, so on Windows
with MSVC it failed to find the correct library. Changed the search order so that
`lua51.lib` is tried first.

---

## 2. `460a12dd` — Fix decals on new NVIDIA drivers

**Files:** `app/assets/shaders/decals.frag.glsl`,
`app/assets/shaders/decals.inc.glsl`,
`IntrinsicRenderer/src/IntrinsicRendererRenderPassClustering.cpp`,
`IntrinsicRenderer/src/IntrinsicRendererResourcesPipeline.cpp`

**Problem:** `calcDecal` accepted `sampler2D[4095]` as a function parameter.
Passing sampler arrays as function arguments is not legal in GLSL and caused a
GPU hang on new drivers.

**Fix:** Removed the array from the parameter list; the shader now accesses
`globalTextures` directly (the same way `lighting.inc.glsl` handles
`globalCubeTextures`). Also removed debug hacks and properly wired up the
pipeline creation and draw call for decals.

---

## 3. `ab909002` — Remove stale debug comment in Clustering

**Files:** `IntrinsicRenderer/src/IntrinsicRendererRenderPassClustering.cpp`

Minor cleanup — removed an outdated debug comment.

---

## 4. `cf298e55` — First Vulkan validation errors (category A)

**Files:** `IntrinsicRenderer/src/IntrinsicRendererHelper.h`,
`IntrinsicRenderer/src/IntrinsicRendererRenderSystem.cpp`

**Problem 1:** The Vulkan spec forbids non-zero `srcAccessMask` / `dstAccessMask`
when the pipeline stage is `VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT`. Many places in
the code ignored this requirement, producing validation errors on new drivers.
Fixed in the barrier helper by zeroing access masks when the stage is
`TOP_OF_PIPE`.

**Problem 2:** The `VK_EXT_debug_marker` extension requires `VK_EXT_debug_report`
as a device extension. The engine tried to enable debug markers without checking
whether debug report was available. Added availability checks for both extensions
before enabling them.

---

## 5. `fa3d8e07` — Detect and enable `VK_KHR_maintenance2`

**Files:** `IntrinsicRenderer/src/IntrinsicRendererRenderSystem.cpp`

`VK_KHR_maintenance2` is required by `VkImageViewUsageCreateInfo` (used in the
next step to fix SRGB image views). Added detection and conditional enabling of
this extension during logical device creation.

---

## 6. `e0b33ce6` — Fix pre-present and post-present barriers

**Files:** `IntrinsicRenderer/src/IntrinsicRendererRenderSystem.h`

**Pre-present barrier:** `dstAccessMask` was set to `VK_ACCESS_MEMORY_READ_BIT`,
but the spec requires `0` for the `BOTTOM_OF_PIPE` stage.

**Post-present barrier:** `dstStage` was `BOTTOM_OF_PIPE`, making
`VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT` invalid. Changed to
`COLOR_ATTACHMENT_OUTPUT`.

---

## 7. `fd363ee7` — Fix barriers for `TOP_OF_PIPE` and `BOTTOM_OF_PIPE`

**Files:** `IntrinsicRenderer/src/IntrinsicRendererHelper.h`

Extended the image and buffer barrier helpers: when the stage is `TOP_OF_PIPE`
or `BOTTOM_OF_PIPE`, access masks are automatically zeroed (per the Vulkan spec).
Previously many barriers had non-zero access masks at these stages, violating the
spec.

---

## 8. `dd8082e4` — Add `STORAGE_BIT` flag to vertex buffers

**Files:** `IntrinsicRenderer/src/IntrinsicRendererHelper.h`

Vertex buffers used in compute shaders (marching cubes, geometry generation)
must have `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT` set. Added it to the default
flag set in the buffer creation helper.

---

## 9. `689f3ea7` — Guard against double `vkBindBufferMemory`

**Files:** `IntrinsicRenderer/src/IntrinsicRendererResourcesBuffer.cpp`

`vkBindBufferMemory` must only be called on a fresh allocation. New drivers treat
re-binding as a validation error. Added a condition that checks whether an
allocation actually occurred before calling `vkBindBufferMemory`.

---

## 10. `73af179b` — `VkImageViewUsageCreateInfo` for SRGB image views

**Files:** `IntrinsicRenderer/src/IntrinsicRendererResourcesImage.cpp`

SRGB textures create two views: one as SRGB (for sampling) and one as UNORM (for
storage). New drivers require explicitly declaring allowed usages per view via
`VkImageViewUsageCreateInfo`. Added the struct for the SRGB view with only
`VK_IMAGE_USAGE_SAMPLED_BIT` (no `STORAGE_BIT`), so the validation layer does
not complain about usage mismatches.

---

## 11. `1d7e94ab` — Fix image barriers and buffer updates in geometry/texture generation

**Files:** `IntrinsicRendererRenderPassDynamicGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassDynamicTextureGeneration.cpp`,
`IntrinsicRendererRenderPassGeometryGeneration.cpp`

**Problem 1 (critical):** `updateDataMemory` called `vkCmdCopyBuffer` with the
same buffer as both source and destination — undefined behavior. Replaced with a
direct `memcpy` into host-visible memory.

**Problem 2:** Missing `UNDEFINED→GENERAL` transitions for `normalsTex` and
generated textures during initialization caused compute shaders to encounter
images in an undefined layout.

**Problem 3:** `TRANSFER_SRC/DST` barriers were replaced with `GENERAL` using
correct pipeline stages.

**Problem 4:** Added a `GENERAL→SHADER_READ_ONLY_OPTIMAL` transition before the
marching cubes dispatch.

---

## 12. `bd927c75` — Fix barrier stages in Bloom and VolumetricLighting

**Files:** `IntrinsicRendererRenderPassBloom.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`

**Bloom:** The barrier after the lum dispatch used `TOP_OF_PIPE` as `srcStage`
instead of `COMPUTE_SHADER_BIT`. Added a first-frame guard so subsequent frames
use the actual `SHADER_READ_ONLY` layout instead of `UNDEFINED`. Fixed `dstStage`
for the final `_lumImageRef` barrier to `FRAGMENT_SHADER_BIT`.

**VolumetricLighting:** Barriers after the accumulation dispatch used `TOP_OF_PIPE`
instead of `COMPUTE_SHADER_BIT` as `srcStage`. Added a first-frame guard. Fixed
`dstStage` of the scatter buffer barrier to `FRAGMENT_SHADER_BIT`.

---

## 13. `fb5bb76a` — Fix `UNDEFINED` layout in Shadow, Clustering, VolumetricLighting, PerPixelPicking

**Files:** `IntrinsicRendererRenderPassShadow.cpp`,
`IntrinsicRendererRenderPassClustering.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`,
`IntrinsicRendererRenderPassPerPixelPicking.cpp`

**Problem:** From the second frame onward every image is in a known, concrete
layout. Using `UNDEFINED` as `oldLayout` in barriers triggered layout tracking
errors on new drivers.

**Fix:** Added `_shadowRendered`, `_clusteringRendered`, `_volLightingRendered`,
`_pickingRendered` flags. The first frame uses `UNDEFINED`; subsequent frames
supply the actual end-of-frame layout from the previous frame
(`SHADER_READ_ONLY_OPTIMAL`, `DEPTH_STENCIL_ATTACHMENT_OPTIMAL`,
`TRANSFER_SRC_OPTIMAL`). Flags are reset in `onReinitRendering`.

---

## 14. `c5ddc856` — Fix buffer barrier stages in geometry/marching cubes

**Files:** `IntrinsicRendererRenderPassDynamicGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassMarchingCubes.cpp`

Barriers using the default `TOP_OF_PIPE` stage had their access masks zeroed by
the helper (see `fd363ee7`), making them effectively no-ops.

**Fixed:** Voxel buffers: `COMPUTE→COMPUTE`. Output vertex buffers (position,
normal, etc.): `COMPUTE→VERTEX_INPUT` with `dstAccessMask =
VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT`, because they are consumed as vertex
attributes after the marching cubes dispatch.

---

## 15. `9bfb146d` — Fix barrier stages in texture upload and PerPixelPicking

**Files:** `IntrinsicRendererResourcesImage.cpp`,
`IntrinsicRendererRenderPassPerPixelPicking.cpp`

**ResourcesImage:** The `UNDEFINED→TRANSFER_DST` barrier now uses
`TOP_OF_PIPE→TRANSFER`. The `TRANSFER_DST→SHADER_READ_ONLY` barrier uses
`TRANSFER→FRAGMENT_SHADER`. Applies to all texture upload paths.

**PerPixelPicking:** The `TRANSFER_SRC→COLOR_ATTACHMENT` barrier uses
`TRANSFER→COLOR_ATTACHMENT_OUTPUT`. The reverse `COLOR_ATTACHMENT→TRANSFER_SRC`
uses `COLOR_ATTACHMENT_OUTPUT→TRANSFER`. The depth barrier is skipped on
subsequent frames (the render pass handles the transition itself).

---

## 16. `750aa2fd` — Fix first-frame layout errors in VolumetricLighting

**Files:** `IntrinsicRendererRenderPassVolumetricLighting.cpp`,
`IntrinsicRendererRenderSystem.cpp`, `IntrinsicRendererRenderProcess.cpp`,
`IntrinsicCoreApplication.cpp`

All ESM and volumetric lighting images are initialized to
`SHADER_READ_ONLY_OPTIMAL` in `init()` via a temporary command buffer. This
ensures draw calls that bind the full image array (all layers) do not see
`UNDEFINED` in any layer on the first frame. The `_volLightingRendered` flag is
set to `true` already in `init()` so per-frame barriers start from
`SHADER_READ_ONLY` rather than `UNDEFINED`.

---

## 17. `d8793ced` — Transition images to `SHADER_READ_ONLY` after reinit

**Files:** `IntrinsicRendererRenderProcess.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`,
`IntrinsicRendererResourcesImage.cpp`

**Problem:** Resolution-dependent images (recreated on every reinit) start in
`UNDEFINED` layout. Global texture descriptors bind these images as
`SHADER_READ_ONLY_OPTIMAL`, causing a validation error on the first submit after
reinit.

**Fix:** After all images are created in `loadRendererConfig()`, a temporary
command buffer transitions them from `UNDEFINED` to `SHADER_READ_ONLY_OPTIMAL`.
Also fixed `VolumetricLighting::onReinitRendering()` to properly initialize
static (non-resolution-dependent) images to `SHADER_READ_ONLY` after reinit.

---

## 18. `57e8245a` — Fix `VUID-VkDescriptorImageInfo-imageLayout-00344` in DynamicTextureGeneration

**Files:** `IntrinsicRendererRenderPassDynamicTextureGeneration.cpp`,
`IntrinsicRendererRenderSystem.cpp`, `IntrinsicCoreApplication.cpp`

**Problem:** The texture generation compute shaders declare `_TextureTex` and
`_SourceTex` as `image2D` (storage images), requiring `GENERAL` layout. The
barrier before the dispatch used `UNDEFINED` as `oldLayout`, which the validation
layer tracked incorrectly.

**Fix:** Barriers now use the actual current layout
(`SHADER_READ_ONLY_OPTIMAL→GENERAL`). Source textures (`hasSourceTex=true`) get
explicit `SHADER_READ_ONLY→GENERAL` barriers before the dispatch and
`GENERAL→SHADER_READ_ONLY` after, so `house_01_E_GEN` is in `GENERAL` when the
NRM generation dispatch accesses it via `_SourceTex`.

---

## 19. `0b7ae636` — Fix `VUID-VkWriteDescriptorSet-descriptorType-00339`

**Files:** `IntrinsicRendererRenderSystem.cpp`, `IntrinsicCoreResourcesMesh.cpp`,
`IntrinsicRendererRenderProcess.cpp`, `IntrinsicRendererResourcesImage.cpp`

**Problem (deep analysis):** `concrete_NRM_GEN` (compute call index 14) and
`concrete_PBR_GEN` (index 15) used `concrete_GEN` as their source texture. The
single-texture entry for `concrete_GEN` was commented out in `RenderSystem.cpp`,
so `getResourceByName("concrete_GEN")` fell back to the `checkerboard` texture in
BC1 SRGB format. BC1 does not have `VK_IMAGE_USAGE_STORAGE_BIT`, so
`vkUpdateDescriptorSets` reported a validation error when trying to bind it as
`VK_DESCRIPTOR_TYPE_STORAGE_IMAGE`.

**Fix:** Uncommented the entry for `concrete_GEN` and replaced the non-existent
`texture_concrete_generation.comp` shader with the existing
`texture_bricks_generation.comp`. Also removed debug logging added during
investigation.

---

## 20. `21964455` — Remove `/* PJ: */` markers from RenderProcess.cpp

**Files:** `IntrinsicRendererRenderProcess.cpp`

Removed 4 stale debug markers `/* PJ: */` from `_renderStepTypeMapping` and
`_renderStepFunctionMapping`.

---

## 21. `21816ab3` — Remove `_*Rendered` flags from render passes

**Files:** `IntrinsicRendererRenderPassBloom.cpp`,
`IntrinsicRendererRenderPassClustering.cpp`,
`IntrinsicRendererRenderPassPerPixelPicking.cpp`,
`IntrinsicRendererRenderPassShadow.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`

The flags `_bloomRendered`, `_clusteringRendered`, `_pickingRendered`,
`_shadowRendered`, `_volLightingRendered` (added in commits `bd927c75` and
`fb5bb76a`) turned out to be unnecessary. `VK_IMAGE_LAYOUT_UNDEFINED` as
`oldLayout` is always accepted by the driver and the validation layer, because
`init()` / `onReinitRendering()` already transition images to the correct layout
before the first `render()` call. The engine works correctly without the flags,
with no validation warnings.

---

## 22. `d3116793` — Fix UB: dangling reference and missing return path

**Files:** `IntrinsicRendererPseudoInstancing.cpp`,
`IntrinsicRendererRenderPassDynamicGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassDynamicGeometryGeneration.h`

**Problem 1 (C4172):** `DynamicGeometryGeneration::getNormal()` was declared as
returning `glm::vec3&`, but internally constructed a temporary `glm::vec3(...)`
and returned a reference to it — classic dangling reference (UB). Changed the
return type to `glm::vec3` (by value).

**Problem 2 (C4715):** `PseudoInstancing::getMeshSizes()` had no `return`
statement on the code path where the loop finishes without finding a match —
undefined behavior. Added `_INTR_ASSERT(false)` and a fallback return to satisfy
the compiler (the function is only called after a positive result from
`isInstancedMesh`).

---

## 23. `ba9758bb` — Fix C4267/C4018/C4305 compiler warnings

**Files:** `IntrinsicCoreResourcesMesh.cpp`, `IntrinsicCoreWorld.cpp`,
`IntrinsicRendererRenderPassDynamicTextureGeneration.cpp`,
`IntrinsicRendererRenderPassGeometryGeneration.cpp`,
`IntrinsicRendererRenderSystem.cpp`

- **C4267** (`size_t→uint32_t`): added explicit `(uint32_t)` casts on assignments
  from vector `.size()` in `ResourcesMesh.cpp` (8 places) and `World.cpp`
  (1 place).
- **C4018** (signed/unsigned mismatch): `for (int j = ...)` loops comparing
  against `uint32_t` changed to `for (uint32_t j = 0u; ...)` in `ResourcesMesh.cpp`
  and `World.cpp`.
- **C4305** (`double→float`): added `f` suffix to floating-point literals
  (`0.02f`, `2.0f`, `1.0f`, `0.1f`, `1.01f`, `5.0f`, `-5.0f`, `2.0f * 3.1415f`,
  etc.) across several geometry and texture generation files.

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
