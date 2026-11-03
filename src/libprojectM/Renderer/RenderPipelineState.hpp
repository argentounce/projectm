/**
 * @file RenderPipelineState.hpp
 * @brief Metal render pipeline state wrapper.
 */

#pragma once

#include <Metal/Metal.h>
#include <memory>

namespace libprojectM {
namespace Renderer {

/**
 * @brief Wrapper for MTLRenderPipelineState.
 */
class RenderPipelineState
{
public:
    RenderPipelineState() = default;
    ~RenderPipelineState() = default;

    RenderPipelineState(const RenderPipelineState&) = delete;
    auto operator=(const RenderPipelineState&) -> RenderPipelineState& = delete;
    RenderPipelineState(RenderPipelineState&&) = default;
    auto operator=(RenderPipelineState&&) -> RenderPipelineState& = default;

    /**
     * @brief Returns the underlying Metal pipeline state.
     */
    auto GetPipelineState() const -> id<MTLRenderPipelineState>;

    /**
     * @brief Returns the vertex function.
     */
    auto GetVertexFunction() const -> id<MTLFunction>;

    /**
     * @brief Returns the fragment function.
     */
    auto GetFragmentFunction() const -> id<MTLFunction>;

private:
    id<MTLRenderPipelineState> m_pipelineState{nullptr};
};

} // namespace Renderer
} // namespace libprojectM
