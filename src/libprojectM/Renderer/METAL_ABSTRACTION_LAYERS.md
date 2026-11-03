# Metal Abstraction Layer Design

## Overview

This document describes the Metal abstraction layer for projectM, designed to mirror the existing OpenGL renderer architecture while providing a clean separation between platform-specific Metal code and the core projectM library.

## Architecture

```
projectM Core (C++)
├── ProjectM.hpp
├── Audio/
├── MilkdropPreset/
└── Renderer/
    ├── Platform/
    │   ├── MetalResolver.hpp/cpp    # Metal device/context resolver
    │   └── GLResolver.hpp/cpp       # Existing OpenGL resolver
    ├── MetalRenderer.hpp/cpp        # Main Metal renderer
    ├── MetalShader.hpp/cpp          # Shader compilation (GLSL→MSL)
    ├── MetalTexture.hpp/cpp         # Texture management
    ├── ShaderCache.hpp/cpp          # Shader caching
    ├── TextureManager.hpp/cpp       # Texture management
    ├── RenderPipelineState.hpp/cpp  # MTLRenderPipelineState wrapper
    ├── ComputePipelineState.hpp/cpp # MTLComputePipelineState wrapper
    └── OpenGL.hpp/cpp               # Existing OpenGL renderer
```

## Key Components

### 1. MetalResolver (Platform Abstraction)

**File:** `src/libprojectM/Renderer/Platform/MetalResolver.hpp`

Purpose: Provides a unified interface for Metal device selection and command queue management.

**Key Features:**
- Device selection strategies (Default, Integrated, Discrete, CPU)
- Command queue management
- Device capabilities query
- Thread-safe singleton pattern
- Debug layer support

**Environment Variables:**
- `PROJECTM_METAL_DEVICE_SELECTION` - Control device selection
- `PROJECTM_METAL_DEBUG` - Enable Metal debug layer

### 2. MetalRenderer (Main Renderer)

**File:** `src/libprojectM/Renderer/MetalRenderer.hpp`

Purpose: Main rendering engine that manages Metal resources and executes preset rendering.

**Key Features:**
- Frame rendering
- Texture management
- Shader management
- Viewport and aspect ratio handling
- Preset path management
- Texture loading and caching

**Methods:**
- `Initialize()` - Initialize Metal renderer
- `RenderFrame()` - Render a single frame
- `SetViewportSize()` - Set rendering viewport
- `SetAspectRatio()` - Set aspect ratio
- `LoadTexture()` / `GetTexture()` - Texture operations
- `GetDevice()` / `GetCommandQueue()` - Access Metal resources

### 3. MetalShader (Shader Compilation)

**File:** `src/libprojectM/Renderer/MetalShader.hpp`

Purpose: Handles GLSL to MSL conversion and shader pipeline state creation.

**Key Features:**
- GLSL → MSL conversion utilities
- Shader cache management
- Pipeline state factory
- Vertex descriptor generation

**Conversion Categories:**
- Version conversion (GLSL 1.50/3.30/4.50 → MSL 1.2/2.0)
- Precision qualifiers (lowp/mediump/highp)
- Built-in variables (gl_FragColor, gl_Position)
- Type qualifiers (uniform, varying, in/out)
- Array syntax
- Texture functions
- Lighting functions
- Matrix functions
- Vector functions
- Math functions

### 4. MetalTexture (Texture Management)

**File:** `src/libprojectM/Renderer/MetalTexture.hpp`

Purpose: Manages Metal texture creation, loading, and operations.

**Key Features:**
- Texture creation from data and files
- Texture caching
- Sampler management
- Texture age tracking for purging
- File scanning

**Supported Formats:**
- JPEG, PNG, TGA, BMP, DDS
- RGBA8Unorm pixel format

## Implementation Status

### ✅ Completed

1. **MetalResolver.hpp/cpp** - Platform abstraction layer
2. **MetalRenderer.hpp/cpp** - Main renderer
3. **MetalShader.hpp/cpp** - Shader compilation
4. **MetalTexture.hpp/cpp** - Texture management
5. **RenderPipelineState.hpp/cpp** - Render pipeline wrapper
6. **ComputePipelineState.hpp/cpp** - Compute pipeline wrapper

### 🔄 In Progress

1. **GLSL → MSL Conversion**
   - Current: Placeholder implementation
   - Needed: Full conversion using glslangValidator or similar

2. **Preset Rendering**
   - Current: Placeholder in `RenderFrame()`
   - Needed: Full preset rendering logic

