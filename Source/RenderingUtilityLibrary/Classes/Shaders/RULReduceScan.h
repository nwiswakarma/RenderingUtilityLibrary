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
#include "RHIResources.h"
#include "RHI/RULRHIBuffer.h"

class FRHICommandListImmediate;

class RENDERINGUTILITYLIBRARY_API FRULReduceScan
{
public:

    enum FScanDataType
    {
        SDT_UINT1 = 0x01,
        SDT_UINT2 = 0x02,
        SDT_UINT4 = 0x04,
        SDT_FLOAT1 = 0x11,
        SDT_FLOAT2 = 0x12,
        SDT_FLOAT4 = 0x14
    };

    enum FScanOpType
    {
        SOT_Max = 0,
        SOT_Min = 1
    };

    const static int32 BLOCK_SIZE  = 128;
    const static int32 BLOCK_SIZE2 = 256;

    const static int32 TEX_BLOCK  = 8;
    const static int32 TEX_BLOCK2 = 16;

    FORCEINLINE static int32 GetBlockOffsetForSize(int32 ElementCount)
    {
        return FPlatformMath::RoundUpToPowerOfTwo(FMath::DivideAndRoundUp(ElementCount, BLOCK_SIZE2));
    }

    template<uint32 ScanDataType>
    static FString GetScanDataTypeName()
    {
        FString DataType(TEXT("uint"));
        int32 Dimension = 1;
        switch (ScanDataType & 0x0F)
        {
            case 1: Dimension = 1; break;
            case 2: Dimension = 2; break;
            case 4: Dimension = 4; break;
        }
        switch ((ScanDataType>>4) & 0x0F)
        {
            case 0: DataType = TEXT("uint"); break;
            case 1: DataType = TEXT("float"); break;
        }
        return DataType + FString::FromInt(Dimension);
    }

    template<uint32 ScanDataType>
    static bool IsValidScanDataType()
    {
        switch (ScanDataType)
        {
            case SDT_UINT1:
            case SDT_UINT2:
            case SDT_UINT4:
            case SDT_FLOAT1:
            case SDT_FLOAT2:
            case SDT_FLOAT4:
                return true;
        }
        return false;
    }

    template<uint32 ScanDataType>
    static bool IsValidScanOpType()
    {
        switch (ScanDataType)
        {
            case SOT_Max:
            case SOT_Min:
                return true;
        }
        return false;
    }

    template<uint32 ScanDataType, uint32 ScanOpType = SOT_Max>
    static int32 Reduce(
        FRHICommandListImmediate& RHICmdList,
        FShaderResourceViewRHIParamRef SrcDataSRV,
        FRULRWBufferStructured& ResultBuffer,
        int32 DataStride,
        int32 ElementCount,
        uint32 AdditionalOutputUsage = 0
        );

    template<uint32 ScanOpType>
    static int32 ReduceTexture(
        FRHICommandListImmediate& RHICmdList,
        FTextureRHIParamRef SourceTexture,
        FRULRWBufferStructured& ResultBuffer,
        FIntPoint Dimension,
        int32 ResultIndex = 0,
        bool bInitializeResultBuffer = true,
        uint32 AdditionalOutputUsage = 0
        );
};
