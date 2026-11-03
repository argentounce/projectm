/**
 * @file ComputePipelineState.hpp
 * @brief Metal compute pipeline state wrapper.
 */

#pragma once

#include <Metal/Metal.h>
#include <memory>

namespace libprojectM {
namespace Renderer {

/**
 * @brief Wrapper for MTLComputePipelineState.
 */
class ComputePipelineState
{
public:
    ComputePipelineState() = default;
    ~ComputePipelineState() = default;

    ComputePipelineState(const ComputePipelineState&) = delete;
    auto operator=(const ComputePipelineState&) -> ComputePipelineState& = delete;
    ComputePipelineState(ComputePipelineState&&) = default;
    auto operator=(ComputePipelineState&&) -> ComputePipelineState& = default;

    /**
     * @brief Returns the underlying Metal pipeline state.
     */
    auto GetPipelineState() const -> id<MTLComputePipelineState>;

    /**
     * @brief Returns the compute function.
     */
    auto GetComputeFunction() const -> id<MTLFunction>;

private:
    id<MTLComputePipelineState> m_pipelineState{nullptr};
};

} // namespace Renderer
} // namespace libprojectM
