/**
 * @file MetalRenderer.hpp
 * @brief Metal-based renderer for projectM.
 * 
 * This is the Metal equivalent of the OpenGL renderer, providing
 * a unified interface for Metal-based rendering operations.
 */

#pragma once

#include "Renderer/RenderContext.hpp"
#include "Renderer/Platform/MetalResolver.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x4.hpp>
#include <glm/mat4x4.hpp>

#include <memory>
#include <string>
#include <vector>

namespace libprojectM {
namespace Renderer {

/**
 * @brief Metal renderer exception.
 */
class MetalRendererException : public std::exception
{
public:
    MetalRendererException(std::string message)
        : m_message(std::move(message))
    {
    }

    virtual ~MetalRendererException() = default;

    const char* what() const noexcept override
    {
        return m_message.c_str();
    }

    const std::string& message() const
    {
        return m_message;
    }

private:
    std::string m_message;
};

/**
 * @brief Metal texture representation.
 */
class MetalTexture
{
public:
    MetalTexture() = default;
    ~MetalTexture() = default;

    MetalTexture(const MetalTexture&) = delete;
    auto operator=(const MetalTexture&) -> MetalTexture& = delete;
    MetalTexture(MetalTexture&&) = default;
    auto operator=(MetalTexture&&) -> MetalTexture& = default;

    /**
     * @brief Creates a texture from a Metal texture descriptor.
     */
    void CreateTexture(id<MTLTexture> texture,
                      MTLTextureType type,
                      size_t width,
                      size_t height,
                      size_t depth,
                      size_t arraySize,
                      MTLPixelFormat pixelFormat,
                      bool mipmapped = false);

    /**
     * @brief Returns the Metal texture.
     */
    auto GetTexture() const -> id<MTLTexture>;

    /**
     * @brief Returns the texture width.
     */
    auto GetWidth() const -> size_t;

    /**
     * @brief Returns the texture height.
     */
    auto GetHeight() const -> size_t;

    /**
     * @brief Returns the texture depth.
     */
    auto GetDepth() const -> size_t;

    /**
     * @brief Returns the texture array size.
     */
    auto GetArraySize() const -> size_t;

    /**
     * @brief Returns the pixel format.
     */
    auto GetPixelFormat() const -> MTLPixelFormat;

    /**
     * @brief Returns whether the texture is mipmapped.
     */
    auto IsMipmapped() const -> bool;

    /**
     * @brief Updates the texture with new data.
     */
    void UpdateData(const void* data, size_t size);

    /**
     * @brief Destroys the texture.
     */
    void Destroy();

private:
    id<MTLTexture> m_texture{nullptr};
    MTLTextureType m_type{MTLTextureType2D};
    size_t m_width{0};
    size_t m_height{0};
    size_t m_depth{0};
    size_t m_arraySize{0};
    MTLPixelFormat m_pixelFormat{MTLPixelFormatRGBA8Unorm};
    bool m_mipmapped{false};
};

/**
 * @brief Metal shader program.
 */
class MetalShader
{
public:
    MetalShader() = default;
    ~MetalShader() = default;

    MetalShader(const MetalShader&) = delete;
    auto operator=(const MetalShader&) -> MetalShader& = delete;
    MetalShader(MetalShader&&) = default;
    auto operator=(MetalShader&&) -> MetalShader& = default;

    /**
     * @brief Compiles a vertex and fragment shader into a pipeline state.
     * @throws MetalRendererException Thrown if compilation failed.
     */
    void CompileProgram(id<MTLDevice> device,
                       const std::string& vertexSource,
                       const std::string& fragmentSource,
                       const std::string& name);

    /**
     * @brief Returns the render pipeline state.
     */
    auto GetRenderPipelineState() const -> std::shared_ptr<class RenderPipelineState>;

    /**
     * @brief Returns the compute pipeline state (if any).
     */
    auto GetComputePipelineState() const -> std::shared_ptr<class ComputePipelineState>;

    /**
     * @brief Sets a single float uniform.
     * The pipeline must be bound before calling this method.
     */
    void SetUniformFloat(const char* uniform, float value) const;

    /**
     * @brief Sets a single integer uniform.
     */
    void SetUniformInt(const char* uniform, int value) const;

    /**
     * @brief Sets a float vec2 uniform.
     */
    void SetUniformFloat2(const char* uniform, const glm::vec2& values) const;

    /**
     * @brief Sets an int vec2 uniform.
     */
    void SetUniformInt2(const char* uniform, const glm::ivec2& values) const;

