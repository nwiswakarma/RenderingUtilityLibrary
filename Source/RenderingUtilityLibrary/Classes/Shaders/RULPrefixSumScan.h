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

class RENDERINGUTILITYLIBRARY_API FRULPrefixSumScan
{
public:

    const static int32 BLOCK_SIZE  = 128;
    const static int32 BLOCK_SIZE2 = 256;

    FORCEINLINE static int32 GetBlockOffsetForSize(int32 ElementCount)
    {
        return FPlatformMath::RoundUpToPowerOfTwo(FMath::DivideAndRoundUp(ElementCount, BLOCK_SIZE2));
    }

    template<uint32 ScanVectorSize> static const TCHAR* GetScanVectorSizeName()
    {
        switch (ScanVectorSize)
        {
            case 1: return TEXT("uint");
            case 2: return TEXT("uint2");
            case 4: return TEXT("uint4");
            default:
                return TEXT("uint");
        }
    }

    template<uint32 ScanVectorSize> static bool IsValidScanVectorSize()
    {
        bool bIsValidScanVectorSize = false;

        switch (ScanVectorSize)
        {
            case 1:
            case 2:
            case 4:
                bIsValidScanVectorSize = true;
                break;

            default:
                break;
        }

        return bIsValidScanVectorSize;
    }

    template<uint32 ScanVectorSize>
    static int32 ExclusiveScan(
        FShaderResourceViewRHIParamRef SrcDataSRV,
        int32 DataStride,
        int32 ElementCount,
        FRULRWBufferStructured& ScanResult,
        FRULRWBufferStructured& SumBuffer,
        uint32 AdditionalOutputUsage = 0
        );

    static int32 ExclusiveScan1D(FShaderResourceViewRHIParamRef SrcDataSRV, int32 DataStride, int32 ElementCount, FRULRWBufferStructured& ScanResult, FRULRWBufferStructured& SumBuffer, uint32 AdditionalOutputUsage = 0);
    static int32 ExclusiveScan2D(FShaderResourceViewRHIParamRef SrcDataSRV, int32 DataStride, int32 ElementCount, FRULRWBufferStructured& ScanResult, FRULRWBufferStructured& SumBuffer, uint32 AdditionalOutputUsage = 0);
    static int32 ExclusiveScan4D(FShaderResourceViewRHIParamRef SrcDataSRV, int32 DataStride, int32 ElementCount, FRULRWBufferStructured& ScanResult, FRULRWBufferStructured& SumBuffer, uint32 AdditionalOutputUsage = 0);
};
