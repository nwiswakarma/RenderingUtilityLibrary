////////////////////////////////////////////////////////////////////////////////
//
// MIT License
// 
// Copyright (c) 2018-2019 Nuraga Wiswakarma
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////
// 

#include "Shaders/RULReduceScan.h"

#include "ShaderParameters.h"
#include "ShaderCore.h"
#include "UniformBuffer.h"

#include "RHI/RULAlignedTypes.h"
#include "Shaders/RULShaderDefinitions.h"

template<uint32 ScanDataType, uint32 ScanOpType>
class FRULReduceLocalScanCS : public FRULBaseComputeShader<>
{
    typedef FRULBaseComputeShader<> FBaseType;

    DECLARE_SHADER_TYPE(FRULReduceLocalScanCS, Global);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return RHISupportsComputeShaders(Parameters.Platform);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FBaseType::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
        OutEnvironment.SetDefine(TEXT("data_t"), *FRULReduceScan::GetScanDataTypeName<ScanDataType>());
        OutEnvironment.SetDefine(TEXT("REDUCE_OP"), ScanOpType);
    }

    RUL_DECLARE_SHADER_CONSTRUCTOR_SERIALIZER(FRULReduceLocalScanCS)

    RUL_DECLARE_SHADER_PARAMETERS_1(
        SRV,
        FShaderResourceParameter,
        FResourceId,
        "SrcData", SrcData
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        UAV,
        FShaderResourceParameter,
        FResourceId,
        "SumData", SumData
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Value,
        FShaderParameter,
        FParameterId,
        "_ElementCount", Params_ElementCount
        )
};

template<uint32 ScanDataType, uint32 ScanOpType>
class FRULReduceTextureLocalScanCS : public FRULBaseComputeShader<>
{
    typedef FRULBaseComputeShader<> FBaseType;

    DECLARE_SHADER_TYPE(FRULReduceTextureLocalScanCS, Global);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return RHISupportsComputeShaders(Parameters.Platform);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FBaseType::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
        OutEnvironment.SetDefine(TEXT("data_t"), *FRULReduceScan::GetScanDataTypeName<ScanDataType>());
        OutEnvironment.SetDefine(TEXT("REDUCE_OP"), ScanOpType);
    }

    RUL_DECLARE_SHADER_CONSTRUCTOR_SERIALIZER_WITH_TEXTURE(FRULReduceTextureLocalScanCS)

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Texture,
        FShaderResourceParameter,
        FResourceId,
        "SourceTexture", SourceTexture
        )

    RUL_DECLARE_SHADER_PARAMETERS_0(Sampler,,)

    RUL_DECLARE_SHADER_PARAMETERS_0(SRV,,)

    RUL_DECLARE_SHADER_PARAMETERS_1(
        UAV,
        FShaderResourceParameter,
        FResourceId,
        "SumData", SumData
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Value,
        FShaderParameter,
        FParameterId,
        "_Dimension", Params_Dimension
        )
};

template<uint32 ScanDataType, uint32 ScanOpType>
class FRULReduceTopLevelScanCS : public FRULBaseComputeShader<>
{
    typedef FRULBaseComputeShader<> FBaseType;

    DECLARE_SHADER_TYPE(FRULReduceTopLevelScanCS, Global);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return RHISupportsComputeShaders(Parameters.Platform);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FBaseType::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
        OutEnvironment.SetDefine(TEXT("data_t"), *FRULReduceScan::GetScanDataTypeName<ScanDataType>());
        OutEnvironment.SetDefine(TEXT("REDUCE_OP"), ScanOpType);
    }

    RUL_DECLARE_SHADER_CONSTRUCTOR_SERIALIZER(FRULReduceTopLevelScanCS)

    RUL_DECLARE_SHADER_PARAMETERS_0(SRV,,)

    RUL_DECLARE_SHADER_PARAMETERS_1(
        UAV,
        FShaderResourceParameter,
        FResourceId,
        "SumData", SumData
        )

    RUL_DECLARE_SHADER_PARAMETERS_3(
        Value,
        FShaderParameter,
        FParameterId,
        "_ElementCount",   Params_ElementCount,
        "_BlockCount",     Params_BlockCount,
        "_ScanBlockCount", Params_ScanBlockCount
        )
};

