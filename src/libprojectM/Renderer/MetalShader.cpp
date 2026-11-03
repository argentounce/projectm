/**
 * @file MetalShader.cpp
 * @brief Implementation of Metal shader compilation and management.
 */

#include "Renderer/MetalShader.hpp"

#include <cstring>
#include <regex>

namespace libprojectM {
namespace Renderer {

void MetalShaderCache::Insert(const std::string& key, const std::shared_ptr<MetalShader>& shader)
{
    m_cachedShaders[key] = shader;
}

void MetalShaderCache::Remove(const std::string& key)
{
    m_cachedShaders.erase(key);
}

std::shared_ptr<MetalShader> MetalShaderCache::Get(const std::string& key) const
{
    auto it = m_cachedShaders.find(key);
    if (it != m_cachedShaders.end())
        return it->second;
    return nullptr;
}

void MetalShaderCache::Clear()
{
    m_cachedShaders.clear();
}

std::string MetalShaderCompiler::ConvertToMSL(const std::string& glslSource,
                                              const std::string& targetVersion,
                                              const std::string& profile)
{
    // This is a placeholder implementation.
    // In production, this would use glslangValidator or similar tool.
    
    std::string mslSource = glslSource;

    // Convert GLSL version to MSL version
    mslSource = ConvertGLSLVersionToMSL(0, 0);

    // Convert precision qualifiers
    mslSource = ConvertPrecisionQualifiers(mslSource);

    // Convert built-in variables
    mslSource = ConvertBuiltins(mslSource);

    // Convert type qualifiers
    mslSource = ConvertTypeQualifiers(mslSource);

    // Convert array syntax
    mslSource = ConvertArraySyntax(mslSource);

    // Convert texture functions
    mslSource = ConvertTextureFunctions(mslSource);

    // Convert lighting functions
    mslSource = ConvertLightingFunctions(mslSource);

    // Convert matrix functions
    mslSource = ConvertMatrixFunctions(mslSource);

    // Convert vector functions
    mslSource = ConvertVectorFunctions(mslSource);

    // Convert math functions
    mslSource = ConvertMathFunctions(mslSource);

    return mslSource;
}

std::string MetalShaderCompiler::ConvertGLSLVersionToMSL(int major, int minor)
{
    // GLSL 1.50 -> MSL 1.2
    // GLSL 3.30 -> MSL 2.0
    // GLSL 4.50 -> MSL 2.0
    
    switch (major)
    {
        case 1:
            if (minor <= 5)
                return "#version msl_1_2";
            break;
        case 3:
            if (minor >= 0 && minor <= 3)
                return "#version msl_1_2";
            break;
        case 4:
            if (minor >= 0 && minor <= 5)
                return "#version msl_2_0";
            break;
        default:
            return "#version msl_1_2";
    }
    
    return "#version msl_1_2";
}

std::string MetalShaderCompiler::ConvertPrecisionQualifiers(const std::string& source)
{
    // GLSL: lowp, mediump, highp
    // MSL: __attribute__((precision(...)))
    
    std::string result;
    std::regex precisionRegex(R"((\b(lowp|mediump|highp)\s+)(\w+))");
    
    std::smatch match;
    while (std::regex_search(source, match, precisionRegex))
    {
        std::string qualifier = match[1];
        std::string type = match[2];
        
        std::string mslPrecision;
        if (qualifier == "lowp")
            mslPrecision = "low";
        else if (qualifier == "mediump")
            mslPrecision = "medium";
        else
            mslPrecision = "high";
        
        result += type + " __attribute__((precision(" + mslPrecision + "))) ";
        
        source = match.suffix().str();
    }
    
    return result + source;
}

std::string MetalShaderCompiler::ConvertBuiltins(const std::string& source)
{
    // GLSL: gl_FragColor, gl_Position
    // MSL: [[stage_in]], [[stage_out]]
    
    std::string result;
    std::regex glFragColorRegex(R"(gl_FragColor)");
    std::regex glPositionRegex(R"(gl_Position)");
    
    result = std::regex_replace(source, glFragColorRegex, "[[stage_in]](float4)");
    result = std::regex_replace(result, glPositionRegex, "[[stage_in]](float4)");
    
    return result;
}

std::string MetalShaderCompiler::ConvertTypeQualifiers(const std::string& source)
{
    // GLSL: const, uniform, varying, in, out
    // MSL: [[buffer(N)]], [[attribute(N)]], [[uniform]]
    
    std::string result;
    
    // Convert uniform declarations
    std::regex uniformRegex(R"(uniform\s+(\w+)\s+(\w+))");
    std::smatch match;
    
    while (std::regex_search(source, match, uniformRegex))
    {
        std::string type = match[1];
        std::string name = match[2];
        
        result += " [[uniform]] " + type + " " + name + " ";
        source = match.suffix().str();
    }
    
    return result + source;
}

std::string MetalShaderCompiler::ConvertArraySyntax(const std::string& source)
{
    // GLSL: vec4[2]
    // MSL: float4[2]
    
    std::string result;
    std::regex arrayRegex(R"((\w+)\s*\[\s*(\d+)\s*\])");
    
    std::smatch match;
    while (std::regex_search(source, match, arrayRegex))
    {
        std::string type = match[1];
        std::string size = match[2];
        
        // Convert GLSL types to MSL types
        std::string mslType = type;
        if (mslType == "vec2")
            mslType = "float2";
        else if (mslType == "vec3")
            mslType = "float3";
        else if (mslType == "vec4")
            mslType = "float4";
        else if (mslType == "ivec2")
            mslType = "int2";
        else if (mslType == "ivec3")
            mslType = "int3";
        else if (mslType == "ivec4")
            mslType = "int4";
        else if (mslType == "mat2x2")
            mslType = "float2x2";
        else if (mslType == "mat2x3")
            mslType = "float2x3";
        else if (mslType == "mat2x4")
            mslType = "float2x4";
        else if (mslType == "mat3x2")
            mslType = "float3x2";
        else if (mslType == "mat3x3")
            mslType = "float3x3";
        else if (mslType == "mat3x4")
            mslType = "float3x4";
        else if (mslType == "mat4x2")
            mslType = "float4x2";
        else if (mslType == "mat4x3")
            mslType = "float4x3";
        else if (mslType == "mat4x4")
            mslType = "float4x4";
        
        result += mslType + "[" + size + "] ";
        source = match.suffix().str();
    }
    
    return result + source;
}

std::string MetalShaderCompiler::ConvertTextureFunctions(const std::string& source)
{
    // GLSL: texture2D, textureCube
    // MSL: sampler_state, texture_sample_2d
    
    std::string result;
    std::regex texture2DRegex(R"(texture2D\s*\(\s*(\w+)\s*,\s*(\w+)\s*\))");
    std::regex textureCubeRegex(R"(textureCube\s*\(\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\))");
    
    result = std::regex_replace(result, texture2DRegex, "texture_sample_2d($1, $2)");
    result = std::regex_replace(result, textureCubeRegex, "texture_sample_cube($1, $2, $3)");
    
    return result;
}

std::string MetalShaderCompiler::ConvertLightingFunctions(const std::string& source)
{
    // GLSL: reflect, refract
    // MSL: metal_reflect, metal_refract
    
    std::string result;
    std::regex reflectRegex(R"(reflect\s*\(\s*(\w+)\s*,\s*(\w+)\s*\))");
    std::regex refractRegex(R"(refract\s*\(\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\))");
    
    result = std::regex_replace(result, reflectRegex, "metal_reflect($1, $2)");
    result = std::regex_replace(result, refractRegex, "metal_refract($1, $2, $3)");
    
    return result;
}

std::string MetalShaderCompiler::ConvertMatrixFunctions(const std::string& source)
{
    // GLSL: matrixCompMult, outerProduct
    // MSL: matrix_comp_mult, outer_product
    
    std::string result;
    std::regex matrixCompMultRegex(R"(matrixCompMult\s*\(\s*(\w+)\s*,\s*(\w+)\s*\))");
    std::regex outerProductRegex(R"(outerProduct\s*\(\s*(\w+)\s*,\s*(\w+)\s*\))");
    
    result = std::regex_replace(result, matrixCompMultRegex, "matrix_comp_mult($1, $2)");
    result = std::regex_replace(result, outerProductRegex, "outer_product($1, $2)");
    
    return result;
}

std::string MetalShaderCompiler::ConvertVectorFunctions(const std::string& source)
{
    // GLSL: dot, cross, length, normalize
    // MSL: dot, cross, length, normalize (same)
    
    return source;
}

std::string MetalShaderCompiler::ConvertMathFunctions(const std::string& source)
{
    // GLSL: sin, cos, tan, asin, acos, atan, atan2, exp, log, sqrt
    // MSL: same names
    
    return source;
}

std::string MetalShaderCompiler::ConvertStatement(const std::string& statement)
{
    return statement;
}

std::string MetalShaderCompiler::ConvertFunction(const std::string& function)
{
    return function;
}

std::string MetalShaderCompiler::ConvertUniforms(const std::string& uniforms)
{
    return uniforms;
}

std::string MetalShaderCompiler::ConvertVaryings(const std::string& varyings)
{
    return varyings;
}

std::string MetalShaderCompiler::ConvertInOutQualifiers(const std::string& source)
{
    // GLSL: in, out
    // MSL: [[stage_in]], [[stage_out]]
    
    std::string result;
    std::regex inRegex(R"(in\s+(\w+)\s+(\w+))");
    std::regex outRegex(R"(out\s+(\w+)\s+(\w+))");
    
    result = std::regex_replace(result, inRegex, "[[stage_in]] $2 $1");
    result = std::regex_replace(result, outRegex, "[[stage_out]] $2 $1");
    
    return result;
}

std::shared_ptr<class RenderPipelineState> MetalShaderPipelineFactory::CreateRenderPipeline(
    id<MTLDevice> device,
    const std::string& vertexSource,
    const std::string& fragmentSource,
    const std::string& name)
{
    MTLRenderPipelineDescriptor* descriptor = [[MTLRenderPipelineDescriptor alloc] init];
    
    descriptor.vertexFunction = __builtin_stdlib_cast_symbol(MTLVertexFunctionType, vertexSource.c_str());
    descriptor.fragmentFunction = __builtin_stdlib_cast_symbol(MTLFragmentFunctionType, fragmentSource.c_str());
    
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

std::shared_ptr<class ComputePipelineState> MetalShaderPipelineFactory::CreateComputePipeline(
    id<MTLDevice> device,
    const std::string& computeSource,
    const std::string& name)
{
    MTLComputePipelineDescriptor* descriptor = [[MTLComputePipelineDescriptor alloc] init];
    
    descriptor.computationFunction = __builtin_stdlib_cast_symbol(MTLComputeFunctionType, computeSource.c_str());
    
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

id<MTLVertexDescriptor> MetalShaderPipelineFactory::CreateVertexDescriptor(
    const std::vector<MTLVertexAttribute>& attributes,
    const std::vector<MTLVertexStepFunction>& stepFunctions)
{
    MTLVertexDescriptor* descriptor = [[MTLVertexDescriptor alloc] init];
    
    // Set up attributes
    for (auto& attr : attributes)
    {
        descriptor.attributes[attr.index] = attr;
    }
    
    // Set up step functions
    for (auto& step : stepFunctions)
    {
        descriptor.layouts[step.index].stepFunction = step;
    }
    
    return (__bridge id<MTLVertexDescriptor>)descriptor;
}

id<MTLVertexDescriptor> MetalShaderPipelineFactory::CreateVertexDescriptorFromGLSL(
    const std::string& vertexSource)
{
    // TODO: Parse GLSL source to extract vertex attributes
    // This is a placeholder
    
    MTLVertexDescriptor* descriptor = [[MTLVertexDescriptor alloc] init];
    
    // Default vertex layout
    descriptor.attributes[0] = MTLVertexAttribute{
        .format = MTLVertexFormatFloat2,
        .offset = 0,
        .bufferIndex = 0
    };
    
    descriptor.layouts[0].stride = 8;
    descriptor.layouts[0].stepFunction = MTLVertexStepFunctionVertex;
    
    return (__bridge id<MTLVertexDescriptor>)descriptor;
}

std::vector<MTLVertexAttribute> MetalShaderPipelineFactory::ExtractVertexAttributes(
    const std::string& source)
{
    // TODO: Parse GLSL source to extract vertex attributes
    return {};
}

std::vector<MTLVertexStepFunction> MetalShaderPipelineFactory::ExtractStepFunctions(
    const std::string& source)
{
    // TODO: Parse GLSL source to extract step functions
    return {};
}

} // namespace Renderer
} // namespace libprojectM
