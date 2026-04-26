# Zmiany wprowadzone w gałęzi `fixing_for_new_driver`

Gałąź naprawia kompatybilność silnika z nowymi sterownikami NVIDIA i nowszym
środowiskiem budowania (MSVC 14.44, Vulkan SDK 1.3+). Poniżej szczegółowy opis
każdego commita w porządku chronologicznym.

---

## 1. `670290a1` — Naprawa budowania z nowym MSVC i Vulkan SDK

**Pliki:** `IntrinsicCore/src/stdafx.h`, `cmake/FindLuaJIT.cmake`

MSVC 14.44 przestał dołączać `<chrono>` pośrednio przez `<thread>`, co powodowało
błąd kompilacji. Dodano jawny `#include <chrono>` do `stdafx.h`.

Skrypt `FindLuaJIT.cmake` szukał najpierw `libluajit.a` (format GCC), przez co na
Windows MSVC nie znajdował właściwej biblioteki. Zmieniono kolejność tak, żeby
`lua51.lib` był szukany jako pierwszy.

---

## 2. `460a12dd` — Naprawa decali na nowych sterownikach NVIDIA

**Pliki:** `app/assets/shaders/decals.frag.glsl`,
`app/assets/shaders/decals.inc.glsl`,
`IntrinsicRenderer/src/IntrinsicRendererRenderPassClustering.cpp`,
`IntrinsicRenderer/src/IntrinsicRendererResourcesPipeline.cpp`

**Problem:** Funkcja `calcDecal` przyjmowała `sampler2D[4095]` jako parametr.
Przekazywanie tablic samplerów jako argumentów funkcji GLSL nie jest legalne
w standardzie i powodowało GPU hang na nowych sterownikach.

**Rozwiązanie:** Usunięto tablicę z listy parametrów; shader odwołuje się do
`globalTextures` bezpośrednio (tak jak `lighting.inc.glsl` obsługuje
`globalCubeTextures`). Przy okazji usunięto debug hacki i poprawnie podpięto
tworzenie pipeline'u i draw calla dla decali.

---

## 3. `ab909002` — Usunięcie przestarzałego komentarza debug w Clustering

**Pliki:** `IntrinsicRenderer/src/IntrinsicRendererRenderPassClustering.cpp`

Drobny cleanup — usunięcie nieaktualnego komentarza.

---

## 4. `cf298e55` — Pierwsze błędy walidacji Vulkan (kategoria A)

**Pliki:** `IntrinsicRenderer/src/IntrinsicRendererHelper.h`,
`IntrinsicRenderer/src/IntrinsicRendererRenderSystem.cpp`

**Problem 1:** Specyfikacja Vulkan zabrania ustawiania niezerowych `srcAccessMask`
/ `dstAccessMask` gdy etap pipeline'u to `VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT`.
Wiele miejsc w kodzie ignorowało ten wymóg, co na nowych sterownikach generowało
błędy walidacji. Naprawiono w pomocniku barier przez zerowanie masek dostępu,
kiedy etap to `TOP_OF_PIPE`.

**Problem 2:** Rozszerzenie `VK_EXT_debug_marker` wymaga `VK_EXT_debug_report`
jako rozszerzenia urządzenia. Silnik próbował włączyć debug marker bez sprawdzenia,
czy debug report jest dostępny. Dodano weryfikację dostępności obu rozszerzeń
przed ich włączeniem.

---

## 5. `fa3d8e07` — Wykrywanie i włączanie `VK_KHR_maintenance2`

**Pliki:** `IntrinsicRenderer/src/IntrinsicRendererRenderSystem.cpp`

Rozszerzenie `VK_KHR_maintenance2` jest wymagane przez `VkImageViewUsageCreateInfo`
(używane w następnym kroku do naprawy widoków SRGB). Dodano wykrywanie i warunkowe
włączanie tego rozszerzenia podczas tworzenia urządzenia logicznego.

---

## 6. `e0b33ce6` — Naprawa barier pre-present i post-present

**Pliki:** `IntrinsicRenderer/src/IntrinsicRendererRenderSystem.h`