template<uint32 ScanDataType>
class FRULWriteScanResultCS : public FRULBaseComputeShader<>
{
    typedef FRULBaseComputeShader<> FBaseType;

    DECLARE_SHADER_TYPE(FRULWriteScanResultCS, Global);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return RHISupportsComputeShaders(Parameters.Platform);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FBaseType::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
        OutEnvironment.SetDefine(TEXT("data_t"), *FRULReduceScan::GetScanDataTypeName<ScanDataType>());
    }

    RUL_DECLARE_SHADER_CONSTRUCTOR_SERIALIZER(FRULWriteScanResultCS)

    RUL_DECLARE_SHADER_PARAMETERS_1(
        SRV,
        FShaderResourceParameter,
        FResourceId,
        "SrcData", SrcData
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        UAV,
        FShaderResourceParameter,
        FResourceId,
        "SumData", SumData
        )

    RUL_DECLARE_SHADER_PARAMETERS_2(
        Value,
        FShaderParameter,
        FParameterId,
        "_ElementCount", Params_ElementCount,
        "_ResultIndex", Params_ResultIndex
        )
};

#define SHADER_FILENAME "/Plugin/RenderingUtilityLibrary/Private/RULReduceScanCS.usf"
#define SCAN_KERNEL1(N,VN)   N<VN>
#define SCAN_KERNEL2(N,VN,T) N<VN,T>

#define IMPLEMENT_SCAN_SHADER(VN) \
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL2(FRULReduceLocalScanCS,VN,0), TEXT(SHADER_FILENAME), TEXT("LocalScanKernel"), SF_Compute);\
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL2(FRULReduceLocalScanCS,VN,1), TEXT(SHADER_FILENAME), TEXT("LocalScanKernel"), SF_Compute);\
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL2(FRULReduceTextureLocalScanCS,VN,0), TEXT(SHADER_FILENAME), TEXT("TextureLocalScanKernel"), SF_Compute);\
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL2(FRULReduceTextureLocalScanCS,VN,1), TEXT(SHADER_FILENAME), TEXT("TextureLocalScanKernel"), SF_Compute);\
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL2(FRULReduceTopLevelScanCS,VN,0), TEXT(SHADER_FILENAME), TEXT("TopLevelScanKernel"), SF_Compute);\
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL2(FRULReduceTopLevelScanCS,VN,1), TEXT(SHADER_FILENAME), TEXT("TopLevelScanKernel"), SF_Compute);\
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL1(FRULWriteScanResultCS,VN), TEXT(SHADER_FILENAME), TEXT("WriteScanResultKernel"), SF_Compute);\

IMPLEMENT_SCAN_SHADER(FRULReduceScan::SDT_UINT1)
IMPLEMENT_SCAN_SHADER(FRULReduceScan::SDT_UINT2)
IMPLEMENT_SCAN_SHADER(FRULReduceScan::SDT_UINT4)

IMPLEMENT_SCAN_SHADER(FRULReduceScan::SDT_FLOAT1)
IMPLEMENT_SCAN_SHADER(FRULReduceScan::SDT_FLOAT2)
IMPLEMENT_SCAN_SHADER(FRULReduceScan::SDT_FLOAT4)

#undef IMPLEMENT_SCAN_SHADER
#undef SCAN_KERNEL1
#undef SCAN_KERNEL2
#undef SHADER_FILENAME