    /**
     * @brief Sets a float vec3 uniform.
     */
    void SetUniformFloat3(const char* uniform, const glm::vec3& values) const;

    /**
     * @brief Sets an int vec3 uniform.
     */
    void SetUniformInt3(const char* uniform, const glm::ivec3& values) const;

    /**
     * @brief Sets a float vec4 uniform.
     */
    void SetUniformFloat4(const char* uniform, const glm::vec4& values) const;

    /**
     * @brief Sets an int vec4 uniform.
     */
    void SetUniformInt4(const char* uniform, const glm::ivec4& values) const;

    /**
     * @brief Sets a float 4x4 matrix uniform.
     */
    void SetUniformMat4x4(const char* uniform, const glm::mat4x4& values) const;

    /**
     * @brief Binds the shader pipeline.
     */
    void Bind() const;

    /**
     * @brief Unbinds the shader pipeline.
     */
    static void Unbind();

private:
    std::shared_ptr<class RenderPipelineState> m_renderPipeline{nullptr};
    std::shared_ptr<class ComputePipelineState> m_computePipeline{nullptr};
    std::map<std::string, std::vector<MTLBuffer*>> m_uniformBuffers;
};

/**
 * @brief Metal vertex array.
 */
class MetalVertexArray
{
public:
    MetalVertexArray() = default;
    ~MetalVertexArray() = default;

    MetalVertexArray(const MetalVertexArray&) = delete;
    auto operator=(const MetalVertexArray&) -> MetalVertexArray& = delete;
    MetalVertexArray(MetalVertexArray&&) = default;
    auto operator=(MetalVertexArray&&) -> MetalVertexArray& = default;

    /**
     * @brief Creates a vertex array from buffer and descriptor.
     */
    void CreateArray(id<MTLDevice> device,
                    id<MTLBuffer> buffer,
                    const MTLVertexDescriptor& descriptor,
                    const std::vector<id<MTLBuffer>>& buffers);

    /**
     * @brief Returns the vertex array.
     */
    auto GetVertexArray() const -> id<MTLVertexDescriptor>;

    /**
     * @brief Returns the vertex buffer.
     */
    auto GetVertexBuffer() const -> id<MTLBuffer>;

private:
    id<MTLVertexDescriptor> m_descriptor{nullptr};
    id<MTLBuffer> m_buffer{nullptr};
    std::vector<id<MTLBuffer>> m_buffers;
};

/**
 * @brief Metal framebuffer.
 */
class MetalFramebuffer
{
public:
    MetalFramebuffer() = default;
    ~MetalFramebuffer() = default;

    MetalFramebuffer(const MetalFramebuffer&) = delete;
    auto operator=(const MetalFramebuffer&) -> MetalFramebuffer& = delete;
    MetalFramebuffer(MetalFramebuffer&&) = default;
    auto operator=(MetalFramebuffer&&) -> MetalFramebuffer& = default;

    /**
     * @brief Creates a framebuffer with a color attachment.
     */
    void CreateFramebuffer(id<MTLDevice> device,
                          id<MTLTexture> colorTexture,
                          id<MTLTexture> depthStencilTexture = nullptr);

    /**
     * @brief Returns the color attachment texture.
     */
    auto GetColorAttachment() const -> id<MTLTexture>;

    /**
     * @brief Returns the depth stencil attachment.
     */
    auto GetDepthStencilAttachment() const -> id<MTLTexture>;

    /**
     * @brief Destroys the framebuffer.
     */
    void Destroy();

private:
    id<MTLRenderPassDescriptor> m_renderPass{nullptr};
    id<MTLTexture> m_colorAttachment{nullptr};
    id<MTLTexture> m_depthStencilAttachment{nullptr};
};

/**
 * @brief Metal-based renderer.
 */
class MetalRenderer
{
public:
    /**
     * @brief Constructor.
     * @param context The render context containing shared resources.
     */
    explicit MetalRenderer(RenderContext& context);

    ~MetalRenderer() = default;

    MetalRenderer(const MetalRenderer&) = delete;
    auto operator=(const MetalRenderer&) -> MetalRenderer& = delete;
    MetalRenderer(MetalRenderer&&) = default;
    auto operator=(MetalRenderer&&) -> MetalRenderer& = default;

    /**
     * @brief Initializes the Metal renderer.
     */
    auto Initialize() -> bool;

    /**
     * @brief Renders a single frame.
     */
    void RenderFrame();