**Bariera pre-present:** `dstAccessMask` był ustawiony na `VK_ACCESS_MEMORY_READ_BIT`,
ale specyfikacja wymaga `0` dla etapu `BOTTOM_OF_PIPE`.

**Bariera post-present:** `dstStage` był `BOTTOM_OF_PIPE`, przez co
`VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT` było nieprawidłowe. Zmieniono na
`COLOR_ATTACHMENT_OUTPUT`.

---

## 7. `fd363ee7` — Naprawa barier dla `TOP_OF_PIPE` i `BOTTOM_OF_PIPE`

**Pliki:** `IntrinsicRenderer/src/IntrinsicRendererHelper.h`

Rozszerzono pomocnik barier obrazu i bufora: kiedy etap to `TOP_OF_PIPE` lub
`BOTTOM_OF_PIPE`, maski dostępu są automatycznie zerowane (zgodnie ze
specyfikacją Vulkan). Wcześniej wiele barier miało niezerowe maski dostępu
przy tych etapach, co było niezgodne ze specyfikacją.

---

## 8. `dd8082e4` — Dodanie flagi `STORAGE_BIT` do vertex bufferów

**Pliki:** `IntrinsicRenderer/src/IntrinsicRendererHelper.h`

Vertex buffery używane w compute shaderach (marching cubes, geometry generation)
muszą mieć ustawioną flagę `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT`. Dodano ją
do domyślnego zestawu flag w pomocniku tworzenia bufforów.

---

## 9. `689f3ea7` — Guard przed podwójnym `vkBindBufferMemory`

**Pliki:** `IntrinsicRenderer/src/IntrinsicRendererResourcesBuffer.cpp`

`vkBindBufferMemory` musi być wywołane tylko przy nowej alokacji. Nowe sterowniki
traktują ponowne bindowanie jako błąd walidacji. Dodano warunek sprawdzający,
czy alokacja rzeczywiście nastąpiła, zanim wywoła się `vkBindBufferMemory`.

---

## 10. `73af179b` — `VkImageViewUsageCreateInfo` dla widoków SRGB

**Pliki:** `IntrinsicRenderer/src/IntrinsicRendererResourcesImage.cpp`

Tekstury SRGB tworzą dwa widoki: jeden jako SRGB (do odczytu) i jeden jako
UNORM (do storage). Nowe sterowniki wymagają jawnego określenia dozwolonych
użyć widoku przez `VkImageViewUsageCreateInfo`. Dodano strukturę dla widoku SRGB
z flagą `VK_IMAGE_USAGE_SAMPLED_BIT` (bez `STORAGE_BIT`), żeby walidator nie
narzekał na niezgodność użyć.

---

## 11. `1d7e94ab` — Naprawa barier obrazu i aktualizacji bufora w geometry/texture generation

**Pliki:** `IntrinsicRendererRenderPassDynamicGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassDynamicTextureGeneration.cpp`,
`IntrinsicRendererRenderPassGeometryGeneration.cpp`

**Problem 1 (krytyczny):** `updateDataMemory` wywoływało `vkCmdCopyBuffer`
z tym samym buforem jako źródłem i celem — co jest niezdefiniowanym
zachowaniem. Zastąpiono bezpośrednim `memcpy` do pamięci host-visible.

**Problem 2:** Brakujące przejście `UNDEFINED→GENERAL` dla `normalsTex`
i generowanych tekstur podczas inicjalizacji powodowało, że compute shadery
trafiały na obrazy w niezdefiniowanym layoutcie.

**Problem 3:** Bariery `TRANSFER_SRC/DST` zamieniono na `GENERAL` z poprawnymi
etapami pipeline'u.

**Problem 4:** Dodano przejście `GENERAL→SHADER_READ_ONLY_OPTIMAL` przed
dispatchem marching cubes.

---

## 12. `bd927c75` — Naprawa etapów barier w Bloom i VolumetricLighting

**Pliki:** `IntrinsicRendererRenderPassBloom.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`