template<uint32 ScanDataType, uint32 ScanOpType>
int32 FRULReduceScan::Reduce(
    FRHICommandListImmediate& RHICmdList,
    FShaderResourceViewRHIParamRef SrcDataSRV,
    FRULRWBufferStructured& ResultBuffer,
    int32 DataStride,
    int32 ElementCount,
    uint32 AdditionalOutputUsage
    )
{
    check(IsInRenderingThread());
    check(IsValidScanDataType<ScanDataType>());
    check(IsValidScanOpType<ScanOpType>());

    if (ElementCount < 1)
    {
        return -1;
    }

    check(DataStride > 0);

    int32 BlockCount      = FMath::DivideAndRoundUp(ElementCount, BLOCK_SIZE2);
    int32 BlockGroupCount = FMath::DivideAndRoundUp(BlockCount, BLOCK_SIZE2);

    int32 ScanBlockCount      = FPlatformMath::RoundUpToPowerOfTwo(BlockCount);
    int32 ScanBlockGroupCount = FPlatformMath::RoundUpToPowerOfTwo(BlockGroupCount);

    int32 SumBufferCount = ScanBlockCount + 1;

    check(BlockCount > 0);

    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() ElementCount: %d"), ElementCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() BlockCount: %d"), BlockCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() BlockGroupCount: %d"), BlockGroupCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() ScanBlockCount: %d"), ScanBlockCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() ScanBlockGroupCount: %d"), ScanBlockGroupCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() SumBufferCount: %d"), SumBufferCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() BLOCK_SIZE2: %d"), BLOCK_SIZE2);

    // Reset result buffer

    ResultBuffer.Release();
    ResultBuffer.Initialize(DataStride, 1, AdditionalOutputUsage);

    // Initialize sum buffer

    FRULRWBufferStructured SumBuffer;
    {
        TResourceArray<uint8, VERTEXBUFFER_ALIGNMENT> DefaultSumData(false);
        DefaultSumData.SetNumZeroed(DataStride * SumBufferCount);
        SumBuffer.Initialize(DataStride, SumBufferCount, &DefaultSumData, AdditionalOutputUsage);
    }

    // Local scan kernel

    RHICmdList.BeginComputePass(TEXT("RULReduceLocalScan"));
    TShaderMapRef<FRULReduceLocalScanCS<ScanDataType, ScanOpType>> LocalScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
    LocalScanCS->SetShader(RHICmdList);
    LocalScanCS->BindSRV(RHICmdList, TEXT("SrcData"), SrcDataSRV);
    LocalScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
    LocalScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
    DispatchComputeShader(RHICmdList, *LocalScanCS, BlockCount, 1, 1);
    LocalScanCS->UnbindBuffers(RHICmdList);
    RHICmdList.EndComputePass();

    if (BlockGroupCount > 1)
    {
        int32 SumBlockBufferCount = ScanBlockGroupCount+1;

        FRULRWBufferStructured SumBlockBuffer;
        SumBlockBuffer.Initialize(DataStride, SumBlockBufferCount, BUF_Static);

        // Block sum scan

        RHICmdList.BeginComputePass(TEXT("RULReduceLocalScan"));
        TShaderMapRef<FRULReduceLocalScanCS<ScanDataType, ScanOpType>> BlockScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        BlockScanCS->SetShader(RHICmdList);
        BlockScanCS->BindSRV(RHICmdList, TEXT("SrcData"), SumBuffer.SRV);
        BlockScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBlockBuffer.UAV);
        BlockScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
        DispatchComputeShader(RHICmdList, *BlockScanCS, BlockGroupCount, 1, 1);
        BlockScanCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();

        // Block sum top level scan

        RHICmdList.BeginComputePass(TEXT("RULReduceTopLevelScan"));
        TShaderMapRef<FRULReduceTopLevelScanCS<ScanDataType, ScanOpType>> TopLevelScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        TopLevelScanCS->SetShader(RHICmdList);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBlockBuffer.UAV);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_BlockCount"), BlockGroupCount);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ScanBlockCount"), ScanBlockGroupCount);
        DispatchComputeShader(RHICmdList, *TopLevelScanCS, 1, 1, 1);
        TopLevelScanCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();

        // Write result

        RHICmdList.BeginComputePass(TEXT("RULWriteScanResult"));
        TShaderMapRef<FRULWriteScanResultCS<ScanDataType>> WriteResultCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        WriteResultCS->SetShader(RHICmdList);
        WriteResultCS->BindSRV(RHICmdList, TEXT("SrcData"), SumBlockBuffer.SRV);
        WriteResultCS->BindUAV(RHICmdList, TEXT("SumData"), ResultBuffer.UAV);
        WriteResultCS->SetParameter(RHICmdList, TEXT("_ElementCount"), SumBlockBufferCount);
        DispatchComputeShader(RHICmdList, *WriteResultCS, 1, 1, 1);
        WriteResultCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();
    }
    else
    {
        // Top level scan

        RHICmdList.BeginComputePass(TEXT("RULReduceTopLevelScan"));
        TShaderMapRef<FRULReduceTopLevelScanCS<ScanDataType, ScanOpType>> TopLevelScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        TopLevelScanCS->SetShader(RHICmdList);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_BlockCount"), BlockCount);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ScanBlockCount"), ScanBlockCount);
        DispatchComputeShader(RHICmdList, *TopLevelScanCS, 1, 1, 1);
        TopLevelScanCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();

        // Write result

        RHICmdList.BeginComputePass(TEXT("RULWriteScanResult"));
        TShaderMapRef<FRULWriteScanResultCS<ScanDataType>> WriteResultCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        WriteResultCS->SetShader(RHICmdList);
        WriteResultCS->BindSRV(RHICmdList, TEXT("SrcData"), SumBuffer.SRV);
        WriteResultCS->BindUAV(RHICmdList, TEXT("SumData"), ResultBuffer.UAV);
        WriteResultCS->SetParameter(RHICmdList, TEXT("_ElementCount"), SumBufferCount);
        DispatchComputeShader(RHICmdList, *WriteResultCS, 1, 1, 1);
        WriteResultCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();
    }

    return ScanBlockCount;
}

