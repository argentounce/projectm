# ProjectM Metal Port - Handoff Document

**Date:** 2024-10-10  
**Session:** Metal Abstraction Layer Implementation (Phase 1 Complete)

---

## Current State

### ✅ Completed

#### Architecture Investigation (Session 1)
- Read and analyzed ~15 key source files
- Documented projectM architecture in `INVESTIGATION.MD`
- Identified core components: ProjectM, PCM, Preset, Renderer, Audio
- Analyzed projectM-eval expression evaluator
- Documented `.milk` preset format and JSON proposal

#### Metal Abstraction Layer (Session 2)
- ✅ **MetalResolver** (`src/libprojectM/Renderer/Platform/MetalResolver.hpp/cpp`)
  - Device selection strategies (Default, Integrated, Discrete, CPU)
  - Command queue management
  - Device capabilities query
  - Thread-safe singleton pattern
  - Debug layer support
  - Environment variables: `PROJECTM_METAL_DEVICE_SELECTION`, `PROJECTM_METAL_DEBUG`

- ✅ **MetalRenderer** (`src/libprojectM/Renderer/MetalRenderer.hpp/cpp`)
  - Frame rendering
  - Texture management
  - Shader management
  - Viewport and aspect ratio handling
  - Preset path management
  - Texture loading and caching

- ✅ **MetalShader** (`src/libprojectM/Renderer/MetalShader.hpp/cpp`)
  - GLSL → MSL conversion utilities (regex-based)
  - Shader cache management
  - Pipeline state factory
  - Vertex descriptor generation

- ✅ **MetalTexture** (`src/libprojectM/Renderer/MetalTexture.hpp/cpp`)
  - Texture creation from data and files
  - Texture caching
  - Sampler management
  - Texture age tracking for purging
  - File scanning with stb_image

- ✅ **RenderPipelineState** (`src/libprojectM/Renderer/RenderPipelineState.hpp/cpp`)
  - MTLRenderPipelineState wrapper
  - Vertex/fragment function access

- ✅ **ComputePipelineState** (`src/libprojectM/Renderer/ComputePipelineState.hpp/cpp`)
  - MTLComputePipelineState wrapper
  - Compute function access

- ✅ **Documentation**
  - `src/libprojectM/Renderer/METAL_ABSTRACTION_LAYERS.md` - Comprehensive design doc
  - Updated `INVESTIGATION.MD` with Metal implementation progress

### 🔄 In Progress

#### Metal Shader Compilation
- **Current:** Regex-based GLSL → MSL conversion (placeholder)
- **Status:** Basic conversion implemented, needs full implementation
- **Files:** `MetalShader.cpp`, `MetalShaderCompiler.cpp`

#### Preset Rendering
- **Current:** Placeholder in `RenderFrame()`
- **Status:** Needs full implementation to render actual presets
- **Files:** `MetalRenderer.cpp`

#### Texture File Loading
- **Current:** Uses stb_image for basic formats (JPEG, PNG, TGA, BMP)
- **Status:** Needs full DDS support with mipmaps
- **Files:** `MetalTexture.cpp`

### ❌ Blocked/Not Started

1. **GLSL → MSL Production Conversion**
   - Current regex approach is error-prone
   - Need to integrate glslangValidator or glslang runtime

2. **Full Preset Rendering Logic**
   - Need to port OpenGL rendering code to Metal
   - Vertex buffer management
   - Per-pixel mesh rendering
   - Shader uniform updates

3. **Metal Compute Shaders**
   - Audio processing shaders
   - FFT computation shaders

4. **Build System**
   - Need to replace CMake (per user preference)
   - Not yet designed

5. **New Preset Format**
   - JSON format specification drafted
   - Parser not implemented
   - Converter tool not written

---

## Key Decisions Made

### 1. **Mirror OpenGL Architecture**
   - **Why:** Maintain consistency with existing codebase, easier migration
   - **Impact:** Metal abstraction follows same patterns as GLResolver, Shader, TextureManager
   - **Files:** `MetalResolver.hpp`, `MetalRenderer.hpp`, `MetalTexture.hpp`

### 2. **GLSL → MSL Conversion Strategy**
   - **Decision:** Regex-based initial implementation, glslangValidator for production
   - **Why:** Quick prototype, industry-standard tool for production
   - **Impact:** Short-term manual conversion, long-term external tool integration
   - **Files:** `MetalShaderCompiler.cpp`

### 3. **Device Selection Strategies**
   - **Decision:** Support Default, Integrated, Discrete, CPU
   - **Why:** User control over GPU selection, power/performance trade-offs
   - **Impact:** Environment variable `PROJECTM_METAL_DEVICE_SELECTION`
   - **Files:** `MetalResolver.hpp`, `MetalResolver.cpp`

