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

#include "RHI/RULRHIUtilityLibrary.h"

#include "RHICommandList.h"
#include "RHIResources.h"
#include "RHIUtilities.h"

void FRULRHIUtilityLibrary::DrawIndexedPrimitiveRaw(
    FRHICommandList& RHICmdList,
    uint32 MinVertexIndex,
    uint32 NumVertices,
    uint32 NumIndices,
    uint32 NumPrimitives,
    const void* VertexData,
    uint32 VertexDataStride,
    const void* IndexData,
    uint32 IndexDataStride
    )
{
    check(IndexData != nullptr);
    check(VertexData != nullptr);

    const uint32 VertexDataSize = VertexDataStride * NumVertices;
    const uint32 IndexDataSize = IndexDataStride * NumIndices;

    // Construct volatile vertex buffer

    FVertexBufferRHIRef VertexBufferRHI;
    {
        FRHIResourceCreateInfo CreateInfo;
        void* BufferPtr;
        VertexBufferRHI = RHICreateAndLockVertexBuffer(VertexDataSize, BUF_Volatile, CreateInfo, BufferPtr);
        FPlatformMemory::Memcpy(BufferPtr, VertexData, VertexDataSize);
        RHIUnlockVertexBuffer(VertexBufferRHI);
    }

    // Construct volatile index buffer

    FIndexBufferRHIRef IndexBufferRHI;
    {
        FRHIResourceCreateInfo CreateInfo;
        void* BufferPtr;
        IndexBufferRHI = RHICreateAndLockIndexBuffer(IndexDataStride, IndexDataSize, BUF_Volatile, CreateInfo, BufferPtr);
        FPlatformMemory::Memcpy(BufferPtr, IndexData, IndexDataSize);
        RHIUnlockIndexBuffer(IndexBufferRHI);
    }

    // Draw primitives

	RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(IndexBufferRHI, MinVertexIndex, 0, NumVertices, 0, NumPrimitives, 1);

    // Release buffers

	IndexBufferRHI.SafeRelease();
	VertexBufferRHI.SafeRelease();
}
