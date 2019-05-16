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

#include "Shaders/RULPrefixSumScan.h"

#include "ShaderParameters.h"
#include "ShaderCore.h"
#include "UniformBuffer.h"

#include "RHI/RULAlignedTypes.h"
#include "Shaders/RULShaderDefinitions.h"

template<uint32 ScanDimension, uint32 bUseSrc>
class FRULPrefixSumLocalScanCS : public FRULBaseComputeShader<>
{
    typedef FRULBaseComputeShader<> FBaseType;

    DECLARE_SHADER_TYPE(FRULPrefixSumLocalScanCS, Global);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return RHISupportsComputeShaders(Parameters.Platform);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FBaseType::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
        OutEnvironment.SetDefine(TEXT("data_t"), FRULPrefixSumScan::GetScanDimensionName<ScanDimension>());
        OutEnvironment.SetDefine(TEXT("LOCAL_SCAN_USE_SRC"), bUseSrc);
    }

    RUL_DECLARE_SHADER_CONSTRUCTOR_SERIALIZER(FRULPrefixSumLocalScanCS)

    RUL_DECLARE_SHADER_PARAMETERS_1(
        SRV,
        FShaderResourceParameter,
        FResourceId,
        "SrcData", SrcData
        )

    RUL_DECLARE_SHADER_PARAMETERS_2(
        UAV,
        FShaderResourceParameter,
        FResourceId,
        "DstData", DstData,
        "SumData", SumData
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Value,
        FShaderParameter,
        FParameterId,
        "_ElementCount", Params_ElementCount
        )
};

template<uint32 ScanDimension, uint32 bAppendSumToData>
class FRULPrefixSumTopLevelScanCS : public FRULBaseComputeShader<>
{
    typedef FRULBaseComputeShader<> FBaseType;

    DECLARE_SHADER_TYPE(FRULPrefixSumTopLevelScanCS, Global);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return RHISupportsComputeShaders(Parameters.Platform);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FBaseType::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
        OutEnvironment.SetDefine(TEXT("data_t"), FRULPrefixSumScan::GetScanDimensionName<ScanDimension>());
        OutEnvironment.SetDefine(TEXT("TOP_LEVEL_SCAN_APPEND_SUM_TO_DATA"), bAppendSumToData ? 1 : 0);
    }

    RUL_DECLARE_SHADER_CONSTRUCTOR_SERIALIZER(FRULPrefixSumTopLevelScanCS)

    RUL_DECLARE_SHADER_PARAMETERS_0(SRV,,)

    RUL_DECLARE_SHADER_PARAMETERS_2(
        UAV,
        FShaderResourceParameter,
        FResourceId,
        "DstData", DstData,
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

template<uint32 ScanDimension>
class FRULPrefixSumAddOffsetCS : public FRULBaseComputeShader<>
{
    typedef FRULBaseComputeShader<> FBaseType;

    DECLARE_SHADER_TYPE(FRULPrefixSumAddOffsetCS, Global);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return RHISupportsComputeShaders(Parameters.Platform);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FBaseType::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
        OutEnvironment.SetDefine(TEXT("data_t"), FRULPrefixSumScan::GetScanDimensionName<ScanDimension>());
    }

    RUL_DECLARE_SHADER_CONSTRUCTOR_SERIALIZER(FRULPrefixSumAddOffsetCS)

    RUL_DECLARE_SHADER_PARAMETERS_0(SRV,,)

    RUL_DECLARE_SHADER_PARAMETERS_2(
        UAV,
        FShaderResourceParameter,
        FResourceId,
        "DstData", DstData,
        "SumData", SumData
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Value,
        FShaderParameter,
        FParameterId,
        "_ElementCount", Params_ElementCount
        )
};

#define SHADER_FILENAME "/Plugin/RenderingUtilityLibrary/Private/RULPrefixSumScanCS.usf"
#define SCAN_KERNEL1(N,VN)   N<VN>
#define SCAN_KERNEL2(N,VN,T) N<VN,T>