### 4. **Texture Management**
   - **Decision:** Use stb_image for basic formats, expand to DDS later
   - **Why:** Simple implementation, good enough for initial port
   - **Impact:** Start with RGBA8Unorm, expand formats as needed
   - **Files:** `MetalTexture.cpp`

### 5. **Command Buffer Management**
   - **Decision:** Create per-frame command buffers
   - **Why:** Simple pattern, matches OpenGL frame-by-frame rendering
   - **Impact:** One command buffer per frame, commit at end
   - **Files:** `MetalRenderer.cpp`

### 6. **No CMake**
   - **Decision:** Replace CMake with custom build system
   - **Why:** User preference
   - **Impact:** Need to design new build system (not yet done)
   - **Files:** TBD

### 7. **Keep projectM-eval**
   - **Decision:** Continue using projectM-eval dependency
   - **Why:** Rich ecosystem, expression evaluator already implemented
   - **Impact:** No need to reimplement expression evaluation
   - **Files:** `vendor/projectm-eval/`

---

## Open Questions / Unresolved Threads

### 1. **GLSL → MSL Production Conversion**
   - **Question:** Should we use glslangValidator (external tool) or glslang runtime (library)?
   - **Options:**
     - glslangValidator: External subprocess, industry-standard, slower
     - glslang runtime: Library integration, faster, larger binary
     - Manual conversion: Full control, error-prone, limited
   - **Status:** Not decided
   - **Impact:** Affects shader compilation performance and complexity

### 2. **Build System Choice**
   - **Question:** What build system to use instead of CMake?
   - **Options:**
     - Make: Simple, widely supported, manual dependency management
     - Bazel: Modern, scalable, complex setup
     - Custom: Full control, more work
   - **Status:** Not decided
   - **Impact:** Affects project setup and developer experience

### 3. **New Preset Format Specification**
   - **Question:** What features to add beyond .milk?
   - **Options:**
     - JSON with schema validation
     - YAML for human readability
     - TOML for simplicity
   - **Status:** Draft in INVESTIGATION.MD, needs refinement
   - **Impact:** Affects preset authoring and migration

### 4. **Audio Backend**
   - **Question:** Which audio APIs to support?
   - **Options:**
     - CoreAudio (macOS only)
     - JACK (Linux, professional)
     - PulseAudio (Linux, desktop)
     - WASAPI (Windows)
   - **Status:** Currently using C for cross-platform, needs implementation
   - **Impact:** Affects cross-platform support

### 5. **Performance Optimization**
   - **Question:** How to optimize Metal rendering?
   - **Options:**
     - Command buffer batching
     - Texture streaming
     - Metal Performance Shaders (MPS)
     - MetalFX integration
   - **Status:** Not yet considered
   - **Impact:** Affects frame rate and visual quality

### 6. **Multi-GPU Support**
   - **Question:** Should we support multiple GPUs?
   - **Options:**
     - Single GPU (simpler)
     - Multi-GPU (complex, better performance)
   - **Status:** Not considered
   - **Impact:** Affects device selection and rendering strategy

---

## Next Steps for Fresh Session

### Priority 1: GLSL → MSL Conversion (Week 3)

1. **Choose Conversion Strategy**
   - Decide: glslangValidator vs glslang runtime vs manual
   - File: `src/libprojectM/Renderer/MetalShaderCompiler.cpp`

2. **Implement glslangValidator Integration**
   - Create subprocess invocation
   - Handle errors and output parsing
   - File: `src/libprojectM/Renderer/MetalShaderCompiler.cpp`

3. **Test Conversion**
   - Convert existing GLSL shaders to MSL
   - Verify shader compilation
   - Test with presets

### Priority 2: Preset Rendering (Week 3-4)

1. **Port OpenGL Rendering to Metal**
   - Read `src/libprojectM/Renderer/OpenGL.h` implementation
   - Map OpenGL calls to Metal equivalents
   - File: `src/libprojectM/Renderer/MetalRenderer.cpp`

2. **Implement Vertex Buffer Management**
   - Create vertex buffers for per-pixel mesh
   - Map vertex attributes to Metal
   - File: `MetalRenderer.cpp`

3. **Implement Shader Uniform Updates**
   - Map GLSL uniforms to Metal buffers
   - Update uniforms per frame
   - File: `MetalRenderer.cpp`

4. **Complete RenderFrame()**
   - Full preset rendering logic
   - Test with existing presets

### Priority 3: Build System (Week 4)

1. **Choose Build System**
   - Decide: Make vs Bazel vs custom
   - File: `BUILD.md` or similar

2. **Implement Build System**
   - Define targets and dependencies
   - Replace CMakeLists.txt
   - File: `Makefile` or `BUILD.bazel`

3. **Test Build**
   - Build all components
   - Verify Metal compilation
   - Test with presets

### Priority 4: Texture Loading (Week 4)

1. **Add DDS Support**
   - Integrate stb_image_write or similar
   - Support mipmaps
   - File: `MetalTexture.cpp`