3. **Texture File Loading**
   - Current: Uses stb_image for basic formats
   - Needed: Full DDS support with mipmaps

### ❌ Not Started

1. **Metal Compute Shaders**
   - Audio processing shaders
   - FFT computation shaders

2. **Performance Optimization**
   - Command buffer batching
   - Texture streaming
   - Memory management

## GLSL to MSL Conversion Strategy

### Option 1: Manual Conversion (Current)

```cpp
std::string MetalShaderCompiler::ConvertToMSL(const std::string& glslSource)
{
    // Regex-based conversion
    // - Version: #version msl_1_2
    // - Types: vec4 → float4
    // - Builtins: gl_FragColor → [[stage_in]](float4)
    // - ...
}
```

**Pros:**
- No external dependencies
- Full control over conversion

**Cons:**
- Error-prone
- Limited by regex capabilities
- Hard to maintain

### Option 2: glslangValidator (Recommended)

```bash
glslangValidator -V input.glsl -o output.msl
```

**Pros:**
- Industry-standard tool
- Handles all GLSL features
- Well-tested

**Cons:**
- External dependency
- Requires subprocess invocation
- Slower for frequent conversions

### Option 3: glslang Runtime

```cpp
// Use glslang library for runtime compilation
glslang::InitializeProcess();
glslang::ShaderMesgSink sink;
glslang::ShaderMesgSink::EShMsgType severity = EShMsgVerbose;
// ... compile shaders ...
glslang::FinalizeProcess();
```

**Pros:**
- Runtime compilation
- No subprocess overhead
- Full GLSL support

**Cons:**
- Larger binary size
- More complex integration
- License considerations

## Recommended Approach

1. **Short-term:** Use manual conversion for simple shaders
2. **Medium-term:** Integrate glslangValidator as external tool
3. **Long-term:** Consider glslang runtime for production use

## Performance Considerations

### Command Buffer Management

```cpp
// Create command buffer per frame
id<MTLCommandBuffer> commandBuffer = m_resolver.CreateCommandBuffer();

// Create render pass
id<MTLRenderPassDescriptor> renderPass = CreateRenderPassDescriptor();

// Encode commands
id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPass];

// ... render ...

[encoder encode];
[commandBuffer commit];
```

### Texture Management

- Use `MTLResourceStorageModeShared` for CPU-accessible textures
- Use `MTLResourceStorageModePrivate` for GPU-only textures
- Implement texture age tracking for memory management
- Consider texture streaming for large texture sets

### Shader Caching

- Cache compiled pipeline states
- Invalidate on preset change
- Use weak references to avoid memory leaks

## Testing Strategy

### Unit Tests

1. MetalResolver device selection
2. MetalShader GLSL→MSL conversion
3. MetalTexture loading and creation
4. RenderPipelineState creation

### Integration Tests

1. Full preset rendering
2. Texture loading from files
3. Shader compilation and execution
4. Frame rendering loop

### Performance Tests

1. Frame time measurement
2. Memory usage monitoring
3. Texture loading speed
4. Shader compilation time

## Migration Path from OpenGL

### Step 1: Dual Renderer Support

```cpp
// Existing OpenGL renderer continues to work
OpenGLRenderer glRenderer(context);

// New Metal renderer
MetalRenderer metalRenderer(context);
```

### Step 2: Platform Detection

```cpp
auto renderer = DetectPlatform() == Platform::Metal
    ? std::make_unique<MetalRenderer>(context)
    : std::make_unique<OpenGLRenderer>(context);
```

### Step 3: Feature Parity

- Ensure Metal renderer supports all OpenGL features
- Test with all existing presets
- Performance benchmarking

### Step 4: Deprecation

- Remove OpenGL renderer after Metal is stable
- Update documentation

## Future Enhancements

1. **Multi-GPU Support**
   - Utilize all available GPUs
   - Split rendering across GPUs

2. **Metal Performance Shaders**
   - Optimize compute shaders
   - Use Metal Performance Shaders (MPS)

3. **MetalFX**
   - Super-resolution
   - Tone mapping
   - Temporal anti-aliasing

4. **Dynamic Resolution**
   - Resolution scaling
   - Frame rate limiting

5. **Shader Hot Reloading**
   - Live shader updates
   - Preset debugging

## References

- [Metal Documentation](https://developer.apple.com/metal/)
- [GLSL to MSL Conversion Guide](https://developer.apple.com/metal/Metal-Feature-Set-Layers/)
- [glslangValidator](https://github.com/KhronosGroup/glslang)
- [projectM OpenGL Renderer](src/libprojectM/Renderer/OpenGL.h)
