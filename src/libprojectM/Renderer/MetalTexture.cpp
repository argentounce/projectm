/**
 * @file MetalTexture.cpp
 * @brief Implementation of Metal texture management.
 */

#include "Renderer/MetalTexture.hpp"

#include "Renderer/Sampler.hpp"
#include "Renderer/Texture.hpp"

#include <stb_image/stb_image.h>
#include <cstring>

namespace libprojectM {
namespace Renderer {

MetalTextureManager::MetalTextureManager(const std::vector<std::string>& textureSearchPaths)
    : m_textureSearchPaths(textureSearchPaths)
{
}

void MetalTextureManager::SetCurrentPresetPath(const std::string& path)
{
    m_currentPresetDir = path;
}

TextureSamplerDescriptor MetalTextureManager::GetTexture(const std::string& fullName)
{
    if (m_filesScanned)
    {
        auto it = m_textures.find(fullName);
        if (it != m_textures.end())
        {
            return TextureSamplerDescriptor{
                .texture = it->second->GetTexture(),
                .sampler = CreateSampler(it->second->GetWrapMode(), it->second->GetFilterMode())
            };
        }
    }
    
    if (m_textureLoadCallback)
    {
        return m_textureLoadCallback(fullName);
    }
    
    return TryLoadingTexture(fullName);
}

TextureSamplerDescriptor MetalTextureManager::GetRandomTexture(const std::string& randomName)
{
    if (m_randomTextures.empty())
    {
        ScanTextures();
    }
    
    if (m_randomTextures.empty())
    {
        return TextureSamplerDescriptor{};
    }
    
    std::string selected = m_randomTextures[std::rand() % m_randomTextures.size()];
    return GetTexture(selected);
}

std::shared_ptr<Sampler> MetalTextureManager::GetSampler(const std::string& fullName)
{
    GLint wrapMode, filterMode;
    std::string name;
    
    ExtractTextureSettings(fullName, wrapMode, filterMode, name);
    
    auto it = m_samplers.find(std::make_pair(wrapMode, filterMode));
    if (it != m_samplers.end())
    {
        return it->second;
    }
    
    return CreateSampler(wrapMode, filterMode);
}

void MetalTextureManager::PurgeTextures()
{
    for (auto& pair : m_textures)
    {
        pair.second->SetAge(pair.second->GetAge() + 1);
    }
    
    m_filesScanned = false;
}

void MetalTextureManager::SetTextureLoadCallback(TextureLoadCallback callback)
{
    m_textureLoadCallback = callback;
}

void MetalTextureManager::ScanTextures()
{
    m_scannedTextureFiles.clear();
    
    for (const auto& path : m_textureSearchPaths)
    {
        // TODO: Scan directory for texture files
        // This is a placeholder
    }
    
    if (m_currentPresetDir.empty())
    {
        return;
    }
    
    // TODO: Scan preset directory for texture files
    // This is a placeholder
    
    m_filesScanned = true;
}

size_t MetalTextureManager::GetTextureCount() const
{
    return m_textures.size();
}

std::vector<std::string> MetalTextureManager::GetTextureNames() const
{
    std::vector<std::string> names;
    for (const auto& pair : m_textures)
    {
        names.push_back(pair.first);
    }
    return names;
}

TextureSamplerDescriptor MetalTextureManager::TryLoadingTexture(const std::string& name)
{
    auto it = m_textures.find(name);
    if (it != m_textures.end())
    {
        return TextureSamplerDescriptor{
            .texture = it->second->GetTexture(),
            .sampler = CreateSampler(it->second->GetWrapMode(), it->second->GetFilterMode())
        };
    }
    
    return TextureSamplerDescriptor{};
}

std::shared_ptr<Texture> MetalTextureManager::LoadTexture(const Platform::ScannedFile& file)
{
    auto texture = LoadTextureFromFile(file.filePath);
    if (texture)
    {
        texture->SetName(file.lowerCaseBaseName);
    }
    return texture;
}

void MetalTextureManager::AddTextureFile(const std::string& fileName, const std::string& baseName)
{
    m_scannedTextureFiles.push_back({fileName, baseName});
}

void MetalTextureManager::ExtractTextureSettings(const std::string& qualifiedName,
                                                 GLint& wrapMode,
                                                 GLint& filterMode,
                                                 std::string& name)
{
    // Extract settings from texture name like "texture_name_wrap_clamp_filter_linear"
    std::string cleanName = qualifiedName;
    
    // Remove wrap mode
    size_t wrapPos = cleanName.find("_wrap_");
    if (wrapPos != std::string::npos)
    {
        cleanName = cleanName.substr(0, wrapPos) + cleanName.substr(wrapPos + 6);
    }
    
    // Remove filter mode
    size_t filterPos = cleanName.find("_filter_");
    if (filterPos != std::string::npos)
    {
        cleanName = cleanName.substr(0, filterPos) + cleanName.substr(filterPos + 8);
    }
    
    name = cleanName;
    
    // Default settings
    wrapMode = GL_REPEAT;
    filterMode = GL_LINEAR;
}

uint32_t MetalTextureManager::TextureFormatFromChannels(int channels)
{
    switch (channels)
    {
        case 1:
            return 1;
        case 2:
            return 2;
        case 3:
            return 3;
        case 4:
            return 4;
        default:
            return 4;
    }
}

std::shared_ptr<Texture> MetalTextureManager::CreateTextureFromData(id<MTLDevice> device,
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

std::shared_ptr<Texture> MetalTextureManager::LoadTextureFromFile(const std::string& path)
{
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    
    if (!data)
    {
        return nullptr;
    }
    
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

std::shared_ptr<Sampler> MetalTextureManager::CreateSampler(GLint wrapMode, GLint filterMode)
{
    MTLSamplerDescriptor* descriptor = [[MTLSamplerDescriptor alloc] init];
    
    switch (wrapMode)
    {
        case GL_CLAMP_TO_EDGE:
            descriptor.addressModeS = MTLTextureAddressModeClampToEdge;
            descriptor.addressModeT = MTLTextureAddressModeClampToEdge;
            break;
        case GL_REPEAT:
        default:
            descriptor.addressModeS = MTLTextureAddressModeRepeat;
            descriptor.addressModeT = MTLTextureAddressModeRepeat;
            break;
    }
    
    switch (filterMode)
    {
        case GL_NEAREST:
            descriptor.minMagFilter = MTLTextureFilterNearest;
            break;
        case GL_LINEAR:
        default:
            descriptor.minMagFilter = MTLTextureFilterLinear;
            break;
    }
    
    descriptor.mipFilter = MTLTextureFilterNearest;
    descriptor.maxAnisotropy = 16.0f;
    
    return std::make_shared<Sampler>(descriptor, wrapMode, filterMode);
}

} // namespace Renderer
} // namespace libprojectM
