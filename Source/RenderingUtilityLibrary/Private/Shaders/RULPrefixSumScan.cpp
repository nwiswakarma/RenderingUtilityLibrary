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

template<uint32 ScanVectorSize, uint32 bUseSrc>
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
        OutEnvironment.SetDefine(TEXT("data_t"), FRULPrefixSumScan::GetScanVectorSizeName<ScanVectorSize>());
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

template<uint32 ScanVectorSize, uint32 bAppendSumToData>
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
        OutEnvironment.SetDefine(TEXT("data_t"), FRULPrefixSumScan::GetScanVectorSizeName<ScanVectorSize>());
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

template<uint32 ScanVectorSize>
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
        OutEnvironment.SetDefine(TEXT("data_t"), FRULPrefixSumScan::GetScanVectorSizeName<ScanVectorSize>());
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

template<uint32 ScanVectorSize>
int32 FRULPrefixSumScan::ExclusiveScan(
    FShaderResourceViewRHIParamRef SrcDataSRV,
    int32 DataStride,
    int32 ElementCount,
    FRULRWBufferStructured& ScanResult,
    FRULRWBufferStructured& SumBuffer,
    uint32 AdditionalOutputUsage
    )
{
    check(IsValidScanVectorSize<ScanVectorSize>());

    if (ElementCount < 1)
    {
        return -1;
    }

    enum { VN = ScanVectorSize };

    check(IsInRenderingThread());
    check(DataStride > 0);

    FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();

    int32 blockCount      = FMath::DivideAndRoundUp(ElementCount, BLOCK_SIZE2);
    int32 blockGroupCount = FMath::DivideAndRoundUp(blockCount, BLOCK_SIZE2);

    int32 scanBlockCount      = FPlatformMath::RoundUpToPowerOfTwo(blockCount);
    int32 scanBlockGroupCount = FPlatformMath::RoundUpToPowerOfTwo(blockGroupCount);

    int32 sumBufferCount = scanBlockCount + 1;

    check(blockCount > 0);

    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() ElementCount: %d"), ElementCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() blockCount: %d"), blockCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() blockGroupCount: %d"), blockGroupCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() scanBlockCount: %d"), scanBlockCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() scanBlockGroupCount: %d"), scanBlockGroupCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() sumBufferCount: %d"), sumBufferCount);
    UE_LOG(UntRUL,Warning, TEXT("RULPrefixSumScan::ExclusiveScan() BLOCK_SIZE2: %d"), BLOCK_SIZE2);

    // Clear and initialize output buffers

    // Initialize scan result

    ScanResult.Release();
    ScanResult.Initialize(DataStride, ElementCount, AdditionalOutputUsage);

    // Initialize sum buffer

    SumBuffer.Release();

    switch (ScanVectorSize)
    {
        case 1:
        {
            typedef TResourceArray<FRULAlignedUint, VERTEXBUFFER_ALIGNMENT> FResourceType;
            FResourceType DefaultSumData(false);
            DefaultSumData.SetNumZeroed(sumBufferCount);
            SumBuffer.Initialize(DataStride, sumBufferCount, &DefaultSumData, AdditionalOutputUsage);
        }
        break;

        case 2:
        {
            typedef TResourceArray<FRULAlignedUintPoint, VERTEXBUFFER_ALIGNMENT> FResourceType;
            FResourceType DefaultSumData(false);
            DefaultSumData.SetNumZeroed(sumBufferCount);
            SumBuffer.Initialize(DataStride, sumBufferCount, &DefaultSumData, AdditionalOutputUsage);
        }
        break;

        case 4:
        {
            typedef TResourceArray<FRULAlignedUintVector4, VERTEXBUFFER_ALIGNMENT> FResourceType;
            FResourceType DefaultSumData(false);
            DefaultSumData.SetNumZeroed(sumBufferCount);
            SumBuffer.Initialize(DataStride, sumBufferCount, &DefaultSumData, AdditionalOutputUsage);
        }
        break;

        default:
        {
            typedef TResourceArray<FRULAlignedUint, VERTEXBUFFER_ALIGNMENT> FResourceType;
            FResourceType DefaultSumData(false);
            DefaultSumData.SetNumZeroed(sumBufferCount);
            SumBuffer.Initialize(DataStride, sumBufferCount, &DefaultSumData, AdditionalOutputUsage);
        }
        break;
    }

    // Local scan kernel

    TShaderMapRef<FRULPrefixSumLocalScanCS<VN,1>> LocalScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
    LocalScanCS->SetShader(RHICmdList);
    LocalScanCS->BindSRV(RHICmdList, TEXT("SrcData"), SrcDataSRV);
    LocalScanCS->BindUAV(RHICmdList, TEXT("DstData"), ScanResult.UAV);
    LocalScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
    LocalScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
    DispatchComputeShader(RHICmdList, *LocalScanCS, blockCount, 1, 1);
    LocalScanCS->UnbindBuffers(RHICmdList);

    if (blockGroupCount > 1)
    {
        FRULRWBufferStructured BlockSumData;
        BlockSumData.Initialize(DataStride, (scanBlockGroupCount+1), BUF_Static);

        // Block sum scan

        TShaderMapRef<FRULPrefixSumLocalScanCS<VN,0>> BlockScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        BlockScanCS->SetShader(RHICmdList);
        BlockScanCS->BindUAV(RHICmdList, TEXT("DstData"), SumBuffer.UAV);
        BlockScanCS->BindUAV(RHICmdList, TEXT("SumData"), BlockSumData.UAV);
        BlockScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
        DispatchComputeShader(RHICmdList, *BlockScanCS, blockGroupCount, 1, 1);
        BlockScanCS->UnbindBuffers(RHICmdList);

        // Block sum top level scan

        TShaderMapRef<FRULPrefixSumTopLevelScanCS<VN,1>> TopLevelScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        TopLevelScanCS->SetShader(RHICmdList);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("DstData"), SumBuffer.UAV);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("SumData"), BlockSumData.UAV);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"),   scanBlockCount);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_BlockCount"),     blockGroupCount);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ScanBlockCount"), scanBlockGroupCount);
        DispatchComputeShader(RHICmdList, *TopLevelScanCS, 1, 1, 1);
        TopLevelScanCS->UnbindBuffers(RHICmdList);

        // Add block offset

        TShaderMapRef<FRULPrefixSumAddOffsetCS<VN>> AddOffsetCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));

        AddOffsetCS->SetShader(RHICmdList);
        AddOffsetCS->BindUAV(RHICmdList, TEXT("DstData"), SumBuffer.UAV);
        AddOffsetCS->BindUAV(RHICmdList, TEXT("SumData"), BlockSumData.UAV);
        AddOffsetCS->SetParameter(RHICmdList, TEXT("_ElementCount"), blockCount);
        DispatchComputeShader(RHICmdList, *AddOffsetCS, (blockGroupCount-1), 1, 1);
        AddOffsetCS->UnbindBuffers(RHICmdList);

        AddOffsetCS->SetShader(RHICmdList);
        AddOffsetCS->BindUAV(RHICmdList, TEXT("DstData"), ScanResult.UAV);
        AddOffsetCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
        AddOffsetCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
        DispatchComputeShader(RHICmdList, *AddOffsetCS, (blockCount-1), 1, 1);
        AddOffsetCS->UnbindBuffers(RHICmdList);
    }
    else
    {
        // Top level scan

        TShaderMapRef<FRULPrefixSumTopLevelScanCS<VN,0>> TopLevelScanCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        TopLevelScanCS->SetShader(RHICmdList);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("DstData"), ScanResult.UAV);
        TopLevelScanCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ElementCount"),   0);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_BlockCount"),     blockCount);
        TopLevelScanCS->SetParameter(RHICmdList, TEXT("_ScanBlockCount"), scanBlockCount);
        DispatchComputeShader(RHICmdList, *TopLevelScanCS, 1, 1, 1);
        TopLevelScanCS->UnbindBuffers(RHICmdList);

        // Add block offset to local scan

        if (blockCount > 1)
        {
            TShaderMapRef<FRULPrefixSumAddOffsetCS<VN>> AddOffsetCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            AddOffsetCS->SetShader(RHICmdList);
            AddOffsetCS->BindUAV(RHICmdList, TEXT("DstData"), ScanResult.UAV);
            AddOffsetCS->BindUAV(RHICmdList, TEXT("SumData"), SumBuffer.UAV);
            AddOffsetCS->SetParameter(RHICmdList, TEXT("_ElementCount"), ElementCount);
            DispatchComputeShader(RHICmdList, *AddOffsetCS, (blockCount-1), 1, 1);
            AddOffsetCS->UnbindBuffers(RHICmdList);
        }
    }

    return scanBlockCount;
}

