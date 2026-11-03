/**
 * @file RenderPipelineState.cpp
 * @brief Implementation of Metal render pipeline state wrapper.
 */

#include "Renderer/RenderPipelineState.hpp"

namespace libprojectM {
namespace Renderer {

id<MTLRenderPipelineState> RenderPipelineState::GetPipelineState() const
{
    return m_pipelineState;
}

id<MTLFunction> RenderPipelineState::GetVertexFunction() const
{
    if (!m_pipelineState)
        return nullptr;
    
    MTLRenderPipelineState* pipeline = (__bridge MTLRenderPipelineState*)m_pipelineState;
    return pipeline.vertexFunction;
}

id<MTLFunction> RenderPipelineState::GetFragmentFunction() const
{
    if (!m_pipelineState)
        return nullptr;
    
    MTLRenderPipelineState* pipeline = (__bridge MTLRenderPipelineState*)m_pipelineState;
    return pipeline.fragmentFunction;
}

} // namespace Renderer
} // namespace libprojectM