template<uint32 ScanOpType>
int32 FRULReduceScan::ReduceTexture(
    FRHICommandListImmediate& RHICmdList,
    FTextureRHIParamRef SourceTexture,
    FRULRWBufferStructured& ResultBuffer,
    FIntPoint Dimension,
    int32 ResultIndex,
    bool bInitializeResultBuffer,
    uint32 AdditionalOutputUsage
    )
{
    check(IsInRenderingThread());

    const int32 ElementCount = Dimension.X * Dimension.Y;

    enum { ScanDataType = FRULReduceScan::SDT_FLOAT4 };
    enum { DataStride = sizeof(FVector4) };

    if (! SourceTexture || ElementCount < 1)
    {
        return -1;
    }

    ResultIndex = FMath::Max(0, ResultIndex);

    // Initialize result buffer if required
    if (bInitializeResultBuffer)
    {
        ResultBuffer.Release();
        ResultBuffer.Initialize(DataStride, ResultIndex+1, AdditionalOutputUsage);
    }
    // Otherwise make sure result buffer is valid
    else
    if (! ResultBuffer.IsValidIndex(ResultIndex))
    {
        return -1;
    }

    check(IsValidScanDataType<ScanDataType>());

    int32 TexDispatchX = FMath::DivideAndRoundUp(Dimension.X, TEX_BLOCK2);
    int32 TexDispatchY = FMath::DivideAndRoundUp(Dimension.Y, TEX_BLOCK2);
    int32 TexExtentX = FMath::DivideAndRoundUp(Dimension.X, 2);

    int32 BlockCount      = FMath::DivideAndRoundUp(ElementCount, BLOCK_SIZE2);
    int32 BlockGroupCount = FMath::DivideAndRoundUp(BlockCount, BLOCK_SIZE2);

    int32 ScanBlockCount      = FPlatformMath::RoundUpToPowerOfTwo(BlockCount);
    int32 ScanBlockGroupCount = FPlatformMath::RoundUpToPowerOfTwo(BlockGroupCount);

    int32 SumBufferCount = ScanBlockCount + 1;

    FIntVector4 DimensionData(Dimension.X, Dimension.Y, TexExtentX, 0);

    check(BlockCount > 0);

    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() ElementCount: %d"), ElementCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() BlockCount: %d"), BlockCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() BlockGroupCount: %d"), BlockGroupCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() ScanBlockCount: %d"), ScanBlockCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() ScanBlockGroupCount: %d"), ScanBlockGroupCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() SumBufferCount: %d"), SumBufferCount);
    UE_LOG(UntRUL,Warning, TEXT("RULReduceScan::Reduce() BLOCK_SIZE2: %d"), BLOCK_SIZE2);

    // Initialize sum buffer

    FRULRWBufferStructured SumBuffer;
    {
        TResourceArray<uint8, VERTEXBUFFER_ALIGNMENT> DefaultSumData(false);
        DefaultSumData.SetNumZeroed(DataStride * SumBufferCount);
        SumBuffer.Initialize(DataStride, SumBufferCount, &DefaultSumData, AdditionalOutputUsage);
    }

    // Local scan kernel

    RHICmdList.BeginComputePass(TEXT("RULReduceTextureLocalScan"));
    TShaderMapRef<FRULReduceTextureLocalScanCS<ScanDataType, ScanOpType>> LocalScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
    LocalScanCS->SetShader(RHICmdList);
    LocalScanCS->BindTexture(RHICmdList, TEXT("SourceTexture"), SourceTexture);
    LocalScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
    LocalScanCS->SetParameter(RHICmdList, TEXT("_Dimension"), DimensionData);
    DispatchComputeShader(RHICmdList, *LocalScanCS, BlockCount, 1, 1);
    LocalScanCS->UnbindBuffers(RHICmdList);
    RHICmdList.EndComputePass();

    if (BlockGroupCount > 1)
    {
        int32 SumBlockBufferCount = ScanBlockGroupCount+1;

        FRULRWBufferStructured SumBlockBuffer;
        SumBlockBuffer.Initialize(DataStride, SumBlockBufferCount, BUF_Static);

        // Block sum scan

        RHICmdList.BeginComputePass(TEXT("RULReduceLocalScan"));
        TShaderMapRef<FRULReduceLocalScanCS<ScanDataType, ScanOpType>> BlockScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        BlockScanCS->SetShader(RHICmdList);
        BlockScanCS->BindSRV(RHICmdList, TEXT("SrcData"), SumBuffer.SRV);
        BlockScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBlockBuffer.UAV);
        BlockScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
        DispatchComputeShader(RHICmdList, *BlockScanCS, BlockGroupCount, 1, 1);
        BlockScanCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();

        // Block sum top level scan

        RHICmdList.BeginComputePass(TEXT("RULReduceTopLevelScan"));
        TShaderMapRef<FRULReduceTopLevelScanCS<ScanDataType, ScanOpType>> TopLevelScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        TopLevelScanCS->SetShader(RHICmdList);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBlockBuffer.UAV);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_BlockCount"), BlockGroupCount);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ScanBlockCount"), ScanBlockGroupCount);
        DispatchComputeShader(RHICmdList, *TopLevelScanCS, 1, 1, 1);
        TopLevelScanCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();

        // Write result

        RHICmdList.BeginComputePass(TEXT("RULWriteScanResult"));
        TShaderMapRef<FRULWriteScanResultCS<ScanDataType>> WriteResultCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        WriteResultCS->SetShader(RHICmdList);
        WriteResultCS->BindSRV(RHICmdList, TEXT("SrcData"), SumBlockBuffer.SRV);
        WriteResultCS->BindUAV(RHICmdList, TEXT("SumData"), ResultBuffer.UAV);
        WriteResultCS->SetParameter(RHICmdList, TEXT("_ElementCount"), SumBlockBufferCount);
        WriteResultCS->SetParameter(RHICmdList, TEXT("_ResultIndex"), ResultIndex);
        DispatchComputeShader(RHICmdList, *WriteResultCS, 1, 1, 1);
        WriteResultCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();
    }
    else
    {
        // Top level scan

        RHICmdList.BeginComputePass(TEXT("RULReduceTopLevelScan"));
        TShaderMapRef<FRULReduceTopLevelScanCS<ScanDataType, ScanOpType>> TopLevelScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        TopLevelScanCS->SetShader(RHICmdList);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_BlockCount"), BlockCount);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ScanBlockCount"), ScanBlockCount);
        DispatchComputeShader(RHICmdList, *TopLevelScanCS, 1, 1, 1);
        TopLevelScanCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();

        // Write result

        RHICmdList.BeginComputePass(TEXT("RULWriteScanResult"));
        TShaderMapRef<FRULWriteScanResultCS<ScanDataType>> WriteResultCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        WriteResultCS->SetShader(RHICmdList);
        WriteResultCS->BindSRV(RHICmdList, TEXT("SrcData"), SumBuffer.SRV);
        WriteResultCS->BindUAV(RHICmdList, TEXT("SumData"), ResultBuffer.UAV);
        WriteResultCS->SetParameter(RHICmdList, TEXT("_ElementCount"), SumBufferCount);
        WriteResultCS->SetParameter(RHICmdList, TEXT("_ResultIndex"), ResultIndex);
        DispatchComputeShader(RHICmdList, *WriteResultCS, 1, 1, 1);
        WriteResultCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();
    }

    return ScanBlockCount;
}