#define IMPLEMENT_SCAN_SHADER(VN) \
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL2(FRULPrefixSumLocalScanCS,VN,0), TEXT(SHADER_FILENAME), TEXT("LocalScanKernel"), SF_Compute);\
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL2(FRULPrefixSumLocalScanCS,VN,1), TEXT(SHADER_FILENAME), TEXT("LocalScanKernel"), SF_Compute);\
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL2(FRULPrefixSumTopLevelScanCS,VN,0), TEXT(SHADER_FILENAME), TEXT("TopLevelScanKernel"), SF_Compute);\
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL2(FRULPrefixSumTopLevelScanCS,VN,1), TEXT(SHADER_FILENAME), TEXT("TopLevelScanKernel"), SF_Compute);\
IMPLEMENT_SHADER_TYPE(template<>, SCAN_KERNEL1(FRULPrefixSumAddOffsetCS,VN), TEXT(SHADER_FILENAME), TEXT("AddOffsetKernel"), SF_Compute);

IMPLEMENT_SCAN_SHADER(1)
IMPLEMENT_SCAN_SHADER(2)
IMPLEMENT_SCAN_SHADER(4)

#undef IMPLEMENT_SCAN_SHADER
#undef SCAN_KERNEL1
#undef SCAN_KERNEL2
#undef SHADER_FILENAME