2. **Expand Format Support**
   - Add more pixel formats
   - Handle different color spaces
   - File: `MetalTexture.cpp`

### Priority 5: Audio Backend (Month 2+)

1. **Design Audio Abstraction**
   - Define interface for audio backends
   - File: `src/libprojectM/Audio/AudioBackend.hpp`

2. **Implement CoreAudio Backend**
   - macOS-specific implementation
   - File: `src/libprojectM/Audio/CoreAudioBackend.hpp/cpp`

3. **Implement Cross-Platform Backend**
   - JACK, PulseAudio, WASAPI support
   - File: `src/libprojectM/Audio/CrossPlatformBackend.hpp/cpp`

---

## Relevant File Paths

### Metal Abstraction Layer (NEW)
- `src/libprojectM/Renderer/Platform/MetalResolver.hpp` - Device resolver
- `src/libprojectM/Renderer/Platform/MetalResolver.cpp` - Resolver implementation
- `src/libprojectM/Renderer/MetalRenderer.hpp` - Main renderer
- `src/libprojectM/Renderer/MetalRenderer.cpp` - Renderer implementation
- `src/libprojectM/Renderer/MetalShader.hpp` - Shader compilation
- `src/libprojectM/Renderer/MetalShader.cpp` - Shader implementation
- `src/libprojectM/Renderer/MetalTexture.hpp` - Texture management
- `src/libprojectM/Renderer/MetalTexture.cpp` - Texture implementation
- `src/libprojectM/Renderer/RenderPipelineState.hpp/cpp` - Render pipeline wrapper
- `src/libprojectM/Renderer/ComputePipelineState.hpp/cpp` - Compute pipeline wrapper
- `src/libprojectM/Renderer/METAL_ABSTRACTION_LAYERS.md` - Design documentation

### Existing OpenGL Renderer (REFERENCE)
- `src/libprojectM/Renderer/OpenGL.h` - OpenGL headers
- `src/libprojectM/Renderer/Platform/GLResolver.hpp/cpp` - GL resolver
- `src/libprojectM/Renderer/Shader.hpp/cpp` - Shader management
- `src/libprojectM/Renderer/TextureManager.hpp/cpp` - Texture management
- `src/libprojectM/Renderer/ShaderCache.hpp/cpp` - Shader caching

### Core Library
- `src/libprojectM/ProjectM.hpp` - Main class
- `src/libprojectM/Audio/PCM.hpp` - Audio processing
- `src/libprojectM/MilkdropPreset/PresetState.hpp` - Preset state
- `src/libprojectM/MilkdropPreset/PresetFileParser.cpp` - INI parser
- `src/libprojectM/MilkdropPreset/Factory.hpp` - Factory pattern

### External Dependencies
- `vendor/projectm-eval/api/projectm-eval.h` - Expression evaluator API
- `vendor/glad/` - OpenGL loader
- `vendor/glm/` - Math library
- `vendor/stb_image/` - Image loading

### Documentation
- `INVESTIGATION.MD` - Architecture findings
- `HANDOFF.md` - This document
- `src/libprojectM/Renderer/METAL_ABSTRACTION_LAYERS.md` - Metal design

---

## Environment Notes

- **OS:** macOS (based on file paths)
- **Project Location:** `/Users/levaurei/Code/libs/projectm`
- **Git Submodule:** `vendor/projectm-eval` checked out at commit `22fb0cfd8f2dfbcd2b68f2443e7f44e19b32c09a`
- **Build System:** Currently uses CMake (to be replaced)
- **Metal SDK:** Available on macOS (include `<Metal/Metal.h>`)

---

## Quick Reference Commands

```bash
# Read key files
cat src/libprojectM/ProjectM.hpp
cat src/libprojectM/Audio/PCM.hpp
cat vendor/projectm-eval/api/projectm-eval.h
cat src/libprojectM/Renderer/Platform/MetalResolver.hpp
cat src/libprojectM/Renderer/MetalRenderer.hpp

# List Metal files
find src/libprojectM/Renderer -name "*Metal*" -o -name "*metal*"

# Check git status
git status
git submodule status

# Compile Metal code (test)
clang++ -std=c++17 -I. -Ivendor/glm -Ivendor/glad \
  -framework Metal \
  src/libprojectM/Renderer/Platform/MetalResolver.cpp \
  -o test_metal_resolver

# Run glslangValidator (if installed)
glslangValidator -V test.vert -o test.vert.msl
```

---

## Summary

**Phase 1 Complete:** Metal abstraction layer implemented with resolver, renderer, shader, and texture management.

**Phase 2 In Progress:** GLSL → MSL conversion needs production implementation, preset rendering needs completion.

**Phase 3 Not Started:** Build system, new preset format, audio backend.

**Next Session:** Focus on GLSL → MSL conversion strategy and preset rendering implementation.

---

*End of Handoff Document*
