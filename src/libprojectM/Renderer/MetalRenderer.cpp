/**
 * @file MetalRenderer.cpp
 * @brief Implementation of Metal-based renderer.
 */

#include "Renderer/MetalRenderer.hpp"

#include "Renderer/MetalShader.hpp"
#include "Renderer/MetalTexture.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/Sampler.hpp"

#include <stb_image/stb_image.h>
#include <cstring>

namespace libprojectM {
namespace Renderer {

MetalRenderer::MetalRenderer(RenderContext& context)
    : m_context(context)
    , m_resolver(Platform::MetalResolver::Instance())
{
}

bool MetalRenderer::Initialize()
{
    if (!m_resolver.Initialize())
        return false;

    m_textureManager = std::make_unique<TextureManager>(m_context.textureSearchPaths);
    m_shaderCache = std::make_unique<ShaderCache>();

    return true;
}

void MetalRenderer::RenderFrame()
{
    id<MTLCommandBuffer> commandBuffer = m_resolver.CreateCommandBuffer();
    if (!commandBuffer)
        return;

    id<MTLRenderPassDescriptor> renderPass = CreateRenderPassDescriptor();
    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPass];

    // Clear depth/stencil
    [encoder clearDepthStencilTexture:renderPass.depthAttachmentTexture
                              depth:1.0f
                            stencil:0
                              ops:MTLDepthStencilOperations{
                                  .depth = MTLComparisonOperationLess,
                                  .stencil = MTLComparisonOperationAlways
                              }];

    // Render preset
    // TODO: Implement preset rendering logic

