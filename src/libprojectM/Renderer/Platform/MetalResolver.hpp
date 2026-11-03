/**
 * @file MetalResolver.hpp
 * @brief Metal device and context resolver for macOS/iOS.
 * 
 * This is the Metal equivalent of GLResolver.hpp, providing a unified
 * interface for Metal device selection and command queue management.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <Metal/Metal.h>

namespace libprojectM {
namespace Renderer {
namespace Platform {

/**
 * @brief Metal device selection strategy.
 */
enum class DeviceSelection : std::uint8_t
{
    /**
     * Use the default device (usually the primary GPU).
     */
    Default = 0,

    /**
     * Prefer integrated GPU if available (power saving).
     */
    PreferIntegrated = 1,

    /**
     * Prefer discrete GPU if available (performance).
     */
    PreferDiscrete = 2,

    /**
     * Force CPU emulation (debugging only).
     */
    ForceCPU = 3
};

/**
 * @brief Metal device capabilities.
 */
struct DeviceCapabilities
{
    MTLDeviceType deviceType{MTLDeviceTypeGPU};
    bool supportsCompute{false};
    bool supportsDepthStencil{false};
    bool supportsMultisample{false};
    bool supportsIndirectCommandBuffers{false};
    size_t maxComputeThreads{0};
    size_t maxComputeThreadgroupsPerGrid{0};
    size_t maxTextureSize2D{0};
    size_t maxTextureArraySize{0};
    size_t maxSamplerAnisotropy{0};
    size_t maxVertexUniformBufferRange{0};
    size_t maxFragmentUniformBufferRange{0};
    size_t maxStorageBufferRange{0};
    size_t maxStorageTextureDimension{0};
};

/**
 * @brief Metal backend state.
 */
struct MetalBackendState
{
    id<MTLDevice> device{nullptr};
    id<MTLCommandQueue> commandQueue{nullptr};
    DeviceCapabilities capabilities;
    bool initialized{false};
    std::string error;
};

/**
 * @brief Optional user device selection callback.
 */
using DeviceSelector = DeviceSelection (*) (void* userData);

/**
 * @brief Universal Metal device and context resolver.
 * 
 * Lifecycle:
 * - The resolver is a process-singleton.
 * - Metal devices are cached and reused across frames.
 * - Command queues are created on demand.
 * 
 * Initialization:
 * - Must be called before any Metal operations.
 * - Thread-safe; intended for startup before rendering begins.
 * 
 * Environment variables:
 * - PROJECTM_METAL_DEVICE_SELECTION=default|integrated|discrete|cpu
 *     Controls device selection strategy.
 * - PROJECTM_METAL_DEBUG=1
 *     Enable Metal debug layer (macOS only).
 */
class MetalResolver
{
public:
    MetalResolver() = default;

    ~MetalResolver() = default;

    MetalResolver(const MetalResolver&) = delete;
    auto operator=(const MetalResolver&) -> MetalResolver& = delete;
    MetalResolver(MetalResolver&&) = delete;
    auto operator=(MetalResolver&&) -> MetalResolver& = delete;

    /**
     * @brief Returns the process-wide resolver instance.
     */
    static auto Instance() -> MetalResolver&;

    /**
     * @brief Initializes the Metal resolver.
     * 
     * @param deviceSelector Optional user device selector callback.
     * @param userData Optional user pointer passed to selector.
     * @return true if Metal was successfully initialized, false otherwise.
     */
    auto Initialize(DeviceSelector deviceSelector = nullptr, void* userData = nullptr) -> bool;

    /**
     * @brief Returns true if the resolver was successfully initialized.
     */
    auto IsInitialized() const -> bool;

    /**
     * @brief Returns the selected Metal device.
     */
    auto Device() const -> id<MTLDevice>;

    /**
     * @brief Returns the command queue for rendering.
     */
    auto CommandQueue() const -> id<MTLCommandQueue>;

    /**
     * @brief Returns the current device capabilities.
     */
    auto Capabilities() const -> DeviceCapabilities;

    /**
     * @brief Creates a new command buffer.
     */
    auto CreateCommandBuffer() const -> id<MTLCommandBuffer>;

    /**
     * @brief Creates a compute pipeline state object.
     */
    auto CreateComputePipelineState(id<MTLDevice> device,
                                    const char* source,
                                    const std::string& name) -> std::shared_ptr<class ComputePipelineState>;

    /**
     * @brief Creates a render pipeline state object.
     */
    auto CreateRenderPipelineState(id<MTLDevice> device,
                                   const char* vertexSource,
                                   const char* fragmentSource,
                                   const std::string& name) -> std::shared_ptr<class RenderPipelineState>;

    /**
     * @brief Verifies that the resolver is ready to use from the current thread.
     * 
     * @param reason Return user-readable reason if verification fails.
     * @return true if the resolver can be used from the current thread.
     */
    auto VerifyBeforeUse(std::string& reason) const -> bool;

private:
    /**
     * @brief Selects a Metal device based on strategy.
     */
    static auto SelectDevice(DeviceSelection strategy) -> id<MTLDevice>;

    /**
     * @brief Creates a command queue from a device.
     */
    static auto CreateCommandQueue(id<MTLDevice> device) -> id<MTLCommandQueue>;

    /**
     * @brief Parses Metal device type string.
     */
    static auto ParseDeviceType(const std::string& typeStr) -> MTLDeviceType;

    /**
     * @brief Enables Metal debug layer.
     */
    static void EnableDebugLayer();

    mutable std::mutex m_mutex;
    bool m_loaded{false};
    bool m_initializing{false};
    std::condition_variable m_initCv;

    std::shared_ptr<const MetalBackendState> m_state;
};

/**
 * @brief Converts a DeviceSelection enum value to a human-readable string.
 */
inline auto DeviceSelectionToString(DeviceSelection selection) -> const char*
{
    switch (selection)
    {
        case DeviceSelection::Default:
            return "Default";
        case DeviceSelection::PreferIntegrated:
            return "PreferIntegrated";
        case DeviceSelection::PreferDiscrete:
            return "PreferDiscrete";
        case DeviceSelection::ForceCPU:
            return "ForceCPU";
        default:
            return "Unknown";
    }
}

} // namespace Platform
} // namespace Renderer
} // namespace libprojectM