**Bloom:** Bariera po dispatch lum używała `TOP_OF_PIPE` jako `srcStage`
zamiast `COMPUTE_SHADER_BIT`. Dodano guard dla pierwszej klatki, żeby kolejne
klatki używały faktycznego layoutu `SHADER_READ_ONLY` zamiast `UNDEFINED`.
Naprawiono `dstStage` dla finalnej bariery `_lumImageRef` na `FRAGMENT_SHADER_BIT`.

**VolumetricLighting:** Bariery po dispatch akumulacji używały `TOP_OF_PIPE`
zamiast `COMPUTE_SHADER_BIT` jako `srcStage`. Dodano guard dla pierwszej klatki.
Naprawiono `dstStage` bariery scatter buffera na `FRAGMENT_SHADER_BIT`.

---

## 13. `fb5bb76a` — Naprawa layoutu `UNDEFINED` w Shadow, Clustering, VolumetricLighting, PerPixelPicking

**Pliki:** `IntrinsicRendererRenderPassShadow.cpp`,
`IntrinsicRendererRenderPassClustering.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`,
`IntrinsicRendererRenderPassPerPixelPicking.cpp`

**Problem:** Od drugiej klatki każdy obraz jest w konkretnym, znanych layoucie.
Używanie `UNDEFINED` jako `oldLayout` w barierach wyzwalało błędy śledzenia
layoutu na nowych sterownikach.

**Rozwiązanie:** Dodano flagi `_shadowRendered`, `_clusteringRendered`,
`_volLightingRendered`, `_pickingRendered`. Pierwsza klatka używa `UNDEFINED`,
kolejne podają faktyczny layout końcowy z poprzedniej klatki
(`SHADER_READ_ONLY_OPTIMAL`, `DEPTH_STENCIL_ATTACHMENT_OPTIMAL`,
`TRANSFER_SRC_OPTIMAL`). Flagi są resetowane w `onReinitRendering`.

---

## 14. `c5ddc856` — Naprawa etapów barier bufora w geometry/marching cubes

**Pliki:** `IntrinsicRendererRenderPassDynamicGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassMarchingCubes.cpp`

Bariery używające domyślnych etapów `TOP_OF_PIPE` miały swoje maski dostępu
zerowane przez pomocnik (patrz `fd363ee7`), przez co były faktycznie no-opami.

**Naprawiono:** Voxel buffery: `COMPUTE→COMPUTE`. Wyjściowe buffery vertex
(position, normal, itd.): `COMPUTE→VERTEX_INPUT` z `dstAccessMask =
VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT`, bo są konsumowane jako vertex attributes
po dispatch marching cubes.

---

## 15. `9bfb146d` — Naprawa etapów barier w texture upload i PerPixelPicking

**Pliki:** `IntrinsicRendererResourcesImage.cpp`,
`IntrinsicRendererRenderPassPerPixelPicking.cpp`

**ResourcesImage:** Bariera `UNDEFINED→TRANSFER_DST` używa teraz
`TOP_OF_PIPE→TRANSFER`. Bariera `TRANSFER_DST→SHADER_READ_ONLY` używa
`TRANSFER→FRAGMENT_SHADER`. Dotyczy wszystkich ścieżek uploadu tekstur.

**PerPixelPicking:** Bariera `TRANSFER_SRC→COLOR_ATTACHMENT` używa
`TRANSFER→COLOR_ATTACHMENT_OUTPUT`. Bariera powrotna `COLOR_ATTACHMENT→TRANSFER_SRC`
używa `COLOR_ATTACHMENT_OUTPUT→TRANSFER`. Bariera depth pominięta w kolejnych
klatkach (render pass sam obsługuje przejście).

---

## 16. `750aa2fd` — Naprawa błędów layoutu pierwszej klatki w VolumetricLighting

**Pliki:** `IntrinsicRendererRenderPassVolumetricLighting.cpp`,
`IntrinsicRendererRenderSystem.cpp`, `IntrinsicRendererRenderProcess.cpp`,
`IntrinsicCoreApplication.cpp`

