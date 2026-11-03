/**
 * @file MetalTexture.hpp
 * @brief Metal texture management and operations.
 * 
 * This module handles Metal texture creation, management, and
 * texture data operations.
 */

#pragma once

#include "Renderer/TextureSamplerDescriptor.hpp"
#include "Renderer/TextureTypes.hpp"
#include "Renderer/Platform/MetalResolver.hpp"

#include <memory>
#include <string>
#include <vector>

namespace libprojectM {
namespace Renderer {

/**
 * @brief Metal texture exception.
 */
class MetalTextureException : public std::exception
{
public:
    MetalTextureException(std::string message)
        : m_message(std::move(message))
    {
    }

    virtual ~MetalTextureException() = default;

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
 * @brief Metal texture manager.
 */
class MetalTextureManager
{
public:
    MetalTextureManager() = default;
    ~MetalTextureManager() = default;

    MetalTextureManager(const MetalTextureManager&) = delete;
    auto operator=(const MetalTextureManager&) -> MetalTextureManager& = delete;
    MetalTextureManager(MetalTextureManager&&) = default;
    auto operator=(MetalTextureManager&&) -> MetalTextureManager& = default;

    /**
     * @brief Constructor with texture search paths.
     */
    explicit MetalTextureManager(const std::vector<std::string>& textureSearchPaths);

    /**
     * @brief Sets the current preset path for texture loading.
     */
    void SetCurrentPresetPath(const std::string& path);

    /**
     * @brief Loads a texture and returns a descriptor.
     */
    auto GetTexture(const std::string& fullName) -> TextureSamplerDescriptor;

    /**
     * @brief Returns a random texture descriptor.
     */
    auto GetRandomTexture(const std::string& randomName) -> TextureSamplerDescriptor;

    /**
     * @brief Returns a sampler for the given name.
     */
    auto GetSampler(const std::string& fullName) -> std::shared_ptr<class Sampler>;

    /**
     * @brief Purges unused textures.
     */
    void PurgeTextures();

    /**
     * @brief Sets a callback for loading textures from non-filesystem sources.
     */
    void SetTextureLoadCallback(TextureLoadCallback callback);

    /**
     * @brief Scans for textures in directories.
     */
    void ScanTextures();

    /**
     * @brief Returns the number of loaded textures.
     */
    auto GetTextureCount() const -> size_t;

    /**
     * @brief Returns all loaded texture names.
     */
    auto GetTextureNames() const -> std::vector<std::string>;

private:
    /**
     * @brief Tries to load a texture by name.
     */
    auto TryLoadingTexture(const std::string& name) -> TextureSamplerDescriptor;

    /**
     * @brief Loads a texture from a scanned file.
     */
    auto LoadTexture(const Platform::ScannedFile& file) -> std::shared_ptr<Texture>;

    /**
     * @brief Adds a texture file to the scan list.
     */
    void AddTextureFile(const std::string& fileName, const std::string& baseName);

    /**
     * @brief Extracts texture settings from a qualified name.
     */
    static void ExtractTextureSettings(const std::string& qualifiedName,
                                       GLint& wrapMode,
                                       GLint& filterMode,
                                       std::string& name);

    /**
     * @brief Converts texture channels to format.
     */
    static uint32_t TextureFormatFromChannels(int channels);

    /**
     * @brief Creates a Metal texture from data.
     */
    auto CreateTextureFromData(id<MTLDevice> device,
                              const void* data,
                              size_t width,
                              size_t height,
                              size_t depth,
                              size_t arraySize,
                              MTLPixelFormat pixelFormat) -> std::shared_ptr<Texture>;

    /**
     * @brief Loads a texture from file.
     */
    auto LoadTextureFromFile(const std::string& path) -> std::shared_ptr<Texture>;

    /**
     * @brief Creates a sampler for a texture.
     */
    auto CreateSampler(GLint wrapMode, GLint filterMode) -> std::shared_ptr<Sampler>;

    std::vector<std::string> m_textureSearchPaths;
    std::string m_currentPresetDir;
    std::vector<Platform::ScannedFile> m_scannedTextureFiles;
    bool m_filesScanned{false};
    std::shared_ptr<Texture> m_placeholderTexture;
    std::map<std::string, std::shared_ptr<Texture>> m_textures;
    std::map<std::pair<GLint, GLint>, std::shared_ptr<Sampler>> m_samplers;
    std::map<std::string, Platform::UsageStats> m_textureStats;
    std::vector<std::string> m_randomTextures;
    std::vector<std::string> m_extensions{".jpg", ".jpeg", ".dds", ".png", ".tga", ".bmp", ".dib"};
    TextureLoadCallback m_textureLoadCallback;
};

} // namespace Renderer
} // namespace libprojectM