    /**
     * @brief Sets the viewport size.
     */
    void SetViewportSize(int width, int height);

    /**
     * @brief Sets the aspect ratio.
     */
    void SetAspectRatio(float aspectX, float aspectY);

    /**
     * @brief Sets the blend progress for transitions.
     */
    void SetBlendProgress(float progress);

    /**
     * @brief Sets the per-pixel mesh resolution.
     */
    void SetPerPixelMeshResolution(int meshX, int meshY);

    /**
     * @brief Sets the texel offset for warp shaders.
     */
    void SetTexelOffset(float offsetX, float offsetY);

    /**
     * @brief Sets the current preset path for texture loading.
     */
    void SetCurrentPresetPath(const std::string& path);

    /**
     * @brief Loads a texture.
     */
    auto LoadTexture(const std::string& name) -> MetalTexture;

    /**
     * @brief Gets a cached texture.
     */
    auto GetTexture(const std::string& name) -> MetalTexture;

    /**
     * @brief Gets a random texture.
     */
    auto GetRandomTexture(const std::string& prefix) -> MetalTexture;

    /**
     * @brief Gets a sampler.
     */
    auto GetSampler(const std::string& name) -> std::shared_ptr<class Sampler>;

    /**
     * @brief Purges unused textures.
     */
    void PurgeTextures();

    /**
     * @brief Sets a callback for loading textures from non-filesystem sources.
     */
    void SetTextureLoadCallback(TextureLoadCallback callback);

    /**
     * @brief Returns the Metal device.
     */
    auto GetDevice() const -> id<MTLDevice>;

    /**
     * @brief Returns the command queue.
     */
    auto GetCommandQueue() const -> id<MTLCommandQueue>;

private:
    /**
     * @brief Creates a command buffer.
     */
    auto CreateCommandBuffer() -> id<MTLCommandBuffer>;

    /**
     * @brief Creates a render pass descriptor.
     */
    auto CreateRenderPassDescriptor() -> id<MTLRenderPassDescriptor>;

    /**
     * @brief Creates a vertex buffer.
     */
    auto CreateVertexBuffer(id<MTLDevice> device,
                           const void* data,
                           size_t size) -> id<MTLBuffer>;

    /**
     * @brief Creates a uniform buffer.
     */
    auto CreateUniformBuffer(id<MTLDevice> device,
                            const void* data,
                            size_t size) -> id<MTLBuffer>;

    /**
     * @brief Converts GLSL to MSL.
     */
    static std::string ConvertGLSLToMSL(const std::string& glsl);

    /**
     * @brief Compiles a MSL shader source.
     */
    static std::string CompileMSLShader(id<MTLDevice> device,
                                       const std::string& source,
                                       MSLShaderType type,
                                       const std::string& name);

    /**
     * @brief Creates a shader from source.
     */
    auto CreateShaderFromSource(const std::string& vertexSource,
                               const std::string& fragmentSource,
                               const std::string& name) -> MetalShader;

    /**
     * @brief Creates a texture from data.
     */
    auto CreateTextureFromData(id<MTLDevice> device,
                              const void* data,
                              size_t width,
                              size_t height,
                              size_t depth,
                              size_t arraySize,
                              MTLPixelFormat pixelFormat) -> MetalTexture;

    /**
     * @brief Creates a texture from file.
     */
    auto LoadTextureFromFile(const std::string& path) -> MetalTexture;

    /**
     * @brief Scans for textures in a directory.
     */
    void ScanTextures(const std::vector<std::string>& paths);

    /**
     * @brief Preloads textures.
     */
    void PreloadTextures();

    /**
     * @brief Increments texture age.
     */
    void IncrementTextureAge();

    RenderContext& m_context;
    Platform::MetalResolver& m_resolver;
    std::unique_ptr<TextureManager> m_textureManager;
    std::unique_ptr<ShaderCache> m_shaderCache;
    std::vector<std::string> m_textureSearchPaths;
    std::string m_currentPresetDir;
    std::vector<Platform::ScannedFile> m_scannedTextureFiles;
    bool m_filesScanned{false};
    std::shared_ptr<Texture> m_placeholderTexture;
    std::map<std::pair<GLint, GLint>, std::shared_ptr<Sampler>> m_samplers;
    std::map<std::string, Platform::UsageStats> m_textureStats;
    std::vector<std::string> m_randomTextures;
    TextureLoadCallback m_textureLoadCallback;
};

} // namespace Renderer
} // namespace libprojectM
