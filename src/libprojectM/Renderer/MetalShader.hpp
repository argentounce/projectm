/**
 * @file MetalShader.hpp
 * @brief Metal shader compilation and management.
 * 
 * This module handles GLSL to MSL conversion and shader pipeline
 * state creation for Metal rendering.
 */

#pragma once

#include "Renderer/Shader.hpp"
#include "Renderer/Platform/MetalResolver.hpp"

#include <memory>
#include <string>
#include <vector>

namespace libprojectM {
namespace Renderer {

/**
 * @brief Metal shader type.
 */
enum class MSLShaderType : uint8_t
{
    Vertex = 0,
    Fragment = 1,
    Compute = 2
};

/**
 * @brief Metal shader compilation exception.
 */
class MetalShaderException : public ShaderException
{
public:
    MetalShaderException(std::string message)
        : ShaderException(std::move(message))
    {
    }

    ~MetalShaderException() override = default;
};

/**
 * @brief Metal shader cache.
 */
class MetalShaderCache
{
public:
    MetalShaderCache() = default;
    ~MetalShaderCache() = default;

    MetalShaderCache(const MetalShaderCache&) = delete;
    auto operator=(const MetalShaderCache&) -> MetalShaderCache& = delete;
    MetalShaderCache(MetalShaderCache&&) = default;
    auto operator=(MetalShaderCache&&) -> MetalShaderCache& = default;

    /**
     * @brief Adds a shader to the cache.
     */
    void Insert(const std::string& key, const std::shared_ptr<MetalShader>& shader);

    /**
     * @brief Removes a shader from the cache.
     */
    void Remove(const std::string& key);

    /**
     * @brief Returns a cached shader.
     */
    auto Get(const std::string& key) const -> std::shared_ptr<MetalShader>;

    /**
     * @brief Clears the cache.
     */
    void Clear();

private:
    std::map<std::string, std::shared_ptr<MetalShader>> m_cachedShaders;
};

/**
 * @brief Metal shader compiler.
 */
class MetalShaderCompiler
{
public:
    MetalShaderCompiler() = default;
    ~MetalShaderCompiler() = default;

    MetalShaderCompiler(const MetalShaderCompiler&) = delete;
    auto operator=(const MetalShaderCompiler&) -> MetalShaderCompiler& = delete;
    MetalShaderCompiler(MetalShaderCompiler&&) = default;
    auto operator=(MetalShaderCompiler&&) -> MetalShaderCompiler& = default;

    /**
     * @brief Converts GLSL to MSL.
     * 
     * @param glslSource The GLSL shader source.
     * @param targetVersion The target MSL version (e.g., "msl_1_2").
     * @param profile The shader profile (e.g., "vertex", "fragment").
     * @return The converted MSL source.
     */
    static std::string ConvertToMSL(const std::string& glslSource,
                                   const std::string& targetVersion = "msl_1_2",
                                   const std::string& profile = "vertex");

    /**
     * @brief Converts a GLSL version string to MSL version string.
     */
    static std::string ConvertGLSLVersionToMSL(int major, int minor);

    /**
     * @brief Converts GLSL precision qualifiers to MSL.
     */
    static std::string ConvertPrecisionQualifiers(const std::string& source);

    /**
     * @brief Converts GLSL built-in variables to MSL.
     */
    static std::string ConvertBuiltins(const std::string& source);

    /**
     * @brief Converts GLSL type qualifiers to MSL.
     */
    static std::string ConvertTypeQualifiers(const std::string& source);

    /**
     * @brief Converts GLSL array syntax to MSL.
     */
    static std::string ConvertArraySyntax(const std::string& source);

    /**
     * @brief Converts GLSL texture functions to MSL.
     */
    static std::string ConvertTextureFunctions(const std::string& source);

    /**
     * @brief Converts GLSL lighting functions to MSL.
     */
    static std::string ConvertLightingFunctions(const std::string& source);

    /**
     * @brief Converts GLSL matrix functions to MSL.
     */
    static std::string ConvertMatrixFunctions(const std::string& source);

    /**
     * @brief Converts GLSL vector functions to MSL.
     */
    static std::string ConvertVectorFunctions(const std::string& source);

    /**
     * @brief Converts GLSL math functions to MSL.
     */
    static std::string ConvertMathFunctions(const std::string& source);

    /**
     * @brief Performs a complete GLSL to MSL conversion.
     */
    static std::string FullConvert(const std::string& glslSource);

private:
    /**
     * @brief Converts a single GLSL statement to MSL.
     */
    static std::string ConvertStatement(const std::string& statement);

    /**
     * @brief Converts a GLSL function declaration to MSL.
     */
    static std::string ConvertFunction(const std::string& function);

    /**
     * @brief Converts GLSL uniform declarations to MSL.
     */
    static std::string ConvertUniforms(const std::string& uniforms);

    /**
     * @brief Converts GLSL varying declarations to MSL.
     */
    static std::string ConvertVaryings(const std::string& varyings);

    /**
     * @brief Converts GLSL in/out qualifiers to MSL.
     */
    static std::string ConvertInOutQualifiers(const std::string& source);
};

/**
 * @brief Metal shader pipeline state factory.
 */
class MetalShaderPipelineFactory
{
public:
    MetalShaderPipelineFactory() = default;
    ~MetalShaderPipelineFactory() = default;

    MetalShaderPipelineFactory(const MetalShaderPipelineFactory&) = delete;
    auto operator=(const MetalShaderPipelineFactory&) -> MetalShaderPipelineFactory& = delete;
    MetalShaderPipelineFactory(MetalShaderPipelineFactory&&) = default;
    auto operator=(MetalShaderPipelineFactory&&) -> MetalShaderPipelineFactory& = delete;

    /**
     * @brief Creates a render pipeline state from compiled shaders.
     */
    static std::shared_ptr<class RenderPipelineState> CreateRenderPipeline(
        id<MTLDevice> device,
        const std::string& vertexSource,
        const std::string& fragmentSource,
        const std::string& name);

    /**
     * @brief Creates a compute pipeline state from compiled shader.
     */
    static std::shared_ptr<class ComputePipelineState> CreateComputePipeline(
        id<MTLDevice> device,
        const std::string& computeSource,
        const std::string& name);

    /**
     * @brief Creates a vertex descriptor.
     */
    static id<MTLVertexDescriptor> CreateVertexDescriptor(
        const std::vector<MTLVertexAttribute>& attributes,
        const std::vector<MTLVertexStepFunction>& stepFunctions);

    /**
     * @brief Creates a vertex descriptor from GLSL layout.
     */
    static id<MTLVertexDescriptor> CreateVertexDescriptorFromGLSL(
        const std::string& vertexSource);

private:
    /**
     * @brief Extracts vertex attributes from GLSL source.
     */
    static std::vector<MTLVertexAttribute> ExtractVertexAttributes(
        const std::string& source);

    /**
     * @brief Extracts step functions from GLSL source.
     */
    static std::vector<MTLVertexStepFunction> ExtractStepFunctions(
        const std::string& source);
};

} // namespace Renderer
} // namespace libprojectM