Wszystkie obrazy ESM i volumetric lighting są inicjalizowane do
`SHADER_READ_ONLY_OPTIMAL` w `init()` przez tymczasowy command buffer. Dzięki
temu draw calle bindujące całą tablicę obrazów (wszystkie warstwy) nie widzą
`UNDEFINED` w żadnej warstwie na pierwszej klatce. Flaga `_volLightingRendered`
jest ustawiana na `true` już w `init()`, żeby bariery per-frame zaczynały od
`SHADER_READ_ONLY` zamiast `UNDEFINED`.

---

## 17. `d8793ced` — Przejście obrazów do `SHADER_READ_ONLY` po reinit

**Pliki:** `IntrinsicRendererRenderProcess.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`,
`IntrinsicRendererResourcesImage.cpp`

**Problem:** Obrazy resolution-dependent (tworzone świeżo przy każdym reinit)
startują w layoucie `UNDEFINED`. Globalne deskryptory tekstur bindują te obrazy
jako `SHADER_READ_ONLY_OPTIMAL`, co powodowało błąd walidacji przy pierwszym
submit po reinit.

**Rozwiązanie:** Po stworzeniu wszystkich obrazów w `loadRendererConfig()` dodano
tymczasowy command buffer przechodzący je z `UNDEFINED` do `SHADER_READ_ONLY_OPTIMAL`.
Naprawiono też `VolumetricLighting::onReinitRendering()` by prawidłowo
inicjalizowało obrazy statyczne (nie resolution-dependent) do `SHADER_READ_ONLY`
po reinit.

---

## 18. `57e8245a` — Naprawa `VUID-VkDescriptorImageInfo-imageLayout-00344` w DynamicTextureGeneration

**Pliki:** `IntrinsicRendererRenderPassDynamicTextureGeneration.cpp`,
`IntrinsicRendererRenderSystem.cpp`, `IntrinsicCoreApplication.cpp`

**Problem:** Compute shadery generacji tekstur deklarują `_TextureTex` i
`_SourceTex` jako `image2D` (storage images), co wymaga layoutu `GENERAL`.
Bariera przed dispatchem używała `UNDEFINED` jako `oldLayout`, co warstwa
walidacji śledziła nieprawidłowo.

**Rozwiązanie:** Bariery używają teraz faktycznego bieżącego layoutu
(`SHADER_READ_ONLY_OPTIMAL→GENERAL`). Tekstury źródłowe (`hasSourceTex=true`)
otrzymują jawne bariery `SHADER_READ_ONLY→GENERAL` przed dispatchem i
`GENERAL→SHADER_READ_ONLY` po nim, żeby `house_01_E_GEN` był w `GENERAL`
kiedy NRM generation dispatch odwołuje się do niego przez `_SourceTex`.

---

## 19. `0b7ae636` — Naprawa `VUID-VkWriteDescriptorSet-descriptorType-00339`

**Pliki:** `IntrinsicRendererRenderSystem.cpp`, `IntrinsicCoreResourcesMesh.cpp`,
`IntrinsicRendererRenderProcess.cpp`, `IntrinsicRendererResourcesImage.cpp`

**Problem (głęboka analiza):** `concrete_NRM_GEN` (compute call indeks 14) i
`concrete_PBR_GEN` (indeks 15) używały `concrete_GEN` jako tekstury źródłowej.
Wpis single-texture dla `concrete_GEN` był zakomentowany w `RenderSystem.cpp`,
więc `getResourceByName("concrete_GEN")` wracał do fallbacku — tekstury
`checkerboard` w formacie BC1 SRGB. BC1 nie ma flagi
`VK_IMAGE_USAGE_STORAGE_BIT`, więc `vkUpdateDescriptorSets` zgłaszało błąd
walidacji przy próbie zbindowania jej jako `VK_DESCRIPTOR_TYPE_STORAGE_IMAGE`.

**Rozwiązanie:** Odkomentowano wpis dla `concrete_GEN` i zastąpiono nieistniejący
shader `texture_concrete_generation.comp` istniejącym
`texture_bricks_generation.comp`. Usunięto też debug logging dodany podczas
śledztwa.

---

