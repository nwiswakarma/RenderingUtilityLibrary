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
#include "RHIUtilities.h"

// Encapsulates a GPU read/write buffer with its UAV and SRV
struct FRULRWBuffer
{
	FVertexBufferRHIRef Buffer;
	FUnorderedAccessViewRHIRef UAV;
	FShaderResourceViewRHIRef SRV;
	uint32 NumBytes;

	FRULRWBuffer()
		: NumBytes(0)
	{
    }

	~FRULRWBuffer()
	{
		Release();
	}

    FORCEINLINE bool IsValid() const
    {
        return NumBytes > 0;
    }

	// @param AdditionalUsage passed down to RHICreateVertexBuffer(), get combined with "BUF_UnorderedAccess | BUF_ShaderResource" e.g. BUF_Static
    void Initialize(
        uint32 BytesPerElement,
        uint32 NumElements,
        EPixelFormat Format,
        uint32 AdditionalUsage = 0,
        const TCHAR* InDebugName = NULL
        )
	{
		check( GMaxRHIFeatureLevel == ERHIFeatureLevel::SM5 
			|| IsVulkanPlatform(GMaxRHIShaderPlatform) 
			|| IsMetalPlatform(GMaxRHIShaderPlatform)
			|| (GMaxRHIFeatureLevel == ERHIFeatureLevel::ES3_1 && GSupportsResourceView)
		);

		// Provide a debug name if using Fast VRAM so the allocators diagnostics will work
		ensure(!((AdditionalUsage & BUF_FastVRAM) && !InDebugName));

		NumBytes = BytesPerElement * NumElements;
		FRHIResourceCreateInfo CreateInfo;
		CreateInfo.DebugName = InDebugName;
		Buffer = RHICreateVertexBuffer(NumBytes, BUF_UnorderedAccess | BUF_ShaderResource | AdditionalUsage, CreateInfo);
		UAV = RHICreateUnorderedAccessView(Buffer, Format);
		SRV = RHICreateShaderResourceView(Buffer, BytesPerElement, Format);
	}

	// @param AdditionalUsage passed down to RHICreateVertexBuffer(), get combined with "BUF_UnorderedAccess | BUF_ShaderResource" e.g. BUF_Static
	void Initialize(
        uint32 BytesPerElement,
        uint32 NumElements,
        EPixelFormat Format,
        FResourceArrayInterface* InitResourceArrayPtr,
        uint32 AdditionalUsage = 0,
        const TCHAR* InDebugName = NULL
        )
	{
		check( GMaxRHIFeatureLevel == ERHIFeatureLevel::SM5 
			|| IsVulkanPlatform(GMaxRHIShaderPlatform) 
			|| IsMetalPlatform(GMaxRHIShaderPlatform)
			|| (GMaxRHIFeatureLevel == ERHIFeatureLevel::ES3_1 && GSupportsResourceView)
		);

		// Provide a debug name if using Fast VRAM so the allocators diagnostics will work
		ensure(!((AdditionalUsage & BUF_FastVRAM) && !InDebugName));

		NumBytes = BytesPerElement * NumElements;
		FRHIResourceCreateInfo CreateInfo(InitResourceArrayPtr);
		CreateInfo.DebugName = InDebugName;
		Buffer = RHICreateVertexBuffer(NumBytes, BUF_UnorderedAccess | BUF_ShaderResource | AdditionalUsage, CreateInfo);
		UAV = RHICreateUnorderedAccessView(Buffer, Format);
		SRV = RHICreateShaderResourceView(Buffer, BytesPerElement, Format);
	}

	void Release()
	{
		int32 BufferRefCount = Buffer ? Buffer->GetRefCount() : -1;

		if (BufferRefCount == 1)
		{
			DiscardTransientResource();
		}

		NumBytes = 0;
		Buffer.SafeRelease();
		UAV.SafeRelease();
		SRV.SafeRelease();
	}

	void AcquireTransientResource()
	{
		RHIAcquireTransientResource(Buffer);
	}
	void DiscardTransientResource()
	{
		RHIDiscardTransientResource(Buffer);
	}

    void* LockReadOnly()
    {
        check(IsValid());
        return RHILockVertexBuffer(Buffer, 0, Buffer->GetSize(), RLM_ReadOnly);
    }

    void Unlock()
    {
        check(IsValid());
        return RHIUnlockVertexBuffer(Buffer);
    }
};

// Encapsulates a GPU read buffer with its SRV
struct FRULReadBuffer
{
	FVertexBufferRHIRef Buffer;
	FShaderResourceViewRHIRef SRV;
	uint32 NumBytes;

	FRULReadBuffer()
		: NumBytes(0)
	{
    }

	~FRULReadBuffer()
	{
		Release();
	}