int32 FRULPrefixSumScan::ExclusiveScan1D(
    FShaderResourceViewRHIParamRef SrcDataSRV,
    int32 DataStride,
    int32 ElementCount,
    FRULRWBufferStructured& ScanResult,
    FRULRWBufferStructured& SumBuffer,
    uint32 AdditionalOutputUsage
    )
{
    return ExclusiveScan<1>(SrcDataSRV, DataStride, ElementCount, ScanResult, SumBuffer, AdditionalOutputUsage);
}

int32 FRULPrefixSumScan::ExclusiveScan2D(
    FShaderResourceViewRHIParamRef SrcDataSRV,
    int32 DataStride,
    int32 ElementCount,
    FRULRWBufferStructured& ScanResult,
    FRULRWBufferStructured& SumBuffer,
    uint32 AdditionalOutputUsage
    )
{
    return ExclusiveScan<2>(SrcDataSRV, DataStride, ElementCount, ScanResult, SumBuffer, AdditionalOutputUsage);
}

int32 FRULPrefixSumScan::ExclusiveScan4D(
    FShaderResourceViewRHIParamRef SrcDataSRV,
    int32 DataStride,
    int32 ElementCount,
    FRULRWBufferStructured& ScanResult,
    FRULRWBufferStructured& SumBuffer,
    uint32 AdditionalOutputUsage
    )
{
    return ExclusiveScan<4>(SrcDataSRV, DataStride, ElementCount, ScanResult, SumBuffer, AdditionalOutputUsage);
}