template<uint32 ScanDimension>
int32 FRULPrefixSumScan::ExclusiveScan(
    FRHICommandListImmediate& RHICmdList,
    FShaderResourceViewRHIParamRef SrcDataSRV,
    int32 DataStride,
    int32 ElementCount,
    FRULRWBufferStructured& ScanResult,
    FRULRWBufferStructured& SumBuffer,
    uint32 AdditionalOutputUsage
    )
{
    check(IsInRenderingThread());
    check(IsValidScanDimension<ScanDimension>());

    if (ElementCount < 1)
    {
        return -1;
    }

    check(IsInRenderingThread());
    check(DataStride > 0);

    int32 BlockCount      = FMath::DivideAndRoundUp(ElementCount, BLOCK_SIZE2);
    int32 BlockGroupCount = FMath::DivideAndRoundUp(BlockCount, BLOCK_SIZE2);

    int32 ScanBlockCount      = FPlatformMath::RoundUpToPowerOfTwo(BlockCount);
    int32 ScanBlockGroupCount = FPlatformMath::RoundUpToPowerOfTwo(BlockGroupCount);

    int32 SumBufferCount = ScanBlockCount + 1;

    check(BlockCount > 0);

    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() ElementCount: %d"), ElementCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() BlockCount: %d"), BlockCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() BlockGroupCount: %d"), BlockGroupCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() ScanBlockCount: %d"), ScanBlockCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() ScanBlockGroupCount: %d"), ScanBlockGroupCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() SumBufferCount: %d"), SumBufferCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() BLOCK_SIZE2: %d"), BLOCK_SIZE2);

    // Clear and initialize output buffers

    // Initialize scan result

    ScanResult.Release();
    ScanResult.Initialize(DataStride, ElementCount, AdditionalOutputUsage);

    // Initialize sum buffer

    SumBuffer.Release();

    {
        TResourceArray<uint8, VERTEXBUFFER_ALIGNMENT> DefaultSumData(false);
        DefaultSumData.SetNumZeroed(DataStride * SumBufferCount);
        SumBuffer.Initialize(DataStride, SumBufferCount, &DefaultSumData, AdditionalOutputUsage);
    }

    // Local scan kernel

    RHICmdList.BeginComputePass(TEXT("RULPrefixSumLocalScan"));
    TShaderMapRef<FRULPrefixSumLocalScanCS<ScanDimension,1>> LocalScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
    LocalScanCS->SetShader(RHICmdList);
    LocalScanCS->BindSRV(RHICmdList, TEXT("SrcData"), SrcDataSRV);
    LocalScanCS->BindUAV(RHICmdList, TEXT("DstData"), ScanResult.UAV);
    LocalScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
    LocalScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
    DispatchComputeShader(RHICmdList, *LocalScanCS, BlockCount, 1, 1);
    LocalScanCS->UnbindBuffers(RHICmdList);
    RHICmdList.EndComputePass();

    if (BlockGroupCount > 1)
    {
        FRULRWBufferStructured BlockSumData;
        BlockSumData.Initialize(DataStride, (ScanBlockGroupCount+1), BUF_Static);

        // Block sum scan

        RHICmdList.BeginComputePass(TEXT("RULPrefixSumLocalScan"));
        TShaderMapRef<FRULPrefixSumLocalScanCS<ScanDimension,0>> BlockScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        BlockScanCS->SetShader(RHICmdList);
        BlockScanCS->BindUAV(RHICmdList, TEXT("DstData"), SumBuffer.UAV);
        BlockScanCS->BindUAV(RHICmdList, TEXT("SumData"), BlockSumData.UAV);
        BlockScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
        DispatchComputeShader(RHICmdList, *BlockScanCS, BlockGroupCount, 1, 1);
        BlockScanCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();

        // Block sum top level scan

        RHICmdList.BeginComputePass(TEXT("RULPrefixSumTopLevelScan"));
        TShaderMapRef<FRULPrefixSumTopLevelScanCS<ScanDimension,1>> TopLevelScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        TopLevelScanCS->SetShader(RHICmdList);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("DstData"), SumBuffer.UAV);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("SumData"), BlockSumData.UAV);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"),   ScanBlockCount);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_BlockCount"),     BlockGroupCount);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ScanBlockCount"), ScanBlockGroupCount);
        DispatchComputeShader(RHICmdList, *TopLevelScanCS, 1, 1, 1);
        TopLevelScanCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();

        // Add block offset

        TShaderMapRef<FRULPrefixSumAddOffsetCS<ScanDimension>> AddOffsetCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));

        RHICmdList.BeginComputePass(TEXT("RULPrefixSumAddOffset"));
        AddOffsetCS->SetShader(RHICmdList);
        AddOffsetCS->BindUAV(RHICmdList, TEXT("DstData"), SumBuffer.UAV);
        AddOffsetCS->BindUAV(RHICmdList, TEXT("SumData"), BlockSumData.UAV);
        AddOffsetCS->SetParameter(RHICmdList, TEXT("_ElementCount"), BlockCount);
        DispatchComputeShader(RHICmdList, *AddOffsetCS, (BlockGroupCount-1), 1, 1);
        AddOffsetCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();

        RHICmdList.BeginComputePass(TEXT("RULPrefixSumAddOffset"));
        AddOffsetCS->SetShader(RHICmdList);
        AddOffsetCS->BindUAV(RHICmdList, TEXT("DstData"), ScanResult.UAV);
        AddOffsetCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
        AddOffsetCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
        DispatchComputeShader(RHICmdList, *AddOffsetCS, (BlockCount-1), 1, 1);
        AddOffsetCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();
    }
    else
    {
        // Top level scan

        RHICmdList.BeginComputePass(TEXT("RULPrefixSumTopLevelScan"));
        TShaderMapRef<FRULPrefixSumTopLevelScanCS<ScanDimension,0>> TopLevelScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        TopLevelScanCS->SetShader(RHICmdList);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("DstData"), ScanResult.UAV);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"),   0);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_BlockCount"),     BlockCount);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ScanBlockCount"), ScanBlockCount);
        DispatchComputeShader(RHICmdList, *TopLevelScanCS, 1, 1, 1);
        TopLevelScanCS->UnbindBuffers(RHICmdList);
        RHICmdList.EndComputePass();

        // Add block offset to local scan

        if (BlockCount > 1)
        {
            RHICmdList.BeginComputePass(TEXT("RULPrefixSumAddOffset"));
            TShaderMapRef<FRULPrefixSumAddOffsetCS<ScanDimension>> AddOffsetCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            AddOffsetCS->SetShader(RHICmdList);
            AddOffsetCS->BindUAV(RHICmdList, TEXT("DstData"), ScanResult.UAV);
            AddOffsetCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
            AddOffsetCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
            DispatchComputeShader(RHICmdList, *AddOffsetCS, (BlockCount-1), 1, 1);
            AddOffsetCS->UnbindBuffers(RHICmdList);
            RHICmdList.EndComputePass();
        }
    }

    return ScanBlockCount;
}
