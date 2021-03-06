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

#pragma once

#include "CoreMinimal.h"
#include "RHI/RULRHIBuffer.h"

class FRHICommandListImmediate;

class RENDERINGUTILITYLIBRARY_API FRULPrefixSumScan
{
public:

    const static int32 BLOCK_SIZE  = 128;
    const static int32 BLOCK_SIZE2 = 256;

    FORCEINLINE static int32 GetBlockOffsetForSize(int32 ElementCount)
    {
        return FPlatformMath::RoundUpToPowerOfTwo(FMath::DivideAndRoundUp(ElementCount, BLOCK_SIZE2));
    }

    template<uint32 ScanDimension>
    static const TCHAR* GetScanDimensionName()
    {
        switch (ScanDimension)
        {
            case 1: return TEXT("uint");
            case 2: return TEXT("uint2");
            case 4: return TEXT("uint4");
            default:
                return TEXT("uint");
        }
    }

    template<uint32 ScanDimension>
    static bool IsValidScanDimension()
    {
        switch (ScanDimension)
        {
            case 1:
            case 2:
            case 4:
                return true;

            default:
                return false;
        }
    }

    template<uint32 ScanDimension>
    static int32 ExclusiveScan(
        FRHICommandListImmediate& RHICmdList,
        FShaderResourceViewRHIParamRef SrcDataSRV,
        int32 DataStride,
        int32 ElementCount,
        FRULRWBufferStructured& ScanResult,
        FRULRWBufferStructured& SumBuffer,
        uint32 AdditionalOutputUsage = 0
        );
};

#define DECLARE_SCAN_PARAMS\
    FRHICommandListImmediate& RHICmdList,\
    FShaderResourceViewRHIParamRef SrcDataSRV,\
    int32 DataStride,\
    int32 ElementCount,\
    FRULRWBufferStructured& ScanResult,\
    FRULRWBufferStructured& SumBuffer,\
    uint32 AdditionalOutputUsage = 0

#define DECLARE_SCAN_ARGS\
    RHICmdList,\
    SrcDataSRV,\
    DataStride,\
    ElementCount,\
    ScanResult,\
    SumBuffer,\
    AdditionalOutputUsage

#define IMPLEMENT_SCAN_FUNCTION(VN)\
    static int32 ExclusiveScan(DECLARE_SCAN_PARAMS)\
    {\
        return FRULPrefixSumScan::ExclusiveScan<VN>(DECLARE_SCAN_ARGS);\
    }

#define IMPLEMENT_SCAN_TYPE(VN)                             \
class RENDERINGUTILITYLIBRARY_API FRULPrefixSumScan##VN##D  \
{                                                           \
public:                                                     \
    IMPLEMENT_SCAN_FUNCTION(VN)                             \
};

IMPLEMENT_SCAN_TYPE(1)
IMPLEMENT_SCAN_TYPE(2)
IMPLEMENT_SCAN_TYPE(4)

#undef IMPLEMENT_SCAN_TYPE
#undef IMPLEMENT_SCAN_FUNCTION
#undef DECLARE_SCAN_ARGS
#undef DECLARE_SCAN_PARAMS
