/**
 * @file ComputePipelineState.cpp
 * @brief Implementation of Metal compute pipeline state wrapper.
 */

#include "Renderer/ComputePipelineState.hpp"

namespace libprojectM {
namespace Renderer {

id<MTLComputePipelineState> ComputePipelineState::GetPipelineState() const
{
    return m_pipelineState;
}

id<MTLFunction> ComputePipelineState::GetComputeFunction() const
{
    if (!m_pipelineState)
        return nullptr;
    
    MTLComputePipelineState* pipeline = (__bridge MTLComputePipelineState*)m_pipelineState;
    return pipeline.computeFunction;
}

} // namespace Renderer
} // namespace libprojectM
