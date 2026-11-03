# projectM Architecture Investigation

**Date:** 2024  
**Goal:** Analyze projectM architecture for Metal port and new preset format support

---

## Table of Contents

1. [Overview](#overview)
2. [Project Structure](#project-structure)
3. [Core Architecture](#core-architecture)
4. [Audio Processing](#audio-processing)
5. [Preset System](#preset-system)
6. [Rendering Pipeline](#rendering-pipeline)
7. [Shader System](#shader-system)
8. [Metal Port Strategy](#metal-port-strategy)
9. [New Preset Format Design](#new-preset-format-design)
10. [Language Choices](#language-choices)
11. [Next Steps](#next-steps)

---

## Overview

**projectM** is an open-source library that reimplements the Winamp Milkdrop visualizer. It reads audio input, performs FFT analysis, and renders mesmerizing visuals using shaders.

- **Current State:** C++ library with C API, OpenGL-based rendering
- **License:** LGPL 2.1
- **Version:** 4.x
- **Key Dependencies:** OpenGL, GLM, projectM-eval (expression evaluator)

### Core Purpose
```
Audio Input → FFT Analysis → Preset Evaluation → Shader Rendering → Output
```

---

## Project Structure

```
projectm/
├── src/
│   ├── api/include/projectM-4/      # C API headers
│   │   ├── core.h
│   │   ├── audio.h
│   │   ├── callbacks.h
│   │   ├── parameters.h
│   │   └── render_opengl.h
│   ├── libprojectM/                 # Main C++ library
│   │   ├── ProjectM.hpp             # Main wrapper class
│   │   ├── ProjectMCWrapper.cpp     # C API bridge
│   │   ├── Audio/
│   │   │   ├── PCM.hpp              # Audio processing
│   │   │   ├── MilkdropFFT.hpp/cpp  # FFT implementation
│   │   │   ├── WaveformAligner.hpp  # Waveform alignment
│   │   │   └── Loudness.hpp         # Beat detection
│   │   ├── MilkdropPreset/
│   │   │   ├── PresetState.hpp      # Preset runtime state
│   │   │   ├── PresetFileParser.cpp # .milk file parsing
│   │   │   ├── PerFrameContext.hpp  # Per-frame code execution
│   │   │   └── PerPixelContext.hpp  # Per-pixel code execution
│   │   └── Renderer/
│   │       ├── RenderContext.hpp    # Per-frame render data
│   │       ├── Shader.hpp           # Shader program management
│   │       ├── ShaderCache.hpp      # Shader caching
│   │       ├── TextureManager.hpp   # Texture loading
│   │       ├── Framebuffer.hpp      # FBO management
│   │       └── Platform/            # OpenGL abstraction
│   └── presets/                     # Preset files
│       └── tests/                   # Test presets (.milk files)
├── vendor/
│   ├── projectm-eval/               # Expression evaluator (NOT checked out)
│   ├── glad/                        # OpenGL loader
│   ├── glm/                         # GLM math library
│   └── stb_image/                   # Image loading
└── docs/
    ├── index.rst
    └── building.rst
```

---

## Core Architecture

### Main Class: `ProjectM`

The central `ProjectM` class manages the entire visualization pipeline:

```cpp
class ProjectM {
    // Audio
    Audio::PCM m_audioStorage;           // FFT, waveform, beat detection
    
    // Rendering
    std::unique_ptr<Renderer::TextureManager> m_textureManager;
    std::unique_ptr<Renderer::ShaderCache> m_shaderCache;
    std::unique_ptr<Renderer::CopyTexture> m_textureCopier;
    
    // Presets
    std::unique_ptr<Preset> m_activePreset;
    std::unique_ptr<Preset> m_transitioningPreset;
    std::unique_ptr<PresetTransition> m_transition;
    
    // State
    std::unique_ptr<TimeKeeper> m_timeKeeper;
    PresetFactoryManager m_presetFactoryManager;
};
```

### Key Responsibilities

| Component | Responsibility |
|-----------|---------------|
| `ProjectM` | Main orchestrator, lifecycle management |
| `PCM` | Audio analysis, FFT, waveform alignment, beat detection |
| `PresetState` | Preset runtime state, variable storage, code execution |
| `RenderContext` | Per-frame render data (time, viewport, shaders) |
| `ShaderCache` | Compiled shader programs |
| `TextureManager` | Texture loading and management |

---

## Audio Processing

### Pipeline

```
Raw PCM → Circular Buffer → FFT → Spectrum + Waveform → Beat Detection → FrameAudioData
```

### Key Classes

#### `PCM` (Audio::PCM)
- **Circular buffers:** 576 samples input buffer
- **Waveform samples:** 480 samples per frame
- **Spectrum samples:** 512 frequency bins
- **Operations:**
  - `Add()` - Add PCM data to buffer
  - `UpdateFrameAudioData()` - Process one frame
  - `GetFrameAudioData()` - Return frame data

#### `MilkdropFFT`
- Performs FFT on waveform data
- Implements equalizer patterns
- Envelope curves for smooth transitions

#### `WaveformAligner`
- Mip-based algorithm to align waveforms with previous frame
- Produces calmer, smoother waveforms

#### `Loudness`
- Beat detection in 3 bands:
  - **Bass:** First sixth of spectrum
  - **Middles:** Second sixth
  - **Treble:** Third sixth
- Returns relative loudness (revolves around 1.0)

### FrameAudioData Structure
```cpp
struct FrameAudioData {
    // Beat detection (relative to previous frame)
    float bass, bassAtt;
    float mid, midAtt;
    float treb, trebAtt;
    float vol, volAtt;
    
    // Waveform (480 samples)
    std::array<float, 480> waveformLeft;
    std::array<float, 480> waveformRight;
    
    // Spectrum (512 bins)
    std::array<float, 512> spectrumLeft;
    std::array<float, 512> spectrumRight;
};
```

---

## Preset System

### Preset File Format (.milk)

```ini
[preset00]
fDecay=0.98
fWarpScale=2.85
zoom=1.0
rot=0.0

per_frame_1=ib_r=0.7+0.4*sin(3*time);
per_frame_2=ib_g=0.7+0.4*sin(4*time);

per_pixel_1=zoom=0.9615-rad*0.1;
```

### Supported Extensions
- `.milk` - Standard Milkdrop preset
- `.prjm` - projectM preset

### Preset State (`PresetState`)

```cpp
class PresetState {
    // Runtime variables (BlendableFloat = float)
    float decay, waveScale, warpScale, zoom, rot;
    float ob_r, ob_g, ob_b, ob_a;  // Outer border
    float ib_r, ib_g, ib_b, ib_a;  // Inner border
    
    // Code execution
    std::string perFrameInitCode;
    std::string perFrameCode;
    std::string perPixelCode;
    
    // Shaders
    std::string warpShader;
    std::string compositeShader;
    
    // Eval library
    projectm_eval_mem_buffer globalMemory;
    double globalRegisters[100];
    std::array<double, QVarCount> frameQVariables;
    
    // Audio data (per frame)
    Audio::FrameAudioData audioData;
    
    // Render context
    Renderer::RenderContext renderContext;
};
```

### Code Execution Contexts

#### Per-Frame Context
- Runs once per frame
- Access to: `time`, `bass`, `mid`, `treb`, `vol`, spectrum, waveform
- Modifies: preset variables, Q variables

#### Per-Pixel Context
- Runs once per warp mesh vertex
- Access to: `time`, `rad` (pixel coordinates), `bass`, `mid`, `treb`, `vol`
- Modifies: `zoom`, `rad`, preset variables

### Custom Waveforms & Shapes
- Custom waveforms with init/per-frame/per-point code
- Custom shapes with init/per-frame code
- Defined in `Constants.hpp`

---

## Rendering Pipeline

### Frame Rendering Flow

```cpp
void ProjectM::RenderFrame(uint32_t targetFramebufferObject) {
    // 1. Update timers
    m_timeKeeper->UpdateTimers();
    
    // 2. Process audio
    auto audioData = m_audioStorage.GetFrameAudioData();
    
    // 3. Initialize preset if needed
    if (!m_activePreset) { LoadIdlePreset(); }
    
    // 4. Handle transitions
    if (m_transition) {
        if (transitionDone) {
            m_activePreset = m_transitioningPreset;
        } else {
            m_transitioningPreset->RenderFrame();
        }
    }
    
    // 5. Render preset
    m_activePreset->RenderFrame(audioData, renderContext);
    
    // 6. Copy to main framebuffer
    m_textureCopier->Draw(...);
    
    // 7. Draw user sprites
    m_spriteManager->Draw(...);
}
```

### Render Context

```cpp
struct RenderContext {
    float time{0.0f};          // Time since preset start
    int frame{0};              // Frame count
    float fps{0.0f};           // Frames per second
    float progress{0.0f};      // Preset progress
    int viewportSizeX{0};      // Viewport width
    int viewportSizeY{0};      // Viewport height
    float aspectX{1.0};        // Aspect ratio
    float perPixelMeshX{64};   // Warp mesh resolution
    float perPixelMeshY{48};   // Warp mesh resolution
    TextureManager* textureManager;
    ShaderCache* shaderCache;
};
```

### Per-Pixel Mesh

```cpp
class PerPixelMesh {
    // Creates a high-resolution mesh for per-pixel shader effects
    // Vertex count: perPixelMeshX * perPixelMeshY
    // Used to interpolate values across the screen
    // Fragment shader reads interpolated values
};
```

### Texture System

- **Main texture:** Output of preset rendering
- **Blur textures:** For blur effects (GetBlurX())
- **Random textures:** Procedurally generated textures
- **Texture manager:** Handles loading, caching, purging

---

## Shader System

### Shader Classes

#### `Shader`
- Compiles vertex + fragment shader pairs
- Validates program
- Sets uniforms (float, int, vec2, vec3, vec4, mat4)
- Binds/unbinds program

#### `ShaderCache`
- Stores compiled shaders by key
- Prevents recompilation
- Shared across projectM instance

### Shader Types

1. **Untextured shader:** For waveforms, primitives
2. **Textured shader:** For shapes, warp mesh
3. **Warp shader:** Transforms UV coordinates
4. **Composite shader:** Combines multiple layers

### Uniforms

| Type | Usage |
|------|-------|
| `time` | Animation time |
| `frame` | Frame counter |
| `bass/mid/treb/vol` | Beat detection values |
| `spectrum[]` | Full spectrum array |
| `waveform[]` | Waveform array |
| `viewportSize` | Render dimensions |
| `projectionMatrix` | Projection transform |

---

## Metal Port Strategy

### ✅ Implementation Status

**Date:** 2024-10-10  
**Status:** Phase 1 Complete - Metal Abstraction Layer Implemented

### Implemented Components

1. **MetalResolver** (`src/libprojectM/Renderer/Platform/MetalResolver.hpp/cpp`)
   - Device selection strategies (Default, Integrated, Discrete, CPU)
   - Command queue management
   - Device capabilities query
   - Thread-safe singleton pattern
   - Debug layer support

2. **MetalRenderer** (`src/libprojectM/Renderer/MetalRenderer.hpp/cpp`)
   - Frame rendering
   - Texture management
   - Shader management
   - Viewport and aspect ratio handling
   - Preset path management
   - Texture loading and caching

3. **MetalShader** (`src/libprojectM/Renderer/MetalShader.hpp/cpp`)
   - GLSL → MSL conversion utilities
   - Shader cache management
   - Pipeline state factory
   - Vertex descriptor generation

4. **MetalTexture** (`src/libprojectM/Renderer/MetalTexture.hpp/cpp`)
   - Texture creation from data and files
   - Texture caching
   - Sampler management
   - Texture age tracking for purging
   - File scanning

5. **RenderPipelineState** (`src/libprojectM/Renderer/RenderPipelineState.hpp/cpp`)
   - MTLRenderPipelineState wrapper
   - Vertex/fragment function access

6. **ComputePipelineState** (`src/libprojectM/Renderer/ComputePipelineState.hpp/cpp`)
   - MTLComputePipelineState wrapper
   - Compute function access

### Current OpenGL Abstraction

```cpp
// Platform abstraction in Renderer/Platform/
class GLResolver {
    void Initialize(void* (*load_proc)(const char*, void*), void* user_data);
};

class GladLoader {
    bool Initialize();
};
```

### Metal Migration Plan

#### ✅ Phase 1: Abstraction Layer (COMPLETE)
1. ✅ Create `Renderer/Platform/MetalResolver.hpp`
2. ✅ Create `Renderer/Platform/MetalResolver.cpp`
3. ✅ Implement GL equivalent functions for Metal

#### 🔄 Phase 2: Core Rendering (IN PROGRESS)
1. ⏳ Replace `GLRenderer` with `MetalRenderer`
2. ⏳ Implement Metal texture management
3. ⏳ Implement Metal shader compilation

#### ❌ Phase 3: Preset Integration (NOT STARTED)
1. ❌ Update `RenderContext` for Metal
2. ❌ Modify `PerPixelMesh` for Metal vertex buffers
3. ❌ Update `TextureManager` for Metal textures

### Key Metal Equivalents

| OpenGL | Metal |
|--------|-------|
| `glCreateProgram()` | `MTLRenderPipelineState` |
| `glCreateShader()` | `MTLCompileOptions` + `MTLFunction` |
| `glUseProgram()` | `commandEncoder.setRenderPipelineState()` |
| `glBindTexture()` | `MTLTexture` + `MTLRenderCommandEncoder` |
| `glFramebufferTexture2D()` | `MTLRenderPassDescriptor` |
| `glClear()` | `commandEncoder.clearRenderpass()` |

### Shader Compilation Strategy

**Current Implementation:**
- Regex-based GLSL → MSL conversion
- Handles: version, types, builtins, qualifiers, arrays, functions

**Recommended Production Approach:**
- Use `glslangValidator` as external tool
- Or integrate `glslang` runtime library for full GLSL support

```cpp
// GLSL → Metal Shading Language (MSL)
class MetalShader {
    void Compile(const std::string& glslSource, MTLFunction*& function);
    void Link(MTLRenderPipelineState*& pipelineState);
};
```

### Texture Management

```cpp
class MetalTextureManager {
    std::shared_ptr<MTLTexture> CreateTexture(
        const void* pixels,
        int width, int height,
        GLenum format, GLenum type
    );
    
    std::shared_ptr<MTLTexture> CreateTextureFromStream(
        const std::string& path
    );
};
```

### Environment Variables

```bash
# Metal device selection
export PROJECTM_METAL_DEVICE_SELECTION=default|integrated|discrete|cpu

# Metal debug layer
export PROJECTM_METAL_DEBUG=1

# Existing OpenGL variables (for comparison)
export PROJECTM_GLRESOLVER_DYLIB_DIR=path
export PROJECTM_GLRESOLVER_MACOS_PREFER_CGL=0
export PROJECTM_GLRESOLVER_STRICT_CONTEXT_GATE=0
```

---

## New Preset Format Design

### Goals

1. Support existing `.milk` presets (backward compatibility)
2. Introduce new format with better structure
3. Enable advanced features (custom shaders, effects)
4. Support both C++ and Rust implementations

### Proposed Format

```json
{
  "version": 2,
  "name": "Example Preset",
  "author": "Creator",
  
  "parameters": {
    "decay": 0.98,
    "warpScale": 2.85,
    "zoom": 1.0,
    "rotation": 0.0,
    "outerBorderSize": 0.2,
    "outerBorderColor": [0.0, 0.0, 0.0, 0.0],
    "innerBorderSize": 0.01,
    "innerBorderColor": [1.0, 0.0, 0.0, 1.0]
  },
  
  "code": {
    "perFrameInit": "ib_r = 0.5;",
    "perFrame": "bass = bass * 1.1;",
    "perPixel": "zoom = 1.0 - rad.x * 0.01;"
  },
  
  "warpShader": "// GLSL warp shader...",
  "compositeShader": "// GLSL composite shader...",
  
  "customWaveforms": [
    {
      "name": "custom1",
      "initCode": "x = 0.0;",
      "perFrameCode": "x += 0.01;",
      "perPointCode": "y = sin(x * 10);"
    }
  ],
  
  "customShapes": [
    {
      "name": "shape1",
      "initCode": "x = 0.0; y = 0.0;",
      "perFrameCode": "x += 0.01;"
    }
  ],
  
  "textures": [
    {
      "name": "myTexture",
      "path": "textures/myTexture.jpg",
      "wrapMode": "clamp",
      "filterMode": "linear"
    }
  ],
  
  "effects": {
    "videoEcho": {
      "zoom": 2.0,
      "alpha": 0.5,
      "orientation": 0
    },
    "solarize": false,
    "invert": false
  }
}
```

### Format Comparison

| Feature | .milk | New JSON Format |
|---------|-------|-----------------|
| Syntax | INI-style | JSON |
| Validation | Manual | Schema-based |
| Extensibility | Limited | High |
| Tooling | None | Rich (linters, validators) |
| Size | ~200-500 bytes | ~500-2000 bytes |
| Parsing | String parsing | JSON parser |

### Migration Strategy

1. **Dual parser:** Support both formats simultaneously
2. **Auto-detect:** Check file extension and content
3. **Conversion tool:** Convert .milk → JSON
4. **Fallback:** If JSON parsing fails, try .milk parser

---

## Language Choices

### Current Stack

| Language | Usage |
|----------|-------|
| C++ | Core library, API |
| C | C API bindings |
| GLSL | Shaders |

### Proposed Stack for Metal Port

| Language | Usage | Rationale |
|----------|-------|-----------|
| **C++** | Core library, presets, eval | Mature, rich ecosystem, projectM-eval integration |
| **Objective-C** | macOS bridge, Metal integration | Native Metal support, ObjC++ compatibility |
| **Swift** | Modern macOS/iOS apps | Modern syntax, better Metal support |
| **C** | Platform abstraction, audio I/O | Cross-platform, no dependencies |
| **Rust** | Optional: audio processing, eval | Memory safety, performance, potential future |

### Architecture by Language

**C++ Core Library:**
- `ProjectM.hpp` - Main wrapper class
- `Audio/` - Audio processing (PCM, FFT, waveform, loudness)
- `MilkdropPreset/` - Preset parsing and state management
- `Renderer/` - Rendering abstraction (OpenGL, Metal)

**Objective-C/macOS Bridge:**
- Application wrapper
- Metal integration layer
- UI bindings

**Swift (Future):**
- Modern macOS/iOS applications
- SwiftUI integration

**C Platform Abstraction:**
- Audio I/O interfaces
- Cross-platform utilities

**Rust (Optional Future):**
- Audio processing pipeline
- Expression evaluator
- Performance-critical code

```
┌─────────────────────────────────────────────────────────────┐
│                    Metal Abstraction Layer                   │
├─────────────────────────────────────────────────────────────┤
│  C++ Core Library  │  ObjC/macOS Bridge  │  Swift UI Layer  │
│  - ProjectM        │  - MetalRenderer    │  - App entry     │
│  - Preset system   │  - Texture manager  │  - Controls      │
│  - Audio processing│                     │                  │
├─────────────────────────────────────────────────────────────┤
│  C Audio I/O  │  C Platform Abstraction  │  Rust (future)   │
│  - JACK/Pulse │  - GL/Metal resolver     │  - Audio backend │
└─────────────────────────────────────────────────────────────┘
```

### Recommended Approach

1. **Core library:** Keep in C++ (backward compatibility, ecosystem)
2. **Metal abstraction:** C++ with platform-specific implementations
3. **macOS bridge:** ObjC++ for seamless Metal integration
4. **UI layer:** Swift for new macOS/iOS apps
5. **Audio backend:** C for cross-platform compatibility

---

## Next Steps

### ✅ Completed (Week 1)

1. ✅ Read and analyzed ~15 key source files
2. ✅ Documented projectM architecture in this document
3. ✅ Checked out `vendor/projectm-eval` submodule
4. ✅ Analyzed projectM-eval expression evaluator
5. ✅ Reviewed existing shader files

### 🔄 In Progress (Week 2)

#### Metal Abstraction Layer (PHASE 1 COMPLETE)

**Completed:**
- ✅ `src/libprojectM/Renderer/Platform/MetalResolver.hpp/cpp` - Device resolver
- ✅ `src/libprojectM/Renderer/MetalRenderer.hpp/cpp` - Main renderer
- ✅ `src/libprojectM/Renderer/MetalShader.hpp/cpp` - Shader compilation
- ✅ `src/libprojectM/Renderer/MetalTexture.hpp/cpp` - Texture management
- ✅ `src/libprojectM/Renderer/RenderPipelineState.hpp/cpp` - Render pipeline wrapper
- ✅ `src/libprojectM/Renderer/ComputePipelineState.hpp/cpp` - Compute pipeline wrapper
- ✅ `src/libprojectM/Renderer/METAL_ABSTRACTION_LAYERS.md` - Design documentation

**Remaining:**
- ⬜ GLSL → MSL conversion implementation (currently placeholder)
- ⬜ Full preset rendering logic in `RenderFrame()`
- ⬜ Texture file loading with full format support
- ⬜ Performance benchmarking

### Short-term (Week 3-4)

1. ⬜ Implement GLSL → MSL conversion using glslangValidator
2. ⬜ Complete preset rendering logic in MetalRenderer
3. ⬜ Create new preset format specification (JSON)
4. ⬜ Write .milk → JSON converter tool
5. ⬜ Implement Metal compute shaders for audio processing

### Medium-term (Month 2)

1. ⬜ Implement full Metal texture management
2. ⬜ Port all shader compilation to Metal
3. ⬜ Create Swift UI framework for macOS/iOS
4. ⬜ Test with existing .milk presets
5. ⬜ Performance optimization and profiling

### Long-term (Month 3+)

1. ⬜ Full Metal renderer implementation
2. ⬜ Rust audio backend (optional)
3. ⬜ MetalFX integration (super-resolution, tone mapping)
4. ⬜ Dynamic resolution scaling
5. ⬜ Complete documentation and examples

---

## Key Decisions

### Architecture Decisions

1. **Keep C++ core:** Maintain backward compatibility and ecosystem
2. **Metal abstraction layer:** Platform-agnostic API with Metal implementation
3. **Dual preset format:** Support both .milk and new JSON format
4. **Language polyglot:** C++ core, ObjC/Swift for macOS, C for audio

### Technical Decisions

1. **No CMake:** Use custom build system (CMake replacement)
2. **Metal only:** Drop OpenGL support (or keep as fallback)
3. **ProjectM-eval:** Keep as dependency (don't reimplement)
4. **Incremental port:** Port components one at a time

### Metal Implementation Decisions

1. **Mirror OpenGL structure:** Metal abstraction mirrors GLResolver, Shader, TextureManager patterns
2. **GLSL → MSL conversion:** Regex-based initial implementation, glslangValidator for production
3. **Device selection:** Support Default, Integrated, Discrete, CPU strategies
4. **Texture formats:** Start with RGBA8Unorm, expand to support more formats
5. **Shader caching:** Cache compiled pipeline states per preset
6. **Command buffer:** Create per-frame command buffers
7. **Debug support:** Enable Metal debug layer for development

---

## References

- [projectM GitHub](https://github.com/projectM-visualizer/projectm)
- [Milkdrop Wiki](https://milkdropwiki.com)
- [Metal Documentation](https://developer.apple.com/metal/)
- [GLSL to MSL Converter](https://github.com/KhronosGroup/glslang)

---

*Document generated as part of projectM Metal port investigation.*

---

## Additional Architecture Details

### Transition Shaders

Located in `Renderer/TransitionShaders/`, these handle preset transitions:
- `TransitionVertexShaderGlsl330.vert` - Simple pass-through vertex shader
- `TransitionShaderMainGlsl330.frag` - Main transition logic
- Built-in effects: SimpleBlend, Circle, Plasma, Warp, ZoomBlur, Sweep

Example transition shader structure:
```glsl
precision mediump float;
layout(location = 0) in vec2 iPosition;

void main() {
    gl_Position = vec4(iPosition, 0.0, 1.0);
    mainImage(_user_out_color, gl_FragCoord.xy);
}
```

### Preset State (`PresetState`)

Located in `MilkdropPreset/PresetState.hpp`

```cpp
class PresetState {
    // Runtime variables (BlendableFloat = float)
    float decay, waveScale, warpScale, zoom, rot;
    float ob_r, ob_g, ob_b, ob_a;  // Outer border
    float ib_r, ib_g, ib_b, ib_a;  // Inner border
    
    // Code execution
    std::string perFrameInitCode;
    std::string perFrameCode;
    std::string perPixelCode;
    
    // Shaders
    std::string warpShader;
    std::string compositeShader;
    
    // Eval library
    projectm_eval_mem_buffer globalMemory;
    double globalRegisters[100];
    std::array<double, QVarCount> frameQVariables;
    
    // Audio data (per frame)
    Audio::FrameAudioData audioData;
    
    // Render context
    Renderer::RenderContext renderContext;
};
```

### Additional Code Execution Contexts

#### Per-Point Context (Custom Waveforms)
- Runs once per waveform vertex
- Access to: `time`, `x` (previous point), `y` (previous point)
- Modifies: `x`, `y`

### Preset File Format (.milk)

Located in `MilkdropPreset/PresetFileParser.cpp`

```ini
[preset00]
fDecay=0.98
fWarpScale=2.85
zoom=1.0
rot=0.0

per_frame_1=ib_r=0.7+0.4*sin(3*time);
per_frame_2=ib_g=0.7+0.4*sin(4*time);

per_pixel_1=zoom=0.9615-rad*0.1;
```

### Supported Extensions
- `.milk` - Standard Milkdrop preset
- `.prjm` - projectM preset
- `idle://` - Special URL for idle preset
- `file://` - File URLs

### Audio Constants

```cpp
// Audio constants in Audio/AudioConstants.hpp
constexpr int AudioBufferSamples = 576;  // Input buffer size
constexpr int WaveformSamples = 480;     // Waveform samples per frame
constexpr int SpectrumSamples = 512;     // Spectrum bins

using WaveformBuffer = std::array<float, 576>;
using SpectrumBuffer = std::array<float, 512>;
```

### Additional Renderer Components

#### Texture System
- **Texture:** GPU texture management (`Renderer/Texture.hpp`)
- **Sampler:** Texture sampling descriptor (`Renderer/Sampler.hpp`)
- **TextureAttachment:** Framebuffer texture attachment (`Renderer/TextureAttachment.hpp`)
- **Framebuffer:** FBO management with multiple attachments (`Renderer/Framebuffer.hpp`)
- **CopyTexture:** Texture copying and flipping (`Renderer/CopyTexture.hpp`)

#### TextureAttachment Types
- `Color` - Color texture attachment (GL_RGBA)
- `Depth` - Depth buffer (GL_DEPTH_COMPONENT, GL_FLOAT)
- `Stencil` - Stencil buffer (GL_STENCIL_INDEX, GL_UNSIGNED_BYTE)
- `DepthStencil` - Combined depth-stencil (GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8)

#### Framebuffer Features
- Multiple framebuffers (up to 8 color attachments)
- Shared size across all attachments
- Draw buffer ordering: Color attachments → Depth → Stencil → Depth Stencil
- Masking for individual draw buffers

#### User Sprites

Located in `UserSprites/SpriteManager.hpp`:
- Spawn custom sprites with type and data
- Draw sprites with audio data and render context
- Destroy individual or all sprites
- Sprite slots: configurable (default 16)
- Variable access and modification during execution

#### Logging System

Located in `Logging.hpp`:
- Thread-local and global callbacks
- Log levels: Trace, Debug, Information, Warning, Error, Fatal
- Macro-based logging: `LOG_TRACE`, `LOG_DEBUG`, `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`, `LOG_FATAL`
- Default level: Information

---

## Summary of Key Findings

### ProjectM Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│  (Swift/ObjC) - macOS/iOS UI, controls, user interaction   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Metal Abstraction Layer                  │
│  (C++) - Platform-agnostic API, Metal implementation       │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   Core Library Layer                        │
│  (C++) - ProjectM class, presets, audio processing         │
├─────────────────────────────────────────────────────────────┤
│  Audio: PCM, FFT, WaveformAligner, Loudness                │
│  Presets: Factory, Parser, State, Contexts                  │
│  Renderer: ShaderCache, TextureManager, Framebuffer         │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Platform Layer                           │
│  (C) - Audio I/O, OpenGL/Metal resolution                   │
└─────────────────────────────────────────────────────────────┘
```

### Data Flow

```
Audio Input → PCM Buffer → FFT Analysis → FrameAudioData
                                              │
                                              ▼
                                      Preset Evaluation
                                      (Per-Frame + Per-Pixel)
                                              │
                                              ▼
                                      Shader Rendering
                                      (Warp + Composite)
                                              │
                                              ▼
                                      Output Texture
```

### Key Design Patterns

1. **Factory Pattern:** `PresetFactoryManager` creates presets from files
2. **Observer Pattern:** Event system for preset switching
3. **Strategy Pattern:** Different code execution contexts
4. **Singleton Pattern:** `ShaderCache`, `TextureManager`
5. **Template Method:** `Preset` virtual interface

### Performance Considerations

- **Shader caching:** Compiled shaders stored by key
- **Texture caching:** Loaded textures cached, purged when needed
- **Circular buffers:** Efficient audio buffer management
- **Per-pixel mesh:** High-resolution mesh for smooth effects
- **Frame-based processing:** One frame per audio sample batch

### Dependencies

| Dependency | Purpose | Location |
|------------|---------|----------|
| OpenGL (GLAD) | GPU context, shaders | vendor/glad |
| GLM | Math operations | vendor/glm |
| projectM-eval | Expression evaluation | vendor/projectm-eval |
| stb_image | Image loading | vendor/stb_image |

---

*End of Architecture Investigation Document*

---

## projectM-eval Expression Evaluator Analysis

### Overview

The `vendor/projectm-eval` library is a cross-platform reimplementation of NullSoft's Expression Evaluation Library 2 (ns-eel2). It's used to parse and execute the expression code in Milkdrop presets.

**License:** MIT  
**Location:** `vendor/projectm-eval/`

### Key Components

#### 1. Public API (`api/projectm-eval.h`)

```c
// Context management
struct projectm_eval_context* projectm_eval_context_create(...);
void projectm_eval_context_destroy(struct projectm_eval_context* ctx);

// Variable registration
PRJM_EVAL_F* projectm_eval_context_register_variable(struct projectm_eval_context* ctx, const char* var_name);

// Code compilation
struct projectm_eval_code* projectm_eval_code_compile(struct projectm_eval_context* ctx, const char* code);
void projectm_eval_code_destroy(struct projectm_eval_code* code_handle);

// Execution
PRJM_EVAL_F projectm_eval_code_execute(struct projectm_eval_code* code_handle);

// Error handling
const char* projectm_eval_get_error(struct projectm_eval_context* ctx, int* line, int* column);
```

#### 2. Memory Management

```c
projectm_eval_mem_buffer projectm_eval_memory_buffer_create();
void projectm_eval_memory_buffer_destroy(projectm_eval_mem_buffer buffer);
void projectm_eval_memory_global_destroy();
```

#### 3. Available Functions (from TreeFunctions.h)

**Variable/Constant Access:**
- `const` - Get constant value
- `var` - Get variable value

**Assignment:**
- `set` - Set variable value

**Memory Access:**
- `mem` - Access megabuf/gmegabuf
- `freembuf` - Free memory buffer
- `memcpy` - Memory copy
- `memset` - Memory set

**Operators:**
- Arithmetic: `add`, `sub`, `mul`, `div`, `mod`, `pow`
- Comparison: `equal`, `notequal`, `below`, `above`, `beloweq`, `aboveeq`
- Logical: `bitwise_or`, `bitwise_and`, `boolean_and_op`, `boolean_or_op`
- Negation: `neg`, `bnot`

**Mathematical Functions:**
- Trig: `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`
- Powers: `pow`, `sqrt`, `exp`, `log`, `log10`, `sqr`, `invsqrt`
- Special: `sigmoid`, `abs`, `min`, `max`, `sign`, `rand`, `floor`, `ceil`

**Execution Control:**
- `execute_list` - Execute instruction list
- `execute_loop` - Loop execution
- `execute_while` - While loop
- `if` - Conditional execution
- `exec2`, `exec3` - Execute with arguments

### Expression Syntax Examples

```ini
; Per-frame code
per_frame_1=ib_r=0.7+0.4*sin(3*time);
per_frame_2=ib_g=0.7+0.4*sin(4*time);

; Per-pixel code
per_pixel_1=zoom=0.9615-rad*0.1;

; Custom waveform
per_point_1=y=sin(x*10);
```

### Compile-Time vs Runtime Evaluation

The evaluator supports both compile-time and runtime evaluation:
- **Compile-time:** Functions that can be evaluated to constants (e.g., `sin(0.5)`)
- **Runtime:** Functions that access variables or memory (e.g., `var("time")`)

### Context Lifecycle

```
1. Create memory buffer
2. Create context with memory buffer and global variables
3. Register variables as needed
4. Compile code (can be done multiple times)
5. Execute code (can be done multiple times)
6. Destroy code handles separately
7. Destroy context
8. Destroy memory buffer
```

### Thread Safety

The library requires host-defined lock functions for thread safety:
```c
void projectm_eval_memory_host_lock_mutex();
void projectm_eval_memory_host_unlock_mutex();
```

---

*End of projectM-eval Analysis*