    FORCEINLINE bool IsValid() const
    {
        return NumBytes > 0;
    }

	// @param AdditionalUsage passed down to RHICreateVertexBuffer(), get combined with "BUF_UnorderedAccess | BUF_ShaderResource" e.g. BUF_Static
    void Initialize(
        uint32 BytesPerElement,
        uint32 NumElements,
        EPixelFormat Format,
        uint32 AdditionalUsage = 0,
        const TCHAR* InDebugName = NULL
        )
	{
        check(GSupportsResourceView);

		NumBytes = BytesPerElement * NumElements;
		FRHIResourceCreateInfo CreateInfo;
		CreateInfo.DebugName = InDebugName;
		Buffer = RHICreateVertexBuffer(NumBytes, BUF_UnorderedAccess | BUF_ShaderResource | AdditionalUsage, CreateInfo);
		SRV = RHICreateShaderResourceView(Buffer, BytesPerElement, Format);
	}

	// @param AdditionalUsage passed down to RHICreateVertexBuffer(), get combined with "BUF_UnorderedAccess | BUF_ShaderResource" e.g. BUF_Static
	void Initialize(
        uint32 BytesPerElement,
        uint32 NumElements,
        EPixelFormat Format,
        FResourceArrayInterface* InitResourceArrayPtr,
        uint32 AdditionalUsage = 0,
        const TCHAR* InDebugName = NULL
        )
	{
        check(GSupportsResourceView);

		NumBytes = BytesPerElement * NumElements;
		FRHIResourceCreateInfo CreateInfo(InitResourceArrayPtr);
		CreateInfo.DebugName = InDebugName;
		Buffer = RHICreateVertexBuffer(NumBytes, BUF_UnorderedAccess | BUF_ShaderResource | AdditionalUsage, CreateInfo);
		SRV = RHICreateShaderResourceView(Buffer, BytesPerElement, Format);
	}

	void AcquireTransientResource()
	{
		RHIAcquireTransientResource(Buffer);
	}
	void DiscardTransientResource()
	{
		RHIDiscardTransientResource(Buffer);
	}

	void Release()
	{
		int32 BufferRefCount = Buffer ? Buffer->GetRefCount() : -1;

		if (BufferRefCount == 1)
		{
			DiscardTransientResource();
		}

		NumBytes = 0;
		Buffer.SafeRelease();
		SRV.SafeRelease();
	}
};

// Encapsulates a GPU read/write structured buffer with its UAV and SRV
struct FRULRWBufferStructured
{
	FStructuredBufferRHIRef Buffer;
	FUnorderedAccessViewRHIRef UAV;
	FShaderResourceViewRHIRef SRV;
	uint32 NumBytes;

	FRULRWBufferStructured()
        : NumBytes(0)
    {
    }

	~FRULRWBufferStructured()
	{
		Release();
	}

    FORCEINLINE bool IsValid() const
    {
        return NumBytes > 0;
    }

    FORCEINLINE bool IsValidIndex(int32 Index) const
    {
        return IsValid() && Index >= 0 && Index < GetNumElements();
    }

    FORCEINLINE int32 GetNumElements() const
    {
        return IsValid() ? Buffer->GetSize() / Buffer->GetStride() : 0;
    }

    void Initialize(
        uint32 BytesPerElement,
        uint32 NumElements,
        uint32 AdditionalUsage = 0,
        const TCHAR* InDebugName = NULL,
        bool bUseUavCounter = false,
        bool bAppendBuffer = false
        )
	{
		check(GMaxRHIFeatureLevel == ERHIFeatureLevel::SM5);

		// Provide a debug name if using Fast VRAM so the allocators diagnostics will work
		ensure(!((AdditionalUsage & BUF_FastVRAM) && !InDebugName));

		NumBytes = BytesPerElement * NumElements;
		FRHIResourceCreateInfo CreateInfo;
		CreateInfo.DebugName = InDebugName;
		Buffer = RHICreateStructuredBuffer(BytesPerElement, NumBytes, BUF_UnorderedAccess | BUF_ShaderResource | AdditionalUsage, CreateInfo);
		UAV = RHICreateUnorderedAccessView(Buffer, bUseUavCounter, bAppendBuffer);
		SRV = RHICreateShaderResourceView(Buffer);
	}

