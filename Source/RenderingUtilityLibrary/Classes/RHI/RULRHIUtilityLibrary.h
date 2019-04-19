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

class FRHICommandList;

class RENDERINGUTILITYLIBRARY_API FRULRHIUtilityLibrary
{
public:

    static void DrawIndexedPrimitiveRaw(
        FRHICommandList& RHICmdList,
        uint32 MinVertexIndex,
        uint32 NumVertices,
        uint32 NumIndices,
        uint32 NumPrimitives,
        const void* VertexData,
        uint32 VertexDataStride,
        const void* IndexData,
        uint32 IndexDataStride,
        const void* ColorData = nullptr,
        uint32 ColorDataStride = 0
        );

    template<class FVertexType>
    static void DrawTriangleList(
        FRHICommandList& RHICmdList,
        const TArray<FVertexType>& VertexData,
        const TArray<int32>& IndexData,
        const TArray<FColor>* ColorData = nullptr
        )
    {
        bool bUseColorData = (ColorData && (ColorData->Num() == VertexData.Num()));
        DrawIndexedPrimitiveRaw(
            RHICmdList,
            0,
            VertexData.Num(),
            IndexData.Num(),
            IndexData.Num() / 3,
            VertexData.GetData(),
            VertexData.GetTypeSize(),
            IndexData.GetData(),
            IndexData.GetTypeSize(),
            bUseColorData ? ColorData->GetData()     : nullptr,
            bUseColorData ? ColorData->GetTypeSize() : 0
            );
    }
};
