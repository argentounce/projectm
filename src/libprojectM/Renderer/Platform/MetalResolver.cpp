/**
 * @file MetalResolver.cpp
 * @brief Implementation of Metal device and context resolver.
 */

#include "Renderer/Platform/MetalResolver.hpp"

#include <mutex>
#include <thread>

namespace libprojectM {
namespace Renderer {
namespace Platform {

MetalResolver& MetalResolver::Instance()
{
    static MetalResolver instance;
    return instance;
}

bool MetalResolver::Initialize(DeviceSelector deviceSelector, void* userData)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_loaded)
        return true;

    m_initializing = true;

    try
    {
        EnableDebugLayer();

        DeviceSelection selection = DeviceSelection::Default;
        if (deviceSelector)
            selection = deviceSelector(userData);

        m_state = std::make_shared<MetalBackendState>();
        m_state->device = SelectDevice(selection);

        if (!m_state->device)
        {
            m_state->error = "Failed to select Metal device";
            m_initializing = false;
            m_initCv.notify_all();
            return false;
        }

        m_state->commandQueue = CreateCommandQueue(m_state->device);
        if (!m_state->commandQueue)
        {
            m_state->error = "Failed to create Metal command queue";
            m_initializing = false;
            m_initCv.notify_all();
            return false;
        }

        m_state->capabilities = GetDeviceCapabilities(m_state->device);
        m_state->initialized = true;
        m_loaded = true;

        m_initCv.notify_all();
        return true;
    }
    catch (const std::exception& e)
    {
        m_state->error = e.what();
        m_initializing = false;
        m_initCv.notify_all();
        return false;
    }
}

bool MetalResolver::IsInitialized() const
{
    return m_loaded;
}

id<MTLDevice> MetalResolver::Device() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_loaded && m_state)
        return m_state->device;
    return nullptr;
}

id<MTLCommandQueue> MetalResolver::CommandQueue() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_loaded && m_state)
        return m_state->commandQueue;
    return nullptr;
}

DeviceCapabilities MetalResolver::Capabilities() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_loaded && m_state)
        return m_state->capabilities;
    return DeviceCapabilities{};
}

id<MTLCommandBuffer> MetalResolver::CreateCommandBuffer() const
{
    id<MTLCommandQueue> queue = CommandQueue();
    if (!queue)
        return nullptr;

    return [queue commandBuffer]();
}

std::shared_ptr<class ComputePipelineState> MetalResolver::CreateComputePipelineState(
    id<MTLDevice> device,
    const char* source,
    const std::string& name)
{
    if (!source)
        return nullptr;

    std::string mslSource = ConvertGLSLToMSL(source, "msl_1_2", "compute");
    if (mslSource.empty())
        return nullptr;

    MTLComputePipelineDescriptor* descriptor = [[MTLComputePipelineDescriptor alloc] init];
    descriptor.computationFunction = __builtin_stdlib_cast_symbol(MTLComputeFunctionType, source);

    descriptor.computePipelineStateOptions = MTLComputePipelineStateOptionNone;
    descriptor.threadMode = MTLThreadModeDispatchQueue;

    NSError* error = nil;
    id<MTLComputePipelineState> pipeline = [device makeComputePipelineStateWithDescriptor:descriptor error:&error];

    if (!pipeline)
    {
        return nullptr;
    }

    return std::shared_ptr<class ComputePipelineState>(pipeline, [](id<MTLComputePipelineState> p) {
        [p release];
    });
}