## 20. `21964455` — Usunięcie markerów `/* PJ: */` z RenderProcess.cpp

**Pliki:** `IntrinsicRendererRenderProcess.cpp`

Usunięto 4 bezsensowne markery debugowe `/* PJ: */` z tablic
`_renderStepTypeMapping` i `_renderStepFunctionMapping`.

---

## 21. `21816ab3` — Usunięcie flag `_*Rendered` z render passów

**Pliki:** `IntrinsicRendererRenderPassBloom.cpp`,
`IntrinsicRendererRenderPassClustering.cpp`,
`IntrinsicRendererRenderPassPerPixelPicking.cpp`,
`IntrinsicRendererRenderPassShadow.cpp`,
`IntrinsicRendererRenderPassVolumetricLighting.cpp`

Flagi `_bloomRendered`, `_clusteringRendered`, `_pickingRendered`,
`_shadowRendered`, `_volLightingRendered` (dodane w commitach `bd927c75` i
`fb5bb76a`) okazały się zbędne. `VK_IMAGE_LAYOUT_UNDEFINED` jako `oldLayout`
jest zawsze akceptowane przez sterownik i warstwę walidacji, bo funkcje
`init()` / `onReinitRendering()` już wcześniej przechodzą obrazy do właściwego
layoutu. Silnik działa poprawnie bez flag, bez żadnych warningów walidacji.

---

## 22. `d3116793` — Naprawa UB: dangling reference i brakujący return

**Pliki:** `IntrinsicRendererPseudoInstancing.cpp`,
`IntrinsicRendererRenderPassDynamicGeometryGeneration.cpp`,
`IntrinsicRendererRenderPassDynamicGeometryGeneration.h`

**Problem 1 (C4172):** `DynamicGeometryGeneration::getNormal()` była
zadeklarowana jako zwracająca `glm::vec3&`, ale wewnętrznie konstruowała
tymczasowy obiekt `glm::vec3(...)` i zwracała do niego referencję — klasyczny
dangling reference (UB). Zmieniono typ zwracany na `glm::vec3` (by value).

**Problem 2 (C4715):** `PseudoInstancing::getMeshSizes()` nie miała instrukcji
`return` na ścieżce, gdy pętla kończy się bez znalezienia meshy — niezdefiniowane
zachowanie. Dodano `_INTR_ASSERT(false)` i fallback return aby zadowolić
kompilator (funkcja jest wołana tylko po pozytywnym wyniku `isInstancedMesh`).

---

## 23. `ba9758bb` — Naprawa warningów C4267/C4018/C4305

**Pliki:** `IntrinsicCoreResourcesMesh.cpp`, `IntrinsicCoreWorld.cpp`,
`IntrinsicRendererRenderPassDynamicTextureGeneration.cpp`,
`IntrinsicRendererRenderPassGeometryGeneration.cpp`,
`IntrinsicRendererRenderSystem.cpp`

- **C4267** (`size_t→uint32_t`): jawne rzutowania `(uint32_t)` przy przypisaniach
  z `.size()` wektora w `ResourcesMesh.cpp` (8 miejsc) i `World.cpp` (1 miejsce).
- **C4018** (signed/unsigned): pętle `for (int j = ...)` porównujące z
  `uint32_t` zmieniono na `for (uint32_t j = 0u; ...)` w `ResourcesMesh.cpp`
  i `World.cpp`.
- **C4305** (`double→float`): dodano sufiks `f` do literałów zmiennoprzecinkowych
  (`0.02f`, `2.0f`, `1.0f`, `0.1f`, `1.01f`, `5.0f`, `-5.0f`, `2 * 3.1415f`
  itd.) w kilku plikach generacji geometrii i tekstur.

---

## Podsumowanie

| Kategoria | Liczba commitów |
|-----------|----------------|
| Budowanie / środowisko | 1 |
| Naprawa decali (GLSL) | 1 |
| Vulkan validation errors (bariery, layouty, rozszerzenia) | 16 |
| Cleanup kodu | 2 |
| Naprawa UB i warningów kompilatora | 2 |
| **Łącznie** | **22** |
