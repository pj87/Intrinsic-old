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
8. [Asset Pipeline](#8-asset-pipeline)
9. [Lua Scripting](#9-lua-scripting)
10. [Procedural Systems](#10-procedural-systems)
11. [Physics](#11-physics)
12. [Debugging & Profiling](#12-debugging--profiling)
13. [Common Recipes](#13-common-recipes)

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

### The render pipeline

The frame is orchestrated by `RenderProcess`, which executes passes in order.
The active pass list and their configuration live in `renderer_config.json`.

Default pass order (approximately):

```
GeometryGeneration     → fills G-Buffer (depth, albedo, normal, PBR)
DynamicGeometry        → procedural geometry into G-Buffer
DynamicTexture         → procedural texture generation (compute)
Shadow                 → shadow map depth renders
VolumetricLighting     → ESM generation + scatter accumulation
Clustering             → light culling + lighting pass (reads G-Buffer)
Bloom                  → lum downsample → blur → composite
PerPixelPicking        → off-screen pick buffer (editor only)
Debug                  → debug wireframes, AABBs, etc.
```

### Pass anatomy

Each render pass follows this pattern:

```cpp
struct MyRenderPass
{
    // Called once at startup and on renderer reinit
    static void init();
    static void onReinitRendering();
    static void destroy();

    // Called every frame (or conditionally)
    static void render(float p_DeltaT);

    // Internal Vulkan objects
    static ImageRef _myImageRef;
    static RenderPassRef _renderPassRef;
    static PipelineRef _pipelineRef;
    // ...
};
```

### Adding a new render pass

1. Create `IntrinsicRendererRenderPassMyPass.h` and `.cpp`
2. Implement `init()`, `destroy()`, `onReinitRendering()`, `render()`
3. Register in `IntrinsicRendererRenderProcess.cpp`:

```cpp
// In _renderStepTypeMapping:
{"MyPass", RenderStepType::kMyPass},

// In _renderStepFunctionMapping:
{RenderStepType::kMyPass, RenderProcess::renderMyPass},

// Add function:
void RenderProcess::renderMyPass(float p_DeltaT) {
    RenderPassMyPass::render(p_DeltaT);
}
```

4. Add the pass name to `renderer_config.json` in the `"renderSteps"` array.

### Image memory barriers — rules

These rules avoid Vulkan validation errors:

- Always use `VK_IMAGE_LAYOUT_UNDEFINED` as `oldLayout` at the start of each
  frame. This is always valid per spec and avoids the need to track the previous
  frame's layout.
- `init()` / `onReinitRendering()` must transition newly created images to their
  operational layout before the first `render()` call.
- Match `srcStage` to the actual last stage that wrote the image
  (`COMPUTE_SHADER_BIT`, `COLOR_ATTACHMENT_OUTPUT_BIT`, `TRANSFER_BIT`, etc.)
  — never use `TOP_OF_PIPE` as `srcStage` if real work precedes the barrier.
- Access masks must be `0` when the stage is `TOP_OF_PIPE` or `BOTTOM_OF_PIPE`.

```cpp
// Correct: compute write → fragment shader read
ImageManager::insertImageMemoryBarrier(
    imageRef,
    VK_IMAGE_LAYOUT_UNDEFINED,            // oldLayout (always safe)
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,  // newLayout
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // srcStage
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT // dstStage
);
```

### Accessing a render target from another pass

Render targets (images) are declared in `renderer_config.json` and allocated
once. Any pass can hold a reference:

```cpp
// In the pass header:
static ImageRef _gbufferDepthRef;

// In init():
_gbufferDepthRef = ImageManager::getResourceByName(Name("GBufferDepth"));
```

---

## 8. Asset Pipeline

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

## 9. Lua Scripting

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

## 10. Procedural Systems

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

## 11. Physics

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

## 12. Debugging & Profiling

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

## 13. Common Recipes

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
