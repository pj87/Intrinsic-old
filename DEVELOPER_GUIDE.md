# Intrinsic Engine — Developer Guide

Practical reference for day-to-day work with the engine. Assumes the project
builds and runs. See `README.md` / `GETTING_STARTED.md` for initial setup.

---

## Table of Contents

1. [Project Layout](#1-project-layout)
2. [Build & Run](#2-build--run)
3. [World & Entity System](#3-world--entity-system)
4. [Components Reference](#4-components-reference)
5. [Material System](#5-material-system)
6. [Shader System](#6-shader-system)
7. [Render Passes](#7-render-passes)
   - 7.1 [Frame execution flow](#71-frame-execution-flow)
   - 7.2 [Resource creation patterns](#72-resource-creation-patterns)
   - 7.3 [The two dispatch patterns](#73-the-two-dispatch-patterns)
   - 7.4 [Adding a render pass: complete walkthrough](#74-adding-a-render-pass-complete-walkthrough)
   - 7.5 [Image memory barriers — rules](#75-image-memory-barriers--rules)
   - 7.6 [Accessing a render target from another pass](#76-accessing-a-render-target-from-another-pass)
8. [Example: Forward Transparent Pass](#8-example-forward-transparent-pass)
9. [Example: Ray Tracing Pass](#9-example-ray-tracing-pass)
10. [Asset Pipeline](#10-asset-pipeline)
11. [Lua Scripting](#11-lua-scripting)
12. [Procedural Systems](#12-procedural-systems)
13. [Physics](#13-physics)
14. [Debugging & Profiling](#14-debugging--profiling)
15. [Common Recipes](#15-common-recipes)

---

## 1. Project Layout

```
Intrinsic/
├── IntrinsicCore/          # Engine core: ECS, world, input, physics, scripting
│   └── src/
├── IntrinsicRenderer/      # Vulkan renderer: all render passes, GPU resources
│   └── src/
├── IntrinsicEd/            # Qt5 editor (Windows only)
│   └── src/
├── IntrinsicAssetManagement/  # FBX importer, texture processor
│   └── src/
├── Intrinsic/              # Standalone game entry point (main.cpp + game states)
│   └── src/
├── app/
│   ├── assets/
│   │   └── shaders/        # All GLSL shader source files
│   ├── config/
│   │   ├── renderer_config.json       # Images, framebuffers, render pass graph
│   │   └── material_pass_config.json  # Material pass definitions
│   ├── managers/
│   │   ├── materials/      # *.material.json
│   │   ├── meshes/         # *.mesh.json
│   │   ├── gpu_programs/   # *.gpu_program.json
│   │   ├── images/         # *.image.json
│   │   └── scripts/        # *.script.json
│   ├── scripts/            # Lua scripts
│   ├── worlds/             # *.world.json
│   └── settings.json
├── cmake/                  # FindXxx.cmake modules
└── dependencies/           # Vendored third-party libs (GLM, RapidJSON, etc.)
```

### Key source file naming

| Prefix | Meaning |
|--------|---------|
| `IntrinsicCore*` | Core engine (no renderer dependency) |
| `IntrinsicRenderer*` | Renderer (Vulkan, GPU resources) |
| `IntrinsicRendererRenderPass*` | A specific render pass |
| `IntrinsicRendererResources*` | A GPU resource manager (Image, Buffer, Pipeline, …) |
| `CComponents::` / `CResources::` | Core component / resource namespace |
| `RResources::` | Renderer resource namespace |

---

## 2. Build & Run

### CMake configuration

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
```

Useful CMake options:

| Option | Default | Purpose |
|--------|---------|---------|
| `INTR_BUILD_STANDALONE_APP` | ON | Build `Intrinsic.exe` |
| `INTR_BUILD_INTRINSICED` | ON | Build `IntrinsicEd.exe` (requires Qt5) |
| `INTR_USE_MICROPROFILE` | ON | Enable GPU/CPU profiler (Windows only) |
| `INTR_FINAL_BUILD` | OFF | Strip editor code, enable full optimizations |

### Runtime working directory

Both executables expect the working directory to be `app/`. Either set it in the
VS project properties or launch from there:

```
cd app
../build/Release/Intrinsic.exe
```

### Settings

`app/settings.json` controls startup behaviour:

```json
{
  "initialGameState": "Editing",   // "Main" for game, "Editing" for editor
  "rendererConfig": "renderer_config.json",
  "startupWorld": "Default"
}
```

---

## 3. World & Entity System

### Architecture

Entities are just a **name**. All data lives in **components**. Every component
has a corresponding `*Manager` static class that holds all instances in flat
parallel arrays (Structure of Arrays — cache-friendly). References to components
are 32-bit generational handles (`Ref` = 24-bit index + 8-bit generation counter)
that automatically invalidate after the component is destroyed.

### World files

Worlds are stored as JSON arrays in `app/worlds/`. Each element is one node:

```json
[
  {
    "name": "MyObject",
    "offsetToParent": -1,
    "propertyEntries": [
      {
        "type": "Node",
        "properties": {
          "localPos": [0.0, 0.0, 0.0],
          "localOrient": [0.0, 0.0, 0.0, 1.0],
          "localSize": [1.0, 1.0, 1.0]
        }
      },
      {
        "type": "Mesh",
        "properties": {
          "meshName": "my_mesh",
          "materialName": "my_material"
        }
      }
    ]
  }
]
```

`offsetToParent` is the index offset from the current element to its parent
(-1 means root). Children must appear after their parent in the array.

### Loading / saving worlds in code

```cpp
// Load
World::loadWorld("Default");          // loads app/worlds/Default.world.json

// Save (editor)
World::saveWorld("Default");

// Create entity at runtime
Entity::EntityRef entity = Entity::EntityManager::createEntity(Name("MyThing"));
NodeRef node = NodeManager::createComponent(entity);
NodeManager::setPosition(node, glm::vec3(0, 10, 0));
NodeManager::rebuildTreeAndUpdateTransforms();
```

### Entity lookup

```cpp
Entity::EntityRef ref = Entity::EntityManager::getEntityByName(Name("MyThing"));
NodeRef node = NodeManager::getComponentForEntity(ref);
MeshRef mesh = MeshManager::getComponentForEntity(ref);   // may be invalid
```

Always validate before use:
```cpp
if (!node.isValid()) { /* entity has no Node component */ }
```

### Transform helpers (NodeManager)

```cpp
NodeManager::getPosition(nodeRef);           // world position
NodeManager::getWorldTransform(nodeRef);     // glm::mat4
NodeManager::setPosition(nodeRef, pos);
NodeManager::setOrientation(nodeRef, quat);
NodeManager::setSize(nodeRef, scale);
NodeManager::rebuildTreeAndUpdateTransforms();  // must call after bulk changes
```

### Parenting

```cpp
// In world JSON: set offsetToParent
// In code:
NodeManager::attachChild(parentRef, childRef);
NodeManager::detachChild(parentRef, childRef);
```

---

## 4. Components Reference

### Node

Every entity in the scene must have a Node component. It stores the spatial
transform and connects the entity into the scene hierarchy.

| Property | Type | Description |
|----------|------|-------------|
| `localPos` | vec3 | Local position relative to parent |
| `localOrient` | quat (xyzw) | Local rotation |
| `localSize` | vec3 | Local scale |
| `visibilityMask` | uint32 | Bitmask for visibility culling |

Key constants: `NodeManager::_maxNodeCount = 10240`

### Mesh

Attaches renderable geometry to a node. The mesh asset and material are looked
up by name at load time.

| Property | Type | Description |
|----------|------|-------------|
| `meshName` | string | Filename stem in `app/managers/meshes/` |
| `materialName` | string | Filename stem in `app/managers/materials/` |

Changing the mesh at runtime:
```cpp
MeshManager::setMeshName(meshRef, Name("new_mesh"));
MeshManager::updateMeshData(meshRef);  // reloads vertex/index data
```

### Camera

```json
{
  "type": "Camera",
  "properties": {
    "fov": 60.0,
    "nearPlane": 0.1,
    "farPlane": 5000.0
  }
}
```

The active camera is set by `World::setActiveCamera(cameraRef)`. The renderer
reads it each frame from `World::getActiveCamera()`.

### Light

```json
{
  "type": "Light",
  "properties": {
    "lightType": 0,
    "color": [1.0, 0.95, 0.8],
    "intensity": 100.0,
    "radius": 20.0
  }
}
```

| `lightType` | Meaning |
|-------------|---------|
| 0 | Point light |
| 1 | Directional (sun) |

The directional light direction is derived from the node's orientation.

### Decal

Projected decal. Requires the decal render pass to be active.

```json
{
  "type": "Decal",
  "properties": {
    "materialName": "decal_material",
    "thickness": 1.0
  }
}
```

### RigidBody

```json
{
  "type": "RigidBody",
  "properties": {
    "rigidBodyType": 0,
    "mass": 1.0,
    "shapeType": 1
  }
}
```

| `rigidBodyType` | Meaning |
|-----------------|---------|
| 0 | Static |
| 1 | Dynamic |
| 2 | Kinematic |

| `shapeType` | Meaning |
|-------------|---------|
| 0 | Sphere |
| 1 | Box (uses node scale) |
| 2 | Capsule |
| 3 | Convex hull (from mesh) |

### Script

```json
{
  "type": "Script",
  "properties": {
    "scriptName": "camera"
  }
}
```

References a file in `app/managers/scripts/` which points to a Lua file in
`app/scripts/`. See [Section 9](#9-lua-scripting).

### IrradianceProbe / SpecularProbe

Baked IBL probes. Place them in the scene and bake from the editor
(`Renderer → Bake Irradiance Probes`). Probe data is stored alongside the
world file.

```json
{
  "type": "IrradianceProbe",
  "properties": {
    "radius": 50.0,
    "priority": 0
  }
}
```

### PostEffectVolume

Overrides global post-processing parameters within a volume.

```json
{
  "type": "PostEffectVolume",
  "properties": {
    "postEffectName": "my_post_effect",
    "blendRange": 5.0
  }
}
```

---

## 5. Material System

### Material JSON file

Located in `app/managers/materials/`, one file per material:

```json
{
  "name": "my_material",
  "albedoTextureName": "my_albedo",
  "normalTextureName": "my_normal",
  "pbrTextureName": "my_pbr",
  "emissiveTextureName": "",
  "emissiveIntensity": 0.0,
  "roughnessBias": 0.0,
  "metallicBias": 0.0,
  "specularBias": 0.0,
  "uvOffsetScale": [0.0, 0.0, 1.0, 1.0],
  "uvAnimation": [0.0, 0.0],
  "translucencyThickness": 0.0,
  "materialPassMask": 3
}
```

### Texture packing convention

The PBR texture (`*_pbr`) packs three channels:

| Channel | Data |
|---------|------|
| R | Metallic |
| G | Roughness |
| B | Ambient Occlusion |

This is the standard metallic-roughness workflow (same as glTF).

### `materialPassMask`

Bitmask that selects which render passes consume this material:

| Bit | Pass |
|-----|------|
| 0 (0x01) | GBuffer (opaque geometry) |
| 1 (0x02) | Shadow |
| 2 (0x04) | PerPixelPicking |
| 3 (0x08) | Transparency / custom |

A typical opaque material uses `3` (GBuffer + Shadow).

### Image JSON file

Located in `app/managers/images/`:

```json
{
  "name": "my_albedo",
  "fileName": "textures/my_albedo.dds",
  "samplerName": "LinearRepeat",
  "addressModeU": 0,
  "addressModeV": 0,
  "generateMipmaps": true
}
```

Textures must be in DDS format (BC1–BC7). Use the asset management tools or
`texconv.exe` from the DirectX Texture Tool to convert.

### GPU Program JSON file

Located in `app/managers/gpu_programs/`:

```json
{
  "name": "gbuffer_default",
  "shaderFileName": "gbuffer.vert.glsl",
  "entryPoint": "main",
  "defines": [],
  "programType": 0
}
```

| `programType` | Stage |
|---------------|-------|
| 0 | Vertex |
| 1 | Fragment |
| 2 | Compute |
| 3 | Geometry |

---

## 6. Shader System

### File naming convention

```
app/assets/shaders/
├── gbuffer.vert.glsl              # Vertex shader
├── gbuffer.frag.glsl              # Fragment shader
├── lighting.comp.glsl             # Compute shader
├── lib_buffers.glsl               # Shared buffer layout declarations
├── lib_lighting.glsl              # Shared lighting functions
├── lib_noise.glsl                 # Noise utilities
├── lib_math.glsl                  # Math helpers
└── *.inc.glsl                     # Included fragments (not compiled standalone)
```

### Common includes

```glsl
#include "lib_buffers.glsl"    // uniform buffer bindings (per-frame, per-instance, etc.)
#include "lib_lighting.glsl"   // calcDiffuse, calcSpecular, etc.
#include "lib_math.glsl"       // helpers (saturate, linearizeDepth, etc.)
#include "lib_noise.glsl"      // simplex noise, fbm
```

### Bindless texture access

The engine uses a global bindless texture array. Access it with:

```glsl
// Fragment/compute shader
layout(set = 1, binding = 0) uniform sampler2D globalTextures[4095];

// In code
vec4 color = texture(globalTextures[u_AlbedoTextureIndex], uv);
```

The texture index is passed through the per-draw-call data or a uniform buffer.

### Key uniform buffers (from `lib_buffers.glsl`)

```glsl
// Per-frame (set 0, binding 0)
layout(...) uniform PerFrameData {
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 viewProjMatrix;
    mat4 invViewProjMatrix;
    vec4 camPos;
    vec4 nearFar;        // .x = near, .y = far
    vec4 lightDir;       // directional light direction
    vec4 lightColor;
    float deltaT;
    float time;
    ...
};

// Per-instance (set 0, binding 1)
layout(...) uniform PerInstanceData {
    mat4 worldMatrix;
    mat4 worldMatrixPrev;
    uint albedoTextureIndex;
    uint normalTextureIndex;
    uint pbrTextureIndex;
    ...
};
```

### Adding a new shader

1. Create `app/assets/shaders/my_shader.frag.glsl`
2. Create `app/managers/gpu_programs/my_shader_frag.gpu_program.json`
3. Reference the GPU program from your material or pass pipeline

Shaders are compiled from GLSL to SPIR-V at startup (via `glslang`). Compile
errors appear in the console. There is no offline pre-compilation step required
for development.

---

## 7. Render Passes

### 7.1 Frame execution flow

`RenderProcess::Default::renderFrame()` runs every frame:

```
1. RenderSystem::resizeSwapChain()          – handle window resize
2. RenderSystem::beginFrame()               – acquire swapchain image, begin primary cmd buf
3. Culling phase:
   a. Update cameras and build frustums
   b. Shadow::prepareFrustums()             – split camera frustum into PSSM slices
   c. CameraManager::updateFrustumsAndMatrices()
   d. FrustumManager::cullNodes()           – per-frustum AABB culling
   e. MeshManager::collectDrawCallsAndMeshComponents()
   f. UniformManager::resetAllocator()
4. executeRenderSteps(p_DeltaT)             – dispatch each step from renderer_config.json
5. RenderSystem::endFrame()                 – submit, present
```

`executeRenderSteps` iterates `_renderSteps` (parsed from JSON). For each step it
either dispatches a generic pass directly (`GenericFullscreen`, `GenericMesh`,
`GenericBlur`, `ImageMemoryBarrier`, `SwitchCamera`) or looks up the specialized
pass in `_renderStepFunctionMapping` and calls its `render()`.

**Culling and draw calls are pre-built before the render steps run.** The render
steps read from `RenderProcess::Default::_activeFrustums` and the corresponding
draw call lists — they do not cull themselves.

Default pass order:

```
RenderPassDynamicGeometryGeneration   – voxel + marching cubes compute
RenderPassDynamicTextureGeneration    – procedural texture compute
RenderPassGeometryGeneration          – static procedural geometry
SwitchCamera → ActiveCamera
RenderPassMarchingCubes               – render computed mesh geometry
RenderPassShadow                      – depth-only shadow maps (PSSM)
RenderPassVolumetricLighting          – ESM + scatter accumulation (compute)
RenderPassClustering                  – light culling + deferred lighting (fullscreen)
GenericMesh "GBuffer"                 – fills GBufferAlbedo/Normal/Parameter0/Depth
RenderPassBloom                       – lum downsample → blur → composite (compute)
GenericFullscreen "PostCombine"       – tone mapping + SMAA
RenderPassPerPixelPicking             – off-screen pick buffer (editor only)
RenderPassDebug                       – wireframes, AABBs, probes
```

---

### 7.2 Resource creation patterns

Every GPU resource follows the same three-step pattern:

```cpp
// 1. Allocate a named slot in the resource pool
ImageRef img = ImageManager::createImage(_N(MyImage));

// 2. Reset to defaults, then fill in your descriptor
ImageManager::resetToDefault(img);
ImageManager::addResourceFlags(img, Dod::Resources::ResourceFlags::kResourceVolatile);
ImageManager::_descDimensions(img)   = glm::uvec3(width, height, 1u);
ImageManager::_descImageFormat(img)  = Format::kR16G16B16A16Float;
ImageManager::_descImageType(img)    = ImageType::kTexture;
ImageManager::_descImageFlags(img)   = ImageFlags::kUsageAttachment
                                     | ImageFlags::kUsageSampled;

// 3. Batch-create (allocates VkImage, VkDeviceMemory, VkImageView)
ImageRefArray toCreate = { img };
ImageManager::createResources(toCreate);
```

`kResourceVolatile` marks resources that must be recreated on reinit
(resolution-dependent). Static resources (shadow maps, LUTs) omit this flag.

Key `ImageFlags`:

| Flag | Meaning |
|------|---------|
| `kUsageAttachment` | Can be used as color/depth attachment |
| `kUsageSampled` | Can be read in shaders via sampler |
| `kUsageStorage` | Can be bound as `image2D` in compute |

Key `Format` values (matching Vulkan formats):

| Constant | Format |
|----------|--------|
| `kR8G8B8A8UNorm` | `VK_FORMAT_R8G8B8A8_UNORM` |
| `kR16G16B16A16Float` | `VK_FORMAT_R16G16B16A16_SFLOAT` |
| `kR32SFloat` | `VK_FORMAT_R32_SFLOAT` |
| `kDepth` | Depth format chosen by `RenderSystem::_depthStencilFormatToUse` |

---

### 7.3 The two dispatch patterns

#### Graphics pass (rasterization)

```cpp
// 1. Transition the color attachment to writable
ImageManager::insertImageMemoryBarrier(
    _outputImageRef,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

// 2. Begin render pass
VkClearValue clears[2];
clears[0].color        = {{0.f, 0.f, 0.f, 1.f}};
clears[1].depthStencil = {1.f, 0u};
RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef,
                              VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
                              2u, clears);

// 3. Queue draw calls (they run in secondary cmd buffers in parallel)
DrawCallDispatcher::queueDrawCalls(visibleDrawCalls, _renderPassRef, _framebufferRef);

// OR for a single fullscreen triangle / quad:
RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef,
                              VK_SUBPASS_CONTENTS_INLINE, 1u, clears);
RenderSystem::dispatchDrawCall(_fullscreenDrawCallRef,
                               RenderSystem::getPrimaryCommandBuffer());

// 4. End render pass
RenderSystem::endRenderPass(_renderPassRef);

// 5. Transition to readable for later passes
ImageManager::insertImageMemoryBarrier(
    _outputImageRef,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
```

#### Compute pass

```cpp
VkCommandBuffer cmd = RenderSystem::getPrimaryCommandBuffer();

// 1. Transition storage images to GENERAL
ImageManager::insertImageMemoryBarrier(
    _outputImageRef,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_GENERAL,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

// 2. Upload per-dispatch uniforms
MyPerInstanceData data = { width, height, time };
ComputeCallManager::updateUniformMemory(
    {_computeCallRef}, &data, sizeof(data));

// 3. Dispatch
RenderSystem::dispatchComputeCall(_computeCallRef, cmd);

// 4. Transition output to readable
ImageManager::insertImageMemoryBarrier(
    _outputImageRef,
    VK_IMAGE_LAYOUT_GENERAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
```

---

### 7.4 Adding a render pass: complete walkthrough

**Every file that must be touched:**

| File | What to add |
|------|------------|
| `IntrinsicRendererRenderPassMyPass.h` | struct declaration |
| `IntrinsicRendererRenderPassMyPass.cpp` | implementation |
| `IntrinsicRenderer/CMakeLists.txt` | add to source list |
| `IntrinsicRendererRenderProcess.h` | `#include` the new header |
| `IntrinsicRendererRenderProcess.cpp` | enum value, type mapping, function mapping |
| `app/config/renderer_config.json` | add render step |

#### Step 1 — Header

```cpp
// IntrinsicRendererRenderPassMyPass.h
#pragma once
#include "IntrinsicRendererRenderProcess.h"

namespace Intrinsic { namespace Renderer { namespace RenderPass {

struct MyPass
{
  static void init();
  static void onReinitRendering();
  static void destroy();
  static void render(float p_DeltaT,
                     Components::CameraRef p_CameraRef);

  // Static resources (lifetime = renderer lifetime)
  static PipelineLayoutRef _pipelineLayoutRef;
  static PipelineRef       _pipelineRef;

  // Volatile resources (recreated on reinit)
  static ImageRef        _outputImageRef;
  static FramebufferRef  _framebufferRef;
  static RenderPassRef   _renderPassRef;
  static DrawCallRef     _drawCallRef;
};

}}} // namespace
```

#### Step 2 — Implementation skeleton

```cpp
// IntrinsicRendererRenderPassMyPass.cpp
#include "stdafx.h"
#include "IntrinsicRendererRenderPassMyPass.h"

using namespace RResources;
using namespace CComponents;

namespace Intrinsic { namespace Renderer { namespace RenderPass {

// Static member definitions
PipelineLayoutRef MyPass::_pipelineLayoutRef;
PipelineRef       MyPass::_pipelineRef;
ImageRef          MyPass::_outputImageRef;
FramebufferRef    MyPass::_framebufferRef;
RenderPassRef     MyPass::_renderPassRef;
DrawCallRef       MyPass::_drawCallRef;

// -----------------------------------------------------------------------
// init() — called once at startup.
// Create only resources that do NOT depend on the backbuffer resolution:
// pipelines, pipeline layouts, static images, static buffers.
// -----------------------------------------------------------------------
void MyPass::init()
{
  PipelineLayoutRefArray layoutsToCreate;
  PipelineRefArray       pipesToCreate;

  // --- Pipeline layout (reflects binding slots from the shader) ---
  _pipelineLayoutRef = PipelineLayoutManager::createPipelineLayout(_N(MyPass));
  PipelineLayoutManager::resetToDefault(_pipelineLayoutRef);
  GpuProgramManager::reflectPipelineLayout(
      8u,   // max descriptor set count
      { GpuProgramManager::getResourceByName(_N(my_pass.vert)),
        GpuProgramManager::getResourceByName(_N(my_pass.frag)) },
      _pipelineLayoutRef);
  layoutsToCreate.push_back(_pipelineLayoutRef);

  PipelineLayoutManager::createResources(layoutsToCreate);

  // --- Graphics pipeline ---
  _pipelineRef = PipelineManager::createPipeline(_N(MyPass));
  PipelineManager::resetToDefault(_pipelineRef);
  PipelineManager::_descVertexProgram(_pipelineRef) =
      GpuProgramManager::getResourceByName(_N(my_pass.vert));
  PipelineManager::_descFragmentProgram(_pipelineRef) =
      GpuProgramManager::getResourceByName(_N(my_pass.frag));
  PipelineManager::_descPipelineLayout(_pipelineRef) = _pipelineLayoutRef;
  PipelineManager::_descDepthStencilState(_pipelineRef) =
      DepthStencilStates::kDefaultNoWrite;   // read depth, don't write
  PipelineManager::_descRasterizationState(_pipelineRef) =
      RasterizationStates::kDefault;
  // _descRenderPass and _descVertexLayout are filled in onReinitRendering
  // because the render pass object references a resolution-dependent format
  pipesToCreate.push_back(_pipelineRef);

  PipelineManager::createResources(pipesToCreate);
}

// -----------------------------------------------------------------------
// onReinitRendering() — called at startup (after init) AND after every
// window resize / renderer config reload.
// Create/recreate everything that depends on backbuffer dimensions.
// -----------------------------------------------------------------------
void MyPass::onReinitRendering()
{
  // -- Destroy old volatile resources --
  {
    ImageRefArray      imgDel;
    FramebufferRefArray fbDel;
    RenderPassRefArray  rpDel;
    DrawCallRefArray    dcDel;

    if (_outputImageRef.isValid()) imgDel.push_back(_outputImageRef);
    if (_framebufferRef.isValid()) fbDel.push_back(_framebufferRef);
    if (_renderPassRef.isValid())  rpDel.push_back(_renderPassRef);
    if (_drawCallRef.isValid())    dcDel.push_back(_drawCallRef);

    ImageManager::destroyImagesAndResources(imgDel);
    FramebufferManager::destroyFramebuffersAndResources(fbDel);
    RenderPassManager::destroyRenderPassesAndResources(rpDel);
    DrawCallManager::destroyDrawCallsAndResources(dcDel);
  }

  const glm::uvec2 res = RenderSystem::_backbufferDimensions;

  // -- Output image --
  _outputImageRef = ImageManager::createImage(_N(MyPassOutput));
  {
    ImageManager::resetToDefault(_outputImageRef);
    ImageManager::addResourceFlags(
        _outputImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_outputImageRef) =
        MemoryPoolType::kResolutionDependentImages;
    ImageManager::_descDimensions(_outputImageRef) = glm::uvec3(res, 1u);
    ImageManager::_descImageFormat(_outputImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageFlags(_outputImageRef) =
        ImageFlags::kUsageAttachment | ImageFlags::kUsageSampled;
  }
  ImageManager::createResources({ _outputImageRef });

  // -- Render pass (describes load/store ops and attachment formats) --
  _renderPassRef = RenderPassManager::createRenderPass(_N(MyPass));
  {
    RenderPassManager::resetToDefault(_renderPassRef);
    AttachmentDescription colorAtt = {
        (uint8_t)Format::kR16G16B16A16Float,
        AttachmentFlags::kClearOnLoad };
    RenderPassManager::_descAttachments(_renderPassRef).push_back(colorAtt);
    // Add depth attachment read-only if you need it:
    // AttachmentDescription depthAtt = {
    //     (uint8_t)RenderSystem::_depthStencilFormatToUse,
    //     AttachmentFlags::kLoadFromPreviousPass };
    // RenderPassManager::_descAttachments(_renderPassRef).push_back(depthAtt);
  }
  RenderPassManager::createResources({ _renderPassRef });

  // -- Framebuffer --
  _framebufferRef = FramebufferManager::createFramebuffer(_N(MyPass));
  {
    FramebufferManager::resetToDefault(_framebufferRef);
    FramebufferManager::addResourceFlags(
        _framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    FramebufferManager::_descDimensions(_framebufferRef) = res;
    FramebufferManager::_descRenderPass(_framebufferRef) = _renderPassRef;
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(AttachmentInfo(_outputImageRef));
    // .push_back(AttachmentInfo(gbufferDepthRef));  // if depth read
  }
  FramebufferManager::createResources({ _framebufferRef });

  // -- Wire render pass into the pipeline --
  PipelineManager::_descRenderPass(_pipelineRef) = _renderPassRef;
  PipelineManager::createResources({ _pipelineRef });

  // -- Draw call (fullscreen triangle — no vertex buffer needed) --
  _drawCallRef = DrawCallManager::createDrawCall(_N(MyPass));
  {
    DrawCallManager::resetToDefault(_drawCallRef);
    DrawCallManager::addResourceFlags(
        _drawCallRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    DrawCallManager::_descPipeline(_drawCallRef)       = _pipelineRef;
    DrawCallManager::_descVertexCount(_drawCallRef)    = 3u;  // fullscreen tri
    DrawCallManager::_descFramebuffer(_drawCallRef)    = _framebufferRef;
    DrawCallManager::_descRenderPass(_drawCallRef)     = _renderPassRef;

    // Bind per-frame uniform
    DrawCallManager::bindBuffer(
        _drawCallRef, _N(PerInstance),
        GpuProgramType::kFragment,
        UniformManager::_perInstanceUniformBuffer,
        UboType::kPerInstanceFragment,
        sizeof(MyPerInstanceData));

    // Bind input textures from earlier passes
    DrawCallManager::bindImage(
        _drawCallRef, _N(sceneTex),
        GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(Scene)),   // output of Clustering pass
        Samplers::kLinearClamp);

    DrawCallManager::bindImage(
        _drawCallRef, _N(depthTex),
        GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferDepth)),
        Samplers::kNearestClamp);
  }
  DrawCallManager::createResources({ _drawCallRef });

  // -- Initial layout transition (needed before first render call) --
  VkCommandBuffer tmpCmd = RenderSystem::beginTemporaryCommandBuffer();
  ImageManager::insertImageMemoryBarrier(
      _outputImageRef,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
  RenderSystem::flushTemporaryCommandBuffer();
}

// -----------------------------------------------------------------------
// destroy() — cleanup at shutdown
// -----------------------------------------------------------------------
void MyPass::destroy()
{
  PipelineManager::destroyPipelinesAndResources({ _pipelineRef });
  PipelineLayoutManager::destroyPipelineLayoutsAndResources(
      { _pipelineLayoutRef });
}

// -----------------------------------------------------------------------
// render() — called every frame
// -----------------------------------------------------------------------
void MyPass::render(float p_DeltaT, Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "My Pass");
  _INTR_PROFILE_GPU("My Pass");

  VkCommandBuffer cmd = RenderSystem::getPrimaryCommandBuffer();

  // Update per-instance data
  MyPerInstanceData perInstance;
  perInstance.time     = TaskManager::_totalTimePassed;
  perInstance.invRes   = glm::vec2(1.f) / glm::vec2(RenderSystem::_backbufferDimensions);

  DrawCallManager::allocateAndUpdateUniformMemory(
      { _drawCallRef }, nullptr, 0u, &perInstance, sizeof(perInstance));

  // Transition output to writable
  ImageManager::insertImageMemoryBarrier(
      _outputImageRef,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

  // Render
  VkClearValue clear;
  clear.color = {{0.f, 0.f, 0.f, 1.f}};
  RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef,
                                VK_SUBPASS_CONTENTS_INLINE, 1u, &clear);
  RenderSystem::dispatchDrawCall(_drawCallRef, cmd);
  RenderSystem::endRenderPass(_renderPassRef);

  // Transition to readable
  ImageManager::insertImageMemoryBarrier(
      _outputImageRef,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

}}} // namespace
```

#### Step 3 — Register in RenderProcess

In `IntrinsicRendererRenderProcess.cpp`:

```cpp
// 1. Add to RenderStepType::Enum
namespace RenderStepType {
enum Enum {
  // ... existing values ...
  kRenderPassMyPass        // ADD THIS
};
}

// 2. Add to _renderStepTypeMapping
_INTR_HASH_MAP(Name, RenderStepType::Enum) _renderStepTypeMapping = {
  // ... existing entries ...
  { "RenderPassMyPass", RenderStepType::kRenderPassMyPass },  // ADD THIS
};

// 3. Add to _renderStepFunctionMapping
_INTR_HASH_MAP(RenderStepType::Enum, RenderPassInterface) _renderStepFunctionMapping = {
  // ... existing entries ...
  { RenderStepType::kRenderPassMyPass,
    { RenderPass::MyPass::render, RenderPass::MyPass::onReinitRendering } },
};
```

Also add `init()` and `destroy()` calls in `RenderProcess::init()` and
`RenderProcess::destroy()`.

In `IntrinsicRendererRenderProcess.h` add:
```cpp
#include "IntrinsicRendererRenderPassMyPass.h"
```

#### Step 4 — renderer_config.json

```json
"renderSteps": [
  ...,
  { "type": "RenderPassMyPass" },
  ...
]
```

Place it **after** any passes that produce images you bind as inputs (e.g. after
`RenderPassClustering` if you read `Scene`).

#### Step 5 — Shaders

```glsl
// app/assets/shaders/my_pass.vert.glsl
#version 450
#include "lib_buffers.glsl"

// Fullscreen triangle — no input vertices needed.
// The vertex shader generates clip-space positions from gl_VertexIndex.
void main() {
  vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
  gl_Position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
}
```

```glsl
// app/assets/shaders/my_pass.frag.glsl
#version 450
#include "lib_buffers.glsl"

layout(set = 0, binding = 0) uniform PerInstance {
  vec2 invRes;
  float time;
} uPerInstance;

layout(set = 1, binding = 0) uniform sampler2D sceneTex;
layout(set = 1, binding = 1) uniform sampler2D depthTex;

layout(location = 0) out vec4 outColor;

void main() {
  vec2 uv    = gl_FragCoord.xy * uPerInstance.invRes;
  vec4 scene = texture(sceneTex, uv);
  outColor   = scene;
}
```

Create the GPU program JSON files in `app/managers/gpu_programs/`:

```json
// my_pass_vert.gpu_program.json
{ "name": "my_pass.vert", "shaderFileName": "my_pass.vert.glsl",
  "entryPoint": "main", "programType": 0 }

// my_pass_frag.gpu_program.json
{ "name": "my_pass.frag", "shaderFileName": "my_pass.frag.glsl",
  "entryPoint": "main", "programType": 1 }
```

---

### 7.5 Image memory barriers — rules

```
oldLayout = UNDEFINED is always valid as srcLayout.
The driver discards content and transitions to newLayout.
Use it at the start of every frame for every attachment you're about to write.
```

| Transition | srcStage | dstStage |
|------------|----------|----------|
| init / first use | `TOP_OF_PIPE` | target stage |
| after compute write | `COMPUTE_SHADER_BIT` | consumer stage |
| after color attachment write | `COLOR_ATTACHMENT_OUTPUT_BIT` | consumer stage |
| after depth write | `LATE_FRAGMENT_TESTS_BIT` | consumer stage |
| after transfer | `TRANSFER_BIT` | consumer stage |

`TOP_OF_PIPE` and `BOTTOM_OF_PIPE` must always have access masks `= 0`
(the helper zeroes them automatically since commit `fd363ee7`).

---

### 7.6 Accessing a render target from another pass

```cpp
// In init() — the image was declared in renderer_config.json
// or created by an earlier pass's onReinitRendering()
_gbufferDepthRef = ImageManager::getResourceByName(_N(GBufferDepth));
_sceneRef        = ImageManager::getResourceByName(_N(Scene));
```

The `Name` (`_N(...)`) must match the `"name"` field in the image's
`ImageManager::createImage()` call or its `renderer_config.json` entry.

---

## 8. Example: Forward Transparent Pass

A forward pass renders geometry that cannot be deferred (transparent objects,
alpha-blended surfaces). It reads the filled G-Buffer depth for depth testing and
writes color additively over the existing lighting result.

### What changes versus the generic pass above

- Uses the **existing G-Buffer depth** as a read-only depth attachment.
- Writes over the **existing `Scene` image** (additive or alpha blend) rather
  than a new image.
- Iterates visible draw calls tagged with a **new material pass** `ForwardTransparent`.
- Needs a **blend state** set to additive or source-alpha.
- **No clear** — loads the previous content of the `Scene` attachment.

### New material pass

Add to `app/config/material_pass_config.json`:

```json
{
  "name": "ForwardTransparent",
  "materialPassId": 16
}
```

Materials that should render in this pass set `materialPassMask |= (1 << 4)` (bit 4).
In the material JSON:

```json
{ "materialPassMask": 17 }   // 0x01 (GBuffer) | 0x10 (ForwardTransparent)
```

Or for transparent-only materials that skip the G-Buffer:

```json
{ "materialPassMask": 16 }
```

### Header

```cpp
// IntrinsicRendererRenderPassForwardTransparent.h
struct ForwardTransparent
{
  static void init();
  static void onReinitRendering();
  static void destroy();
  static void render(float p_DeltaT, Components::CameraRef p_CameraRef);

  static PipelineLayoutRef _pipelineLayoutRef;  // reflects forward.vert + forward.frag
  // No volatile image — we write into the existing Scene image
  static RenderPassRef  _renderPassRef;
  static FramebufferRef _framebufferRef;
};
```

### init() — create pipeline with alpha blend

```cpp
void ForwardTransparent::init()
{
  // Pipeline layout
  _pipelineLayoutRef = PipelineLayoutManager::createPipelineLayout(
      _N(ForwardTransparent));
  PipelineLayoutManager::resetToDefault(_pipelineLayoutRef);
  GpuProgramManager::reflectPipelineLayout(
      8u,
      { GpuProgramManager::getResourceByName(_N(forward_transparent.vert)),
        GpuProgramManager::getResourceByName(_N(forward_transparent.frag)) },
      _pipelineLayoutRef);
  PipelineLayoutManager::createResources({ _pipelineLayoutRef });

  // Pipeline — no depth write, back-to-front expected from caller,
  // source-alpha blending
  PipelineRef pip = PipelineManager::createPipeline(_N(ForwardTransparent));
  PipelineManager::resetToDefault(pip);
  PipelineManager::_descVertexProgram(pip) =
      GpuProgramManager::getResourceByName(_N(forward_transparent.vert));
  PipelineManager::_descFragmentProgram(pip) =
      GpuProgramManager::getResourceByName(_N(forward_transparent.frag));
  PipelineManager::_descPipelineLayout(pip)      = _pipelineLayoutRef;
  PipelineManager::_descDepthStencilState(pip)   = DepthStencilStates::kDefaultNoWrite;
  PipelineManager::_descBlendStates(pip).clear();
  PipelineManager::_descBlendStates(pip).push_back(BlendStates::kAlphaBlend);
  // _descRenderPass set in onReinitRendering after the render pass is created
  PipelineManager::createResources({ pip });
}
```

### onReinitRendering() — share Scene + GBufferDepth

```cpp
void ForwardTransparent::onReinitRendering()
{
  // Cleanup
  if (_renderPassRef.isValid())
    RenderPassManager::destroyRenderPassesAndResources({ _renderPassRef });
  if (_framebufferRef.isValid())
    FramebufferManager::destroyFramebuffersAndResources({ _framebufferRef });

  ImageRef sceneRef  = ImageManager::getResourceByName(_N(Scene));
  ImageRef depthRef  = ImageManager::getResourceByName(_N(GBufferDepth));

  // Render pass: LOAD existing color (no clear), depth READ-ONLY
  _renderPassRef = RenderPassManager::createRenderPass(_N(ForwardTransparent));
  {
    RenderPassManager::resetToDefault(_renderPassRef);
    // Color attachment — load existing content (no clear flag)
    AttachmentDescription colorAtt = {
        (uint8_t)Format::kR16G16B16A16Float,
        AttachmentFlags::kLoadFromPreviousPass };
    // Depth attachment — read-only (test but don't write)
    AttachmentDescription depthAtt = {
        (uint8_t)RenderSystem::_depthStencilFormatToUse,
        AttachmentFlags::kLoadFromPreviousPass };
    RenderPassManager::_descAttachments(_renderPassRef).push_back(colorAtt);
    RenderPassManager::_descAttachments(_renderPassRef).push_back(depthAtt);
  }
  RenderPassManager::createResources({ _renderPassRef });

  // Framebuffer — attach the existing Scene and GBufferDepth images
  _framebufferRef = FramebufferManager::createFramebuffer(_N(ForwardTransparent));
  {
    FramebufferManager::resetToDefault(_framebufferRef);
    FramebufferManager::addResourceFlags(
        _framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    FramebufferManager::_descDimensions(_framebufferRef) =
        RenderSystem::_backbufferDimensions;
    FramebufferManager::_descRenderPass(_framebufferRef)  = _renderPassRef;
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(AttachmentInfo(sceneRef));
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(AttachmentInfo(depthRef));
  }
  FramebufferManager::createResources({ _framebufferRef });

  // Wire the render pass into the pipeline
  PipelineManager::_descRenderPass(_pipelineRef) = _renderPassRef;
  PipelineManager::createResources({ _pipelineRef });
}
```

### render()

```cpp
void ForwardTransparent::render(float p_DeltaT,
                                Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Forward Transparent");
  _INTR_PROFILE_GPU("Forward Transparent");

  ImageRef sceneRef = ImageManager::getResourceByName(_N(Scene));
  ImageRef depthRef = ImageManager::getResourceByName(_N(GBufferDepth));

  // Collect draw calls for the ForwardTransparent material pass
  static DrawCallRefArray visibleDCs;
  visibleDCs.clear();
  RenderProcess::Default::getVisibleDrawCalls(
      p_CameraRef, 0u,
      MaterialManager::getMaterialPassId(_N(ForwardTransparent)))
      .copy(visibleDCs);

  if (visibleDCs.empty()) return;   // nothing to draw this frame

  // Sort back-to-front for correct alpha blending
  DrawCallManager::sortDrawCallsBackToFront(visibleDCs);

  // Upload per-instance data for each visible mesh
  Components::MeshManager::updatePerInstanceData(p_CameraRef, 0u);
  Components::MeshManager::updateUniformData(visibleDCs);

  // Scene was left in SHADER_READ_ONLY by Clustering; bring it back to COLOR_ATTACHMENT
  ImageManager::insertImageMemoryBarrier(
      sceneRef,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

  // Depth must be readable for depth test
  ImageManager::insertImageMemoryBarrier(
      depthRef,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

  // No clear values — we LOAD both attachments
  RenderSystem::beginRenderPass(
      _renderPassRef, _framebufferRef,
      VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

  DrawCallDispatcher::queueDrawCalls(visibleDCs, _renderPassRef, _framebufferRef);

  RenderSystem::endRenderPass(_renderPassRef);

  // Transition Scene back to readable for post-processing
  ImageManager::insertImageMemoryBarrier(
      sceneRef,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}
```

### Placement in renderer_config.json

```json
{ "type": "RenderPassClustering" },
{ "type": "RenderPassForwardTransparent" },   // AFTER Clustering, BEFORE Bloom
{ "type": "RenderPassBloom" },
```

---

## 9. Example: Ray Tracing Pass

Ray tracing requires `VK_KHR_ray_tracing_pipeline` (and its dependencies). The
engine as shipped does not include it — the steps below describe how you would
add it from scratch. This is a significant addition, not a one-afternoon task.

### New Vulkan extensions required

These must all be enabled at device creation time in `IntrinsicRendererRenderSystem.cpp`:

```cpp
// Required chain (order matters — each depends on the previous)
"VK_KHR_deferred_host_operations"
"VK_KHR_buffer_device_address"       // also needs feature flag
"VK_KHR_acceleration_structure"
"VK_KHR_ray_tracing_pipeline"
"VK_EXT_descriptor_indexing"         // likely already present
```

In `RenderSystem::init()` where extensions are enumerated and enabled:

```cpp
// Feature chain — must be linked via pNext
VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeatures = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
accelFeatures.accelerationStructure = VK_TRUE;
accelFeatures.pNext = &bufferDeviceAddressFeatures;

VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtFeatures = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
rtFeatures.rayTracingPipeline = VK_TRUE;
rtFeatures.pNext = &accelFeatures;

// Chain into VkDeviceCreateInfo::pNext
deviceCreateInfo.pNext = &rtFeatures;
```

Also query `VkPhysicalDeviceRayTracingPipelinePropertiesKHR` to get
`shaderGroupHandleSize` and `shaderGroupBaseAlignment` (needed for the SBT).

Load the function pointers (they are extension functions, not in the Vulkan core):

```cpp
auto vkCreateAccelerationStructureKHR =
    (PFN_vkCreateAccelerationStructureKHR)
    vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR");
// ... vkDestroyAccelerationStructureKHR
// ... vkGetAccelerationStructureBuildSizesKHR
// ... vkCmdBuildAccelerationStructuresKHR
// ... vkGetAccelerationStructureDeviceAddressKHR
// ... vkCreateRayTracingPipelinesKHR
// ... vkGetRayTracingShaderGroupHandlesKHR
// ... vkCmdTraceRaysKHR
```

Store them as static members on `RenderSystem` for other passes to use.

### Pass structure overview

```
init()
 ├── createShadersAndPipeline()    – raygen, miss, closest-hit SPIR-V → pipeline
 └── createShaderBindingTable()    – SBT buffer with aligned handles

onReinitRendering()
 └── (re)create output image

buildBLAS()                        – called once per mesh (or on mesh change)
 └── one VkAccelerationStructureKHR per mesh (triangle geometry)

buildTLAS()                        – called every frame (or when scene changes)
 └── one VkAccelerationStructureKHR with N instances pointing to BLASes

render()
 ├── buildTLAS()                   – or update if only transforms changed
 ├── updateDescriptorSet()         – bind TLAS + output image + G-Buffer inputs
 ├── insertImageMemoryBarrier()    – output UNDEFINED → GENERAL
 ├── vkCmdTraceRaysKHR()           – dispatch rays
 └── insertImageMemoryBarrier()    – output GENERAL → SHADER_READ_ONLY_OPTIMAL
```

### Building a BLAS

A BLAS (Bottom-Level Acceleration Structure) represents the triangle geometry of
one mesh. Build it once and cache it in a `BufferRef` or a
`VkAccelerationStructureKHR` stored alongside the mesh data.

```cpp
void buildBLAS(MeshRef meshRef)
{
  // Get the vertex and index buffers for this mesh
  BufferRef vbRef = MeshManager::_vertexBuffer(meshRef);
  BufferRef ibRef = MeshManager::_indexBuffer(meshRef);

  // Describe the triangle geometry
  VkAccelerationStructureGeometryKHR geometry = {};
  geometry.sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
  geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
  geometry.flags        = VK_GEOMETRY_OPAQUE_BIT_KHR;

  auto& tris = geometry.geometry.triangles;
  tris.sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
  tris.vertexFormat  = VK_FORMAT_R16G16B16_SFLOAT;  // packed half-float positions
  tris.vertexData.deviceAddress =
      getBufferDeviceAddress(RenderSystem::_vkDevice, BufferManager::_vkBuffer(vbRef));
  tris.vertexStride  = sizeof(uint16_t) * 4;  // xyz + padding
  tris.maxVertex     = MeshManager::_vertexCount(meshRef) - 1;
  tris.indexType     = VK_INDEX_TYPE_UINT32;
  tris.indexData.deviceAddress =
      getBufferDeviceAddress(RenderSystem::_vkDevice, BufferManager::_vkBuffer(ibRef));

  uint32_t primitiveCount = MeshManager::_indexCount(meshRef) / 3;

  // Query required sizes
  VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
  buildInfo.sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
  buildInfo.type          = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
  buildInfo.flags         = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
  buildInfo.geometryCount = 1u;
  buildInfo.pGeometries   = &geometry;

  VkAccelerationStructureBuildSizesInfoKHR sizes = {
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
  vkGetAccelerationStructureBuildSizesKHR(
      RenderSystem::_vkDevice,
      VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
      &buildInfo, &primitiveCount, &sizes);

  // Allocate BLAS buffer and scratch buffer
  // (use BufferManager with STORAGE_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)
  VkBuffer blasBuffer   = allocateBuffer(sizes.accelerationStructureSize,
      VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
  VkBuffer scratchBuffer = allocateBuffer(sizes.buildScratchSize,
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

  // Create the AS object
  VkAccelerationStructureCreateInfoKHR createInfo = {};
  createInfo.sType  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
  createInfo.buffer = blasBuffer;
  createInfo.size   = sizes.accelerationStructureSize;
  createInfo.type   = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

  VkAccelerationStructureKHR blas;
  vkCreateAccelerationStructureKHR(RenderSystem::_vkDevice, &createInfo, nullptr, &blas);

  // Build on the GPU
  buildInfo.dstAccelerationStructure  = blas;
  buildInfo.scratchData.deviceAddress = getBufferDeviceAddress(RenderSystem::_vkDevice,
                                                               scratchBuffer);
  VkAccelerationStructureBuildRangeInfoKHR rangeInfo = { primitiveCount };
  const VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = &rangeInfo;

  VkCommandBuffer cmd = RenderSystem::beginTemporaryCommandBuffer();
  vkCmdBuildAccelerationStructuresKHR(cmd, 1u, &buildInfo, &pRangeInfo);
  RenderSystem::flushTemporaryCommandBuffer();

  // Store blas handle alongside mesh data (add a static map or extend MeshData)
  _blasMap[meshRef] = blas;
}
```

### Building a TLAS every frame

A TLAS (Top-Level Acceleration Structure) contains one instance per visible
object, each pointing to a BLAS and carrying a world transform.

```cpp
void buildTLAS(Components::CameraRef p_CameraRef)
{
  // Collect visible mesh instances
  const DrawCallRefArray& dcs =
      RenderProcess::Default::getVisibleDrawCalls(p_CameraRef, 0u,
          MaterialManager::getMaterialPassId(_N(GBufferDefault)));

  _INTR_ARRAY(VkAccelerationStructureInstanceKHR) instances;
  for (DrawCallRef dc : dcs)
  {
    MeshRef mesh    = DrawCallManager::_descMesh(dc);
    NodeRef node    = DrawCallManager::_descNode(dc);
    glm::mat4 world = NodeManager::getWorldTransform(node);

    VkAccelerationStructureInstanceKHR inst = {};
    // VkTransformMatrixKHR is a row-major 3x4 matrix
    memcpy(&inst.transform, glm::value_ptr(glm::transpose(world)), sizeof(inst.transform));
    inst.instanceCustomIndex                    = instances.size();  // gl_InstanceCustomIndexEXT
    inst.mask                                   = 0xFF;
    inst.instanceShaderBindingTableRecordOffset = 0u;  // offset into hit group table
    inst.flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    inst.accelerationStructureReference         =
        getAccelerationStructureAddress(RenderSystem::_vkDevice, _blasMap[mesh]);

    instances.push_back(inst);
  }

  // Upload instance data to a device-visible buffer, then build TLAS
  // (pattern identical to BLAS build — query sizes, allocate, vkCmdBuildAccelerationStructuresKHR)
  // For dynamic scenes use VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR
  // or UPDATE mode if only transforms changed.
}
```

### Creating the RT pipeline

```cpp
void createRTPipeline()
{
  // Load SPIR-V shader modules
  VkShaderModule raygenModule = loadSPIRV("rt_raygen.rgen.spv");
  VkShaderModule missModule   = loadSPIRV("rt_miss.rmiss.spv");
  VkShaderModule chitModule   = loadSPIRV("rt_chit.rchit.spv");

  VkPipelineShaderStageCreateInfo stages[3];
  stages[0] = makeStage(raygenModule, VK_SHADER_STAGE_RAYGEN_BIT_KHR,   "main");
  stages[1] = makeStage(missModule,   VK_SHADER_STAGE_MISS_BIT_KHR,     "main");
  stages[2] = makeStage(chitModule,   VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, "main");

  // Shader groups: one per logical entry point
  VkRayTracingShaderGroupCreateInfoKHR groups[3] = {};
  // Raygen group (general type)
  groups[0].sType              = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
  groups[0].type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
  groups[0].generalShader      = 0;  // index into stages[]
  groups[0].closestHitShader   = VK_SHADER_UNUSED_KHR;
  groups[0].anyHitShader       = VK_SHADER_UNUSED_KHR;
  groups[0].intersectionShader = VK_SHADER_UNUSED_KHR;
  // Miss group
  groups[1] = groups[0];
  groups[1].generalShader = 1;
  // Hit group (triangles type)
  groups[2].sType              = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
  groups[2].type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
  groups[2].generalShader      = VK_SHADER_UNUSED_KHR;
  groups[2].closestHitShader   = 2;  // index into stages[]
  groups[2].anyHitShader       = VK_SHADER_UNUSED_KHR;
  groups[2].intersectionShader = VK_SHADER_UNUSED_KHR;

  VkRayTracingPipelineCreateInfoKHR pipelineCI = {};
  pipelineCI.sType                        = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
  pipelineCI.stageCount                   = 3u;
  pipelineCI.pStages                      = stages;
  pipelineCI.groupCount                   = 3u;
  pipelineCI.pGroups                      = groups;
  pipelineCI.maxPipelineRayRecursionDepth = 1u;  // 1 = primary rays only (no reflections)
  pipelineCI.layout                       = _rtPipelineLayout;

  vkCreateRayTracingPipelinesKHR(RenderSystem::_vkDevice,
      VK_NULL_HANDLE, RenderSystem::_vkPipelineCache,
      1u, &pipelineCI, nullptr, &_rtPipeline);
}
```

### Creating the Shader Binding Table (SBT)

The SBT is a GPU buffer containing one aligned handle per shader group.

```cpp
void createSBT()
{
  // Query hardware handle size and alignment from device properties
  uint32_t handleSize      = _rtProperties.shaderGroupHandleSize;
  uint32_t handleAlignment = _rtProperties.shaderGroupHandleAlignment;
  uint32_t groupCount      = 3u;  // raygen + miss + hit

  uint32_t sbtStride = alignUp(handleSize, handleAlignment);
  uint32_t sbtSize   = groupCount * sbtStride;

  // Get raw handles from the pipeline
  std::vector<uint8_t> handles(groupCount * handleSize);
  vkGetRayTracingShaderGroupHandlesKHR(
      RenderSystem::_vkDevice, _rtPipeline,
      0u, groupCount, handles.size(), handles.data());

  // Allocate a host-visible, device-addressable buffer
  // Must have VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
  //           | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
  _sbtBuffer = allocateSBTBuffer(sbtSize);
  uint8_t* mapped = mapSBTBuffer(_sbtBuffer);

  // Copy each handle at the correct aligned offset
  for (uint32_t i = 0; i < groupCount; ++i)
    memcpy(mapped + i * sbtStride, handles.data() + i * handleSize, handleSize);

  unmapSBTBuffer(_sbtBuffer);

  // Store strided device address regions for dispatch
  VkDeviceAddress sbtBase = getBufferDeviceAddress(RenderSystem::_vkDevice, _sbtBuffer);
  _raygenRegion   = { sbtBase + 0 * sbtStride, sbtStride, sbtStride };
  _missRegion     = { sbtBase + 1 * sbtStride, sbtStride, sbtStride };
  _hitRegion      = { sbtBase + 2 * sbtStride, sbtStride, sbtStride };
  _callableRegion = {};  // no callable shaders
}
```

### render()

```cpp
void RayTracing::render(float p_DeltaT, Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Ray Tracing");
  _INTR_PROFILE_GPU("Ray Tracing");

  VkCommandBuffer cmd = RenderSystem::getPrimaryCommandBuffer();

  // Rebuild TLAS for this frame's visible geometry
  buildTLAS(p_CameraRef);

  // Transition output image to GENERAL (storage write)
  ImageManager::insertImageMemoryBarrier(
      _rtOutputImageRef,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_GENERAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

  // Bind pipeline and descriptor sets
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, _rtPipeline);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
      _rtPipelineLayout, 0u, 1u, &_rtDescriptorSet, 0u, nullptr);

  // Dispatch rays — one ray per pixel
  const glm::uvec2 res = RenderSystem::_backbufferDimensions;
  vkCmdTraceRaysKHR(cmd,
      &_raygenRegion, &_missRegion, &_hitRegion, &_callableRegion,
      res.x, res.y, 1u);

  // Transition result to readable so it can be composited
  ImageManager::insertImageMemoryBarrier(
      _rtOutputImageRef,
      VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}
```

### Minimal raygen shader

```glsl
// app/assets/shaders/rt_raygen.rgen.glsl
#version 460
#extension GL_EXT_ray_tracing : require
#include "lib_buffers.glsl"   // PerFrameData with invViewProjMatrix, camPos

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 1, rgba16f) uniform image2D outputImage;

layout(location = 0) rayPayloadEXT vec4 payload;

void main()
{
  // Reconstruct ray from screen pixel
  vec2 uv = (vec2(gl_LaunchIDEXT.xy) + 0.5) / vec2(gl_LaunchSizeEXT.xy);
  vec2 ndc = uv * 2.0 - 1.0;

  vec4 rayOriginH    = uPerFrame.invViewProjMatrix * vec4(ndc, 0.0, 1.0);
  vec4 rayEndH       = uPerFrame.invViewProjMatrix * vec4(ndc, 1.0, 1.0);
  vec3 rayOrigin     = rayOriginH.xyz / rayOriginH.w;
  vec3 rayDir        = normalize(rayEndH.xyz / rayEndH.w - rayOrigin);

  payload = vec4(0.0);

  traceRayEXT(
      topLevelAS,
      gl_RayFlagsOpaqueEXT,   // ray flags
      0xFF,                   // cull mask
      0u,                     // sbtRecordOffset
      0u,                     // sbtRecordStride
      0u,                     // miss index
      rayOrigin,
      0.001,                  // tMin
      rayDir,
      10000.0,                // tMax
      0                       // payload location
  );

  imageStore(outputImage, ivec2(gl_LaunchIDEXT.xy), payload);
}
```

### Placement in renderer_config.json

```json
{ "type": "RenderPassClustering" },
{ "type": "RenderPassRayTracing" },          // after deferred lighting
{ "type": "GenericFullscreen",               // composite RT result over scene
    "name": "RTComposite",
    "fragmentGpuProgram": "rt_composite.frag",
    "inputs": [
      ["Image", "Scene",        "sceneTex",    "Fragment", "LinearClamp"],
      ["Image", "RTOutput",     "rtTex",       "Fragment", "LinearClamp"],
      ["Image", "GBufferDepth", "depthTex",    "Fragment", "NearestClamp"]
    ],
    "outputs": [ ["Scene"] ]
},
{ "type": "RenderPassBloom" },
```

### Practical notes on adding RT to this engine

- **Buffer device addresses**: every BLAS and TLAS scratch buffer needs
  `VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT`. The current `BufferManager` would
  need a new `BufferType` or usage flag for this. The simplest approach is to add
  a `kRaytracingBuffer` enum value and handle it in the buffer creation code.
- **BLAS lifecycle**: BLASes should be rebuilt when a mesh is loaded/unloaded.
  Hook into `MeshManager::createResources` / `destroyResources`.
- **TLAS update vs. rebuild**: if only transforms change, use
  `VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR` and rebuild with
  `mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR`. This is ~10× faster
  than a full rebuild.
- **Descriptor layout**: the TLAS is bound as
  `VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR`. The output image is
  `VK_DESCRIPTOR_TYPE_STORAGE_IMAGE`. These cannot use the existing
  `GpuProgramManager::reflectPipelineLayout` (which does not know about RT
  descriptor types) — you'll need to build the descriptor set layout manually
  with `vkCreateDescriptorSetLayout`.
- **Shadow/AO**: if you only want RT for shadows or ambient occlusion rather than
  full path tracing, `maxPipelineRayRecursionDepth = 1` and a single closest-hit
  shader that writes 0.0/1.0 (occluded/visible) is sufficient — much cheaper than
  full GI.

---

## 10. Asset Pipeline

### Mesh import

1. Export model from your DCC tool as FBX
2. Place in the assets source folder
3. In IntrinsicEd: **Asset Management → Import FBX**
4. The importer generates:
   - `app/managers/meshes/<name>.mesh.json`
   - Cooked binary mesh data (positions, normals, UVs, tangents)
   - PhysX collision mesh if requested

The `.mesh.json` file records sub-mesh count, vertex count, and material slot
names. The material slot names must match material files of the same name in
`app/managers/materials/`.

### Texture import

Textures must be converted to DDS before the engine can load them:

- **BC1** — RGB, no alpha, smallest (for albedo without transparency)
- **BC3** — RGBA (for albedo with alpha / cutout)
- **BC5** — RG (for normal maps — store X and Y, reconstruct Z in shader)
- **BC7** — high-quality RGBA (for important albedos, UI)

Use Microsoft `texconv.exe`:
```
texconv -f BC7_UNORM -o app/assets/textures/ my_texture.png
```

Then create `app/managers/images/my_texture.image.json` pointing to the DDS file.

### Procedurally generated textures

Some textures are generated at runtime by compute shaders
(`DynamicTextureGeneration` pass). They are defined in `RenderSystem.cpp`
via `addDynamicGeneratedTexture()` and reference a compute shader. Generated
textures are registered in the bindless array automatically.

### Physics mesh cooking

PhysX requires a pre-cooked convex hull for `shapeType = 3`. The cooking happens
in `IntrinsicAssetManagement` during FBX import. If you update a mesh and need
to re-cook: re-import through the editor or call
`AssetManagement::cookPhysicsMesh(meshRef)` in code.

---

## 11. Lua Scripting

### Script component setup

1. Create `app/scripts/my_script.lua`
2. Create `app/managers/scripts/my_script.script.json`:

```json
{ "name": "my_script", "scriptFileName": "scripts/my_script.lua" }
```

3. Add a Script component to your entity:

```json
{ "type": "Script", "properties": { "scriptName": "my_script" } }
```

### Script lifecycle

```lua
-- Called once when the entity is spawned
function onCreate(entityRef)
  -- store entity ref for later use
  myEntity = entityRef
end

-- Called every frame
function tick(entityRef, deltaT)
  local node = Node.getComponent(entityRef)
  local pos  = Node.getPosition(node)
  Node.setPosition(node, pos + glm.vec3(0, deltaT, 0))
  Node.rebuildTransforms()
end

-- Called when the entity is destroyed
function onDestroy(entityRef)
end
```

### Exposed engine API

**Entity:**
```lua
local ref = Entity.getEntityByName("MyObject")
local name = Entity.getName(ref)
```

**Node:**
```lua
local node = Node.getComponent(entityRef)
Node.getPosition(node)              -- returns glm.vec3
Node.setPosition(node, vec3)
Node.getOrientation(node)           -- returns glm.quat
Node.setOrientation(node, quat)
Node.getSize(node)
Node.setSize(node, vec3)
Node.rebuildTransforms()            -- required after bulk changes
```

**Camera:**
```lua
local cam = Camera.getComponent(entityRef)
Camera.setFov(cam, 75.0)
```

**Input:**
```lua
if Input.isKeyPressed(Key.W) then ... end
if Input.isMouseButtonPressed(MouseButton.Left) then ... end
local dx, dy = Input.getMouseDelta()
```

**GLM math in Lua:**
```lua
local v = glm.vec3(1, 0, 0)
local q = glm.quat(glm.vec3(0, math.pi, 0))   -- from euler angles
local m = glm.mat4(1.0)
```

---

## 12. Procedural Systems

### Marching cubes terrain

`RenderPassMarchingCubes` generates a triangle mesh from a 3D voxel grid each
frame using a compute shader. The voxel data comes from
`DynamicGeometryGeneration`, which writes density values via a noise function.

Key parameters (in `DynamicGeometryGeneration.cpp`):
```cpp
// Grid resolution (must match shader defines)
const uint32_t GRID_SIZE_X = 64;
const uint32_t GRID_SIZE_Y = 64;
const uint32_t GRID_SIZE_Z = 64;
```

To change the terrain generator: edit `dynamic_geometry_generation.comp.glsl`.
The shader writes into a `_VoxelBuffer` SSBO. Marching cubes then reads this
buffer and writes triangles into a vertex buffer consumed by the draw pass.

### Pseudo-instancing

`PseudoInstancing` handles large numbers of the same mesh (e.g. grass, trees)
by duplicating mesh vertices in a single oversized vertex buffer, avoiding
per-instance draw call overhead. Instances are placed by `populateMeshes()` which
reads voxel heightmap data.

To register a mesh for pseudo-instancing:
```cpp
PseudoInstancing::addPseudoInstancingMesh(
    Name("grass_blade"),
    64,   // grid X
    64,   // grid Z
    0.8f  // placement probability
);
```

`generateInstances()` must be called once after the vertex buffer is created to
bake the transformed positions into GPU memory. This is a CPU-side operation.

---

## 13. Physics

The engine uses **NVIDIA PhysX 3.x**. Physics runs as part of the task system
and updates node transforms after the simulation step.

### Setting up a rigid body

```json
[
  {
    "name": "PhysicsBox",
    "offsetToParent": -1,
    "propertyEntries": [
      { "type": "Node",      "properties": { "localPos": [0, 10, 0], "localSize": [1, 1, 1] } },
      { "type": "Mesh",      "properties": { "meshName": "cube" } },
      { "type": "RigidBody", "properties": { "rigidBodyType": 1, "shapeType": 1, "mass": 1.0 } }
    ]
  }
]
```

### Runtime physics API

```cpp
RigidBodyRef rb = RigidBodyManager::getComponentForEntity(entityRef);

// Apply forces
RigidBodyManager::applyForce(rb, glm::vec3(0, 500, 0));
RigidBodyManager::applyImpulse(rb, glm::vec3(0, 10, 0));

// Change kinematic target
RigidBodyManager::setKinematicTarget(rb, worldTransform);

// Query velocity
glm::vec3 vel = RigidBodyManager::getLinearVelocity(rb);
```

### Character controller

The `CharacterController` component wraps PhysX's `PxController` for
first/third-person movement. It handles step-climbing and sliding automatically.

```cpp
CharacterControllerRef cc = CharacterControllerManager::getComponentForEntity(entityRef);
CharacterControllerManager::move(cc, displacement, deltaT);
bool grounded = CharacterControllerManager::isGrounded(cc);
```

---

## 14. Debugging & Profiling

### Vulkan validation layers

Enable by running with the Vulkan SDK validation layers active. On Windows:

```
set VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
Intrinsic.exe
```

Validation errors print to stderr and to the engine log. The most common
categories encountered in this codebase:

| VUID | Meaning |
|------|---------|
| `VkWriteDescriptorSet-descriptorType-00339` | Bound image lacks `STORAGE_BIT` |
| `VkDescriptorImageInfo-imageLayout-00344` | Image in wrong layout for descriptor |
| Access mask violations | Non-zero masks with `TOP_OF_PIPE`/`BOTTOM_OF_PIPE` |

### Microprofile (CPU + GPU profiling)

Available in non-final builds on Windows. Open the profile UI:

```
http://localhost:1338
```

Add markers in C++ code:
```cpp
MICROPROFILE_SCOPEI("Renderer", "MyPass", 0xFF0000);
```

GPU timing markers:
```cpp
// Uses VK_EXT_debug_marker — only available with the validation layers active
vkCmdDebugMarkerBeginEXT(cmdBuf, &markerInfo);
// ... commands ...
vkCmdDebugMarkerEndEXT(cmdBuf);
```

### Engine log macros

```cpp
_INTR_LOG_INFO("Loaded %d meshes", count);
_INTR_LOG_WARNING("Texture not found: %s", name.getString().c_str());
_INTR_LOG_ERROR("Fatal: %s", msg);
```

### Debug render pass

The `Debug` render pass can overlay wireframe AABBs, frustums, and physics
shapes. Toggle categories at runtime:

```cpp
// Toggle AABB visualization
Debug::Settings::showAABBs = !Debug::Settings::showAABBs;
```

### Common crash sites

- **Stale Ref after destroy**: a `Ref` becomes invalid once its component or
  entity is destroyed. Always check `.isValid()` before use.
- **`rebuildTreeAndUpdateTransforms()` not called**: transform changes do not
  propagate until this is called. Forgetting it leaves world matrices stale.
- **Descriptor set update on in-flight image**: updating a descriptor while the
  GPU is reading from it is UB. Either double-buffer or sync with a fence.

---

## 15. Common Recipes

### Spawn an entity at runtime

```cpp
// Create entity
Entity::EntityRef entity = Entity::EntityManager::createEntity(Name("Explosion"));

// Add node component
NodeRef node = NodeManager::createComponent(entity);
NodeManager::setPosition(node, spawnPosition);

// Add mesh component
MeshRef mesh = MeshManager::createComponent(entity);
MeshManager::setMeshName(mesh, Name("explosion_sphere"));
MeshManager::setMaterialName(mesh, Name("explosion_material"));

// Register mesh in the renderer
MeshManager::createResources(mesh);

// Update transforms
NodeManager::rebuildTreeAndUpdateTransforms();
```

### Destroy an entity and its components

```cpp
NodeRef node = NodeManager::getComponentForEntity(entity);
MeshRef mesh = MeshManager::getComponentForEntity(entity);

if (mesh.isValid()) {
    MeshManager::destroyResources(mesh);
    MeshManager::destroyComponent(mesh);
}
NodeManager::destroyComponent(node);
Entity::EntityManager::destroyEntity(entity);
NodeManager::rebuildTreeAndUpdateTransforms();
```

### Move the active camera

```cpp
NodeRef camNode = NodeManager::getComponentForEntity(
    CameraManager::getEntity(World::getActiveCamera()));

NodeManager::setPosition(camNode, newPos);
NodeManager::setOrientation(camNode, newOrientation);
NodeManager::rebuildTreeAndUpdateTransforms();
```

### Add a new compute pass that writes to a texture

```cpp
// In init():
GpuProgramRef computeProg = GpuProgramManager::getResourceByName(Name("my_compute"));

ImageDesc imgDesc;
imgDesc.width = 512;
imgDesc.height = 512;
imgDesc.format = VK_FORMAT_R8G8B8A8_UNORM;
imgDesc.imageUsageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
_myOutputImage = ImageManager::createImage(imgDesc);

// Transition to GENERAL so compute can write
// (use a one-time command buffer — see RenderProcess.cpp for the pattern)

// In render():
ImageManager::insertImageMemoryBarrier(
    _myOutputImage,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_GENERAL,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

// dispatch...

ImageManager::insertImageMemoryBarrier(
    _myOutputImage,
    VK_IMAGE_LAYOUT_GENERAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
```

### Read back a GPU buffer to CPU

```cpp
// Buffer must be created with HOST_VISIBLE | HOST_COHERENT
void* ptr = BufferManager::mapMemory(bufferRef);
memcpy(cpuData, ptr, dataSize);
BufferManager::unmapMemory(bufferRef);
```

### Save / restore a world region

```cpp
// Save a subtree starting from node
_INTR_ARRAY(Node) snapshot;
World::cloneNodeTree(rootNode, snapshot);

// Restore (destroy current, re-create from snapshot)
World::destroyNodeTree(rootNode);
World::restoreNodeTree(snapshot);
```

---

## Appendix: Pool Sizes

If you hit assertion failures about running out of slots, increase these
constants in `IntrinsicCorePrerequisites.h` and rebuild:

| Constant | Default | Component |
|----------|---------|-----------|
| `_INTR_MAX_NODE_COUNT` | 10240 | Node |
| `_INTR_MAX_MESH_COUNT` | 10240 | Mesh |
| `_INTR_MAX_CAMERA_COUNT` | 1024 | Camera |
| `_INTR_MAX_LIGHT_COUNT` | 1024 | Light |
| `_INTR_MAX_RIGID_BODY_COUNT` | 1024 | RigidBody |
| `_INTR_MAX_SCRIPT_COUNT` | 1024 | Script |
| `_INTR_MAX_DECAL_COUNT` | 1024 | Decal |
| `_INTR_MAX_DRAW_CALL_COUNT` | 4096 | DrawCall |
| `_INTR_MAX_PIPELINE_COUNT` | 512 | Pipeline |
| `_INTR_MAX_IMAGE_COUNT` | 4096 | Image |
| `_INTR_MAX_BUFFER_COUNT` | 2048 | Buffer |