std::shared_ptr<class RenderPipelineState> MetalResolver::CreateRenderPipelineState(
    id<MTLDevice> device,
    const char* vertexSource,
    const char* fragmentSource,
    const std::string& name)
{
    if (!vertexSource || !fragmentSource)
        return nullptr;

    std::string vertexMSL = ConvertGLSLToMSL(vertexSource, "msl_1_2", "vertex");
    std::string fragmentMSL = ConvertGLSLToMSL(fragmentSource, "msl_1_2", "fragment");

    if (vertexMSL.empty() || fragmentMSL.empty())
        return nullptr;

    MTLRenderPipelineDescriptor* descriptor = [[MTLRenderPipelineDescriptor alloc] init];
    descriptor.vertexFunction = __builtin_stdlib_cast_symbol(MTLVertexFunctionType, vertexSource);
    descriptor.fragmentFunction = __builtin_stdlib_cast_symbol(MTLFragmentFunctionType, fragmentSource);

    descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;
    descriptor.colorAttachments[0].isBlendingEnabled = YES;
    descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdditive;
    descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdditive;
    descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    descriptor.colorAttachments[0].destRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    descriptor.colorAttachments[0].destAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

    descriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    descriptor.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float;

    NSError* error = nil;
    id<MTLRenderPipelineState> pipeline = [device makeRenderPipelineStateWithDescriptor:descriptor error:&error];

    if (!pipeline)
    {
        return nullptr;
    }

    return std::shared_ptr<class RenderPipelineState>(pipeline, [](id<MTLRenderPipelineState> p) {
        [p release];
    });
}

bool MetalResolver::VerifyBeforeUse(std::string& reason) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_loaded || !m_state)
    {
        reason = "Metal resolver not initialized";
        return false;
    }

    if (!m_state->initialized)
    {
        reason = "Metal device not initialized";
        return false;
    }

    return true;
}

id<MTLDevice> MetalResolver::SelectDevice(DeviceSelection strategy)
{
    NSArray<MTLDevice*>* devices = [MTLDevice devices];

    if (strategy == DeviceSelection::ForceCPU)
    {
        return nil;
    }

    if (strategy == DeviceSelection::Default)
    {
        return [MTLDevice defaultDevice];
    }

    MTLDevice* bestDevice = nil;

    for (MTLDevice* device in devices)
    {
        if (strategy == DeviceSelection::PreferIntegrated &&
            device.deviceType == MTLDeviceTypeIntegratedGPU)
        {
            bestDevice = device;
            break;
        }

        if (strategy == DeviceSelection::PreferDiscrete &&
            device.deviceType == MTLDeviceTypeGPU)
        {
            bestDevice = device;
        }
    }

    if (!bestDevice)
    {
        bestDevice = [MTLDevice defaultDevice];
    }

    return (__bridge id<MTLDevice>)bestDevice;
}

id<MTLCommandQueue> MetalResolver::CreateCommandQueue(id<MTLDevice> device)
{
    return [device newCommandQueue];
}

DeviceCapabilities MetalResolver::GetDeviceCapabilities(id<MTLDevice> device)
{
    DeviceCapabilities caps;
    caps.deviceType = device.deviceType;
    caps.supportsCompute = device.supportsCompute;
    caps.supportsDepthStencil = device.supportsDepthStencil;
    caps.supportsMultisample = device.supportsMultisample;
    caps.supportsIndirectCommandBuffers = device.supportsIndirectCommandBuffers;
    caps.maxComputeThreads = device.maxComputeThreadsPerThreadgroup;
    caps.maxComputeThreadgroupsPerGrid = device.maxComputeThreadgroupsPerGrid;
    caps.maxTextureSize2D = device.maxTextureSize2D;
    caps.maxTextureArraySize = device.maxTextureArraySize;
    caps.maxSamplerAnisotropy = device.maxSamplerAnisotropy;
    caps.maxVertexUniformBufferRange = device.maxVertexUniformBufferRange;
    caps.maxFragmentUniformBufferRange = device.maxFragmentUniformBufferRange;
    caps.maxStorageBufferRange = device.maxStorageBufferRange;
    caps.maxStorageTextureDimension = device.maxStorageTextureDimension;

    return caps;
}

void MetalResolver::EnableDebugLayer()
{
#ifdef __APPLE__
    MTLDebugCommandEncoder* encoder = [MTLDebugCommandEncoder new];
    [encoder encodeCommandBuffer:[MTLCommandBuffer new]];
#endif
}

std::string MetalResolver::ConvertGLSLToMSL(const std::string& glsl,
                                           const std::string& version,
                                           const std::string& profile)
{
    // This is a placeholder - in production this would use glslang or similar
    // For now, return the source as-is with a note that conversion is needed
    return glsl;
}

} // namespace Platform
} // namespace Renderer
} // namespace libprojectM