    [encoder encode];
    [commandBuffer commit];
}

void MetalRenderer::SetViewportSize(int width, int height)
{
    m_context.viewportSizeX = width;
    m_context.viewportSizeY = height;
}

void MetalRenderer::SetAspectRatio(float aspectX, float aspectY)
{
    m_context.aspectX = aspectX;
    m_context.aspectY = aspectY;
    m_context.invAspectX = 1.0f / aspectX;
    m_context.invAspectY = 1.0f / aspectY;
}

void MetalRenderer::SetBlendProgress(float progress)
{
    m_context.blendProgress = progress;
}

void MetalRenderer::SetPerPixelMeshResolution(int meshX, int meshY)
{
    m_context.perPixelMeshX = meshX;
    m_context.perPixelMeshY = meshY;
}

void MetalRenderer::SetTexelOffset(float offsetX, float offsetY)
{
    m_context.texelOffsetX = offsetX;
    m_context.texelOffsetY = offsetY;
}

void MetalRenderer::SetCurrentPresetPath(const std::string& path)
{
    m_currentPresetDir = path;
    m_textureManager->SetCurrentPresetPath(path);
}

MetalTexture MetalRenderer::LoadTexture(const std::string& name)
{
    return m_textureManager->GetTexture(name);
}

MetalTexture MetalRenderer::GetTexture(const std::string& name)
{
    return m_textureManager->GetTexture(name);
}

MetalTexture MetalRenderer::GetRandomTexture(const std::string& prefix)
{
    return m_textureManager->GetRandomTexture(prefix);
}

std::shared_ptr<Sampler> MetalRenderer::GetSampler(const std::string& name)
{
    return m_textureManager->GetSampler(name);
}

void MetalRenderer::PurgeTextures()
{
    m_textureManager->PurgeTextures();
}

void MetalRenderer::SetTextureLoadCallback(TextureLoadCallback callback)
{
    m_textureLoadCallback = callback;
    m_textureManager->SetTextureLoadCallback(callback);
}

id<MTLDevice> MetalRenderer::GetDevice() const
{
    return m_resolver.Device();
}

id<MTLCommandQueue> MetalRenderer::GetCommandQueue() const
{
    return m_resolver.CommandQueue();
}

id<MTLCommandBuffer> MetalRenderer::CreateCommandBuffer()
{
    return m_resolver.CreateCommandBuffer();
}

id<MTLRenderPassDescriptor> MetalRenderer::CreateRenderPassDescriptor()
{
    MTLRenderPassDescriptor* descriptor = [[MTLRenderPassDescriptor alloc] init];
    descriptor.colorAttachments[0].textureType = MTLTextureType2D;
    descriptor.colorAttachments[0].loadAction = MTLTextureLoadActionClear;
    descriptor.colorAttachments[0].storeAction = MTLTextureStoreActionStore;
    descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;
    descriptor.colorAttachments[0].isBlendingEnabled = YES;

    descriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    descriptor.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float;

    return descriptor;
}

id<MTLBuffer> MetalRenderer::CreateVertexBuffer(id<MTLDevice> device,
                                               const void* data,
                                               size_t size)
{
    MTLBuffer* buffer = [device newBufferWithData:
                        __builtin_stdlib_cast_data(data)
                        options:MTLResourceStorageModeShared
                           length:size];
    return (__bridge id<MTLBuffer>)buffer;
}

id<MTLBuffer> MetalRenderer::CreateUniformBuffer(id<MTLDevice> device,
                                                const void* data,
                                                size_t size)
{
    MTLBuffer* buffer = [device newBufferWithData:
                        __builtin_stdlib_cast_data(data)
                        options:MTLResourceStorageModePrivate
                           length:size];
    return (__bridge id<MTLBuffer>)buffer;
}

std::string MetalRenderer::ConvertGLSLToMSL(const std::string& glsl)
{
    // TODO: Implement GLSL to MSL conversion
    // This would typically use glslangValidator or similar tool
    return glsl;
}

std::string MetalRenderer::CompileMSLShader(id<MTLDevice> device,
                                           const std::string& source,
                                           MSLShaderType type,
                                           const std::string& name)
{
    // TODO: Implement MSL shader compilation
    return source;
}

MetalShader MetalRenderer::CreateShaderFromSource(const std::string& vertexSource,
                                                  const std::string& fragmentSource,
                                                  const std::string& name)
{
    MetalShader shader;
    shader.CompileProgram(m_resolver.Device(), vertexSource, fragmentSource, name);
    return shader;
}

std::shared_ptr<Texture> MetalRenderer::CreateTextureFromData(id<MTLDevice> device,
                                                             const void* data,
                                                             size_t width,
                                                             size_t height,
                                                             size_t depth,
                                                             size_t arraySize,
                                                             MTLPixelFormat pixelFormat)
{
    MTLTextureDescriptor* descriptor = [[MTLTextureDescriptor alloc] init];
    descriptor.type = MTLTextureType2D;
    descriptor.width = width;
    descriptor.height = height;
    descriptor.depth = depth;
    descriptor.arraySize = arraySize;
    descriptor.pixelFormat = pixelFormat;
    descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;

    id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];

    if (data && width > 0 && height > 0)
    {
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        [texture replaceRegion:region
                         mipmap:0
                        withBytes:data
                          bytesPerRow:width * 4];
    }

    return std::make_shared<Texture>(texture,
                                     MTLTextureType2D,
                                     width,
                                     height,
                                     depth,
                                     arraySize,
                                     pixelFormat,
                                     false);
}

std::shared_ptr<Texture> MetalRenderer::LoadTextureFromFile(const std::string& path)
{
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data)
        return nullptr;

    MTLPixelFormat format = MTLPixelFormatRGBA8Unorm;

    std::shared_ptr<Texture> texture = CreateTextureFromData(m_resolver.Device(),
                                                             data,
                                                             width,
                                                             height,
                                                             1,
                                                             1,
                                                             format);

    stbi_image_free(data);
    return texture;
}

void MetalRenderer::ScanTextures(const std::vector<std::string>& paths)
{
    m_textureSearchPaths = paths;
    m_textureManager->ScanTextures();
}

void MetalRenderer::PreloadTextures()
{
    m_textureManager->Preload();
}

void MetalRenderer::IncrementTextureAge()
{
    m_textureManager->PurgeTextures();
}

} // namespace Renderer
} // namespace libprojectM