    void Initialize(
        uint32 BytesPerElement,
        uint32 NumElements,
        FResourceArrayInterface* InitResourceArrayPtr,
        uint32 AdditionalUsage = 0,
        const TCHAR* InDebugName = NULL,
        bool bUseUavCounter = false,
        bool bAppendBuffer = false
        )
	{
		check(GMaxRHIFeatureLevel == ERHIFeatureLevel::SM5);
		// Provide a debug name if using Fast VRAM so the allocators diagnostics will work
		ensure(!((AdditionalUsage & BUF_FastVRAM) && !InDebugName));

		NumBytes = BytesPerElement * NumElements;
		FRHIResourceCreateInfo CreateInfo(InitResourceArrayPtr);
		CreateInfo.DebugName = InDebugName;
		Buffer = RHICreateStructuredBuffer(BytesPerElement, NumBytes, BUF_UnorderedAccess | BUF_ShaderResource | AdditionalUsage, CreateInfo);
		UAV = RHICreateUnorderedAccessView(Buffer, bUseUavCounter, bAppendBuffer);
		SRV = RHICreateShaderResourceView(Buffer);
	}

	void Release()
	{
		int32 BufferRefCount = Buffer ? Buffer->GetRefCount() : -1;

		if (BufferRefCount == 1)
		{
			DiscardTransientResource();
		}

		NumBytes = 0;
		Buffer.SafeRelease();
		UAV.SafeRelease();
		SRV.SafeRelease();
	}

	void AcquireTransientResource()
	{
		RHIAcquireTransientResource(Buffer);
	}
	void DiscardTransientResource()
	{
		RHIDiscardTransientResource(Buffer);
	}

    void* LockReadOnly()
    {
        check(IsValid());
        return RHILockStructuredBuffer(Buffer, 0, Buffer->GetSize(), RLM_ReadOnly);
    }

    void Unlock()
    {
        check(IsValid());
        return RHIUnlockStructuredBuffer(Buffer);
    }
};

// Encapsulates a GPU read ByteAddress buffer with its SRV
struct FRULByteAddressBuffer
{
	FStructuredBufferRHIRef Buffer;
	FShaderResourceViewRHIRef SRV;
	uint32 NumBytes;

	FRULByteAddressBuffer(): NumBytes(0) {}

    FORCEINLINE bool IsValid() const
    {
        return NumBytes > 0;
    }

	void Initialize(uint32 InNumBytes, uint32 AdditionalUsage = 0)
	{
		NumBytes = InNumBytes;
		check(GMaxRHIFeatureLevel == ERHIFeatureLevel::SM5);
		check(NumBytes % 4 == 0);
		FRHIResourceCreateInfo CreateInfo;
		Buffer = RHICreateStructuredBuffer(4, NumBytes, BUF_ShaderResource | BUF_ByteAddressBuffer | AdditionalUsage, CreateInfo);
		SRV = RHICreateShaderResourceView(Buffer);
	}

    void Initialize(uint32 InNumBytes, FResourceArrayInterface* InitResourceArrayPtr, uint32 AdditionalUsage = 0)
    {
		NumBytes = InNumBytes;
		check(GMaxRHIFeatureLevel == ERHIFeatureLevel::SM5);
		check(NumBytes % 4 == 0);
		FRHIResourceCreateInfo CreateInfo(InitResourceArrayPtr);
		Buffer = RHICreateStructuredBuffer(4, NumBytes, BUF_ShaderResource | BUF_ByteAddressBuffer | AdditionalUsage, CreateInfo);
		SRV = RHICreateShaderResourceView(Buffer);
    }

	void Release()
	{
		NumBytes = 0;
		Buffer.SafeRelease();
		SRV.SafeRelease();
	}

    void* LockReadOnly()
    {
        check(IsValid());
        return RHILockStructuredBuffer(Buffer, 0, Buffer->GetSize(), RLM_ReadOnly);
    }

    void* LockWriteOnly()
    {
        check(IsValid());
        return RHILockStructuredBuffer(Buffer, 0, Buffer->GetSize(), RLM_WriteOnly);
    }

    void Unlock()
    {
        check(IsValid());
        return RHIUnlockStructuredBuffer(Buffer);
    }
};

// Encapsulates a GPU read/write ByteAddress buffer with its UAV and SRV
struct FRULRWByteAddressBuffer : public FRULByteAddressBuffer
{
	FUnorderedAccessViewRHIRef UAV;

	void Initialize(uint32 InNumBytes, uint32 AdditionalUsage = 0)
	{
		FRULByteAddressBuffer::Initialize(InNumBytes, BUF_UnorderedAccess | AdditionalUsage);
		UAV = RHICreateUnorderedAccessView(Buffer, false, false);
	}

    void Initialize(uint32 InNumBytes, FResourceArrayInterface* InitResourceArrayPtr, uint32 AdditionalUsage = 0)
	{
		FRULByteAddressBuffer::Initialize(InNumBytes, InitResourceArrayPtr, BUF_UnorderedAccess | AdditionalUsage);
		UAV = RHICreateUnorderedAccessView(Buffer, false, false);
	}

	void Release()
	{
		FRULByteAddressBuffer::Release();
		UAV.SafeRelease();
	}
};
