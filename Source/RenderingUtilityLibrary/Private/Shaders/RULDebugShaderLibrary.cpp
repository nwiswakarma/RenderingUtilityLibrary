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

#include "Shaders/RULDebugShaderLibrary.h"

#include "RHIResources.h"

#include "GWTTickUtilities.h"

#include "RenderingUtilityLibrary.h"
#include "RHI/RULAlignedTypes.h"
#include "RHI/RULRHIBuffer.h"
#include "Shaders/RULPrefixSumScan.h"
#include "Shaders/RULReduceScan.h"

void URULDebugShaderLibrary::TestPrefixSumScan(UObject* WorldContextObject, int32 TestCount, UGWTTickEvent* CallbackEvent)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

    if (! World || TestCount < 1)
    {
        return;
    }

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_TestPrefixSumScan)(
		[TestCount](FRHICommandListImmediate& RHICmdList)
		{
            check(IsInRenderingThread());

            FRULRWBufferStructured SourceData;
            FRULRWBufferStructured OffsetData;
            FRULRWBufferStructured SumData;

            typedef TResourceArray<uint32, VERTEXBUFFER_ALIGNMENT> FResourceType;

            FResourceType SourceArr(false);
            SourceArr.Init(1, TestCount);

            SourceData.Initialize(
                sizeof(FResourceType::ElementType),
                TestCount,
                &SourceArr,
                BUF_Static,
                TEXT("SourceData")
                );

            const int32 ScanBlockCount = FRULPrefixSumScan::ExclusiveScan<1>(
                RHICmdList,
                SourceData.SRV,
                sizeof(FResourceType::ElementType),
                TestCount,
                OffsetData,
                SumData,
                BUF_Static
                );

            UE_LOG(LogTemp,Warning, TEXT("ScanBlockCount: %d"), ScanBlockCount);

            if (SumData.Buffer->GetStride() > 0 && ScanBlockCount > 0)
            {
                const int32 SumDataCount = SumData.Buffer->GetSize() / SumData.Buffer->GetStride();

                FResourceType SumArr;
                SumArr.SetNumUninitialized(SumDataCount);

                void* SumDataPtr = RHILockStructuredBuffer(SumData.Buffer, 0, SumData.Buffer->GetSize(), RLM_ReadOnly);
                FMemory::Memcpy(SumArr.GetData(), SumDataPtr, SumData.Buffer->GetSize());
                RHIUnlockStructuredBuffer(SumData.Buffer);

                FResourceType::ElementType ScanSum = SumArr[ScanBlockCount];

                UE_LOG(LogTemp,Warning, TEXT("SumDataCount: %d"), SumDataCount);
                UE_LOG(LogTemp,Warning, TEXT("ScanSum: %d"), ScanSum);
            }
		}
    ); // RULShaderLibrary_TestPrefixSumScan
}

void URULDebugShaderLibrary::TestReduceScan1D(
    UObject* WorldContextObject,
    int32 Seed,
    int32 TestCount,
    float MaxValue,
    int32 MaxValueIndex,
    bool bRandomizeIndex,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

    if (! World || TestCount < 1)
    {
        return;
    }

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_TestReduceScan)(
		[Seed, TestCount, MaxValue, MaxValueIndex, bRandomizeIndex](FRHICommandListImmediate& RHICmdList)
		{
            check(IsInRenderingThread());

            FRULRWBufferStructured SourceData;
            FRULRWBufferStructured SumData;

            typedef TResourceArray<float, VERTEXBUFFER_ALIGNMENT> FResourceType;

            FResourceType SourceArr(false);
            SourceArr.SetNumZeroed(TestCount);

            float SourceMax = 0.f;

            if (bRandomizeIndex)
            {
                FRandomStream Rand(Seed);

                for (int32 i=0; i<TestCount; ++i)
                {
                    SourceArr[i] = Rand.GetFraction() * MaxValue;

                    if (SourceMax < SourceArr[i])
                    {
                        SourceMax = SourceArr[i];
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax %d: %f"), i, SourceMax);
                    }
                }
            }
            else
            {
                SourceArr[FMath::Clamp(MaxValueIndex, 0, TestCount-1)] = MaxValue;
                SourceMax = MaxValue;
            }

            SourceData.Initialize(
                sizeof(FResourceType::ElementType),
                TestCount,
                &SourceArr,
                BUF_Static,
                TEXT("SourceData")
                );

            const int32 ScanBlockCount = FRULReduceScan::Reduce<FRULReduceScan::SDT_FLOAT1>(
                RHICmdList,
                SourceData.SRV,
                SumData,
                sizeof(FResourceType::ElementType),
                TestCount,
                BUF_Static
                );

            UE_LOG(LogTemp,Warning, TEXT("ScanBlockCount: %d"), ScanBlockCount);

            if (SumData.Buffer->GetStride() > 0 && ScanBlockCount > 0)
            {
                const int32 SumDataCount = SumData.Buffer->GetSize() / SumData.Buffer->GetStride();

                FResourceType SumArr;
                SumArr.SetNumUninitialized(SumDataCount);

                void* SumDataPtr = RHILockStructuredBuffer(SumData.Buffer, 0, SumData.Buffer->GetSize(), RLM_ReadOnly);
                FMemory::Memcpy(SumArr.GetData(), SumDataPtr, SumData.Buffer->GetSize());
                RHIUnlockStructuredBuffer(SumData.Buffer);

                FResourceType::ElementType ScanSum = SumArr[0];

                UE_LOG(LogTemp,Warning, TEXT("SumDataCount: %d"), SumDataCount);
                UE_LOG(LogTemp,Warning, TEXT("Scan Result: %f"), ScanSum);
            }

            UE_LOG(LogTemp,Warning, TEXT("Max Value: %f"), SourceMax);
		}
    ); // RULShaderLibrary_TestReduceScan
}

void URULDebugShaderLibrary::TestReduceScan2D(
    UObject* WorldContextObject,
    int32 Seed,
    int32 TestCount,
    float MaxValue,
    int32 MaxValueIndex,
    bool bRandomizeIndex,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

    if (! World || TestCount < 1)
    {
        return;
    }

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_TestReduceScan)(
		[Seed, TestCount, MaxValue, MaxValueIndex, bRandomizeIndex](FRHICommandListImmediate& RHICmdList)
		{
            check(IsInRenderingThread());

            FRULRWBufferStructured SourceData;
            FRULRWBufferStructured SumData;

            typedef TResourceArray<FRULAlignedVector2D, VERTEXBUFFER_ALIGNMENT> FResourceType;

            FResourceType SourceArr(false);
            SourceArr.SetNumZeroed(TestCount);

            FVector2D SourceMax(0.f, 0.f);

            if (bRandomizeIndex)
            {
                FRandomStream Rand(Seed);

                for (int32 i=0; i<TestCount; ++i)
                {
                    SourceArr[i].X = Rand.GetFraction() * MaxValue;
                    SourceArr[i].Y = Rand.GetFraction() * MaxValue;

                    if (SourceMax.X < SourceArr[i].X)
                    {
                        SourceMax.X = SourceArr[i].X;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.X %d: %f"), i, SourceMax.X);
                    }

                    if (SourceMax.Y < SourceArr[i].Y)
                    {                             
                        SourceMax.Y = SourceArr[i].Y;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.Y %d: %f"), i, SourceMax.Y);
                    }
                }
            }
            else
            {
                SourceMax = FVector2D(MaxValue, MaxValue);
                SourceArr[FMath::Clamp(MaxValueIndex, 0, TestCount-1)] = SourceMax;
            }

            SourceData.Initialize(
                sizeof(FResourceType::ElementType),
                TestCount,
                &SourceArr,
                BUF_Static,
                TEXT("SourceData")
                );

            const int32 ScanBlockCount = FRULReduceScan::Reduce<FRULReduceScan::SDT_FLOAT2>(
                RHICmdList,
                SourceData.SRV,
                SumData,
                sizeof(FResourceType::ElementType),
                TestCount,
                BUF_Static
                );

            UE_LOG(LogTemp,Warning, TEXT("ScanBlockCount: %d"), ScanBlockCount);

            if (SumData.Buffer->GetStride() > 0 && ScanBlockCount > 0)
            {
                const int32 SumDataCount = SumData.Buffer->GetSize() / SumData.Buffer->GetStride();

                FResourceType SumArr;
                SumArr.SetNumUninitialized(SumDataCount);

                void* SumDataPtr = RHILockStructuredBuffer(SumData.Buffer, 0, SumData.Buffer->GetSize(), RLM_ReadOnly);
                FMemory::Memcpy(SumArr.GetData(), SumDataPtr, SumData.Buffer->GetSize());
                RHIUnlockStructuredBuffer(SumData.Buffer);

                FResourceType::ElementType ScanSum = SumArr[0];

                UE_LOG(LogTemp,Warning, TEXT("SumDataCount: %d"), SumDataCount);
                UE_LOG(LogTemp,Warning, TEXT("Scan Result: %s"), *ScanSum.ToString());
            }

            UE_LOG(LogTemp,Warning, TEXT("Max Value: %s"), *SourceMax.ToString());
		}
    ); // RULShaderLibrary_TestReduceScan
}

void URULDebugShaderLibrary::TestReduceScan4D(
    UObject* WorldContextObject,
    int32 Seed,
    int32 TestCount,
    float MaxValue,
    int32 MaxValueIndex,
    bool bRandomizeIndex,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

    if (! World || TestCount < 1)
    {
        return;
    }

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_TestReduceScan)(
		[Seed, TestCount, MaxValue, MaxValueIndex, bRandomizeIndex](FRHICommandListImmediate& RHICmdList)
		{
            check(IsInRenderingThread());

            FRULRWBufferStructured SourceData;
            FRULRWBufferStructured SumData;

            typedef TResourceArray<FVector4, VERTEXBUFFER_ALIGNMENT> FResourceType;

            FResourceType SourceArr(false);
            SourceArr.SetNumZeroed(TestCount);

            FVector4 SourceMax(0.f, 0.f, 0.f, 0.f);

            if (bRandomizeIndex)
            {
                FRandomStream Rand(Seed);

                for (int32 i=0; i<TestCount; ++i)
                {
                    SourceArr[i].X = Rand.GetFraction() * MaxValue;
                    SourceArr[i].Y = Rand.GetFraction() * MaxValue;
                    SourceArr[i].Z = Rand.GetFraction() * MaxValue;
                    SourceArr[i].W = Rand.GetFraction() * MaxValue;

                    if (SourceMax.X < SourceArr[i].X)
                    {
                        SourceMax.X = SourceArr[i].X;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.X %d: %f"), i, SourceMax.X);
                    }

                    if (SourceMax.Y < SourceArr[i].Y)
                    {                             
                        SourceMax.Y = SourceArr[i].Y;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.Y %d: %f"), i, SourceMax.Y);
                    }

                    if (SourceMax.Z < SourceArr[i].Z)
                    {
                        SourceMax.Z = SourceArr[i].Z;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.Z %d: %f"), i, SourceMax.Z);
                    }

                    if (SourceMax.W < SourceArr[i].W)
                    {                             
                        SourceMax.W = SourceArr[i].W;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.W %d: %f"), i, SourceMax.W);
                    }
                }
            }
            else
            {
                SourceMax = FVector4(MaxValue, MaxValue, MaxValue, MaxValue);
                SourceArr[FMath::Clamp(MaxValueIndex, 0, TestCount-1)] = SourceMax;
            }

            SourceData.Initialize(
                sizeof(FResourceType::ElementType),
                TestCount,
                &SourceArr,
                BUF_Static,
                TEXT("SourceData")
                );

            const int32 ScanBlockCount = FRULReduceScan::Reduce<FRULReduceScan::SDT_FLOAT4>(
                RHICmdList,
                SourceData.SRV,
                SumData,
                sizeof(FResourceType::ElementType),
                TestCount,
                BUF_Static
                );

            UE_LOG(LogTemp,Warning, TEXT("ScanBlockCount: %d"), ScanBlockCount);

            if (SumData.Buffer->GetStride() > 0 && ScanBlockCount > 0)
            {
                const int32 SumDataCount = SumData.Buffer->GetSize() / SumData.Buffer->GetStride();

                FResourceType SumArr;
                SumArr.SetNumUninitialized(SumDataCount);

                void* SumDataPtr = RHILockStructuredBuffer(SumData.Buffer, 0, SumData.Buffer->GetSize(), RLM_ReadOnly);
                FMemory::Memcpy(SumArr.GetData(), SumDataPtr, SumData.Buffer->GetSize());
                RHIUnlockStructuredBuffer(SumData.Buffer);

                FResourceType::ElementType ScanSum = SumArr[0];

                UE_LOG(LogTemp,Warning, TEXT("SumDataCount: %d"), SumDataCount);
                UE_LOG(LogTemp,Warning, TEXT("Scan Result: %s"), *ScanSum.ToString());
            }

            UE_LOG(LogTemp,Warning, TEXT("Max Value: %s"), *SourceMax.ToString());
		}
    ); // RULShaderLibrary_TestReduceScan
}

void URULDebugShaderLibrary::TestReduceScan1DUint(
    UObject* WorldContextObject,
    int32 Seed,
    int32 TestCount,
    int32 MaxValueIndex,
    bool bRandomizeIndex,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

    if (! World || TestCount < 1)
    {
        return;
    }

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_TestReduceScan)(
		[Seed, TestCount, MaxValueIndex, bRandomizeIndex](FRHICommandListImmediate& RHICmdList)
		{
            check(IsInRenderingThread());

            FRULRWBufferStructured SourceData;
            FRULRWBufferStructured SumData;

            typedef TResourceArray<uint32, VERTEXBUFFER_ALIGNMENT> FResourceType;

            FResourceType SourceArr(false);
            SourceArr.SetNumZeroed(TestCount);

            uint32 SourceMax = 0;

            if (bRandomizeIndex)
            {
                FRandomStream Rand(Seed);

                for (int32 i=0; i<TestCount; ++i)
                {
                    SourceArr[i] = Rand.GetUnsignedInt();

                    if (SourceMax < SourceArr[i])
                    {
                        SourceMax = SourceArr[i];
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax %d: %u"), i, SourceMax);
                    }
                }
            }
            else
            {
                SourceArr[FMath::Clamp(MaxValueIndex, 0, TestCount-1)] = 0xFF;
                SourceMax = 0xFF;
            }

            SourceData.Initialize(
                sizeof(FResourceType::ElementType),
                TestCount,
                &SourceArr,
                BUF_Static,
                TEXT("SourceData")
                );

            const int32 ScanBlockCount = FRULReduceScan::Reduce<FRULReduceScan::SDT_UINT1>(
                RHICmdList,
                SourceData.SRV,
                SumData,
                sizeof(FResourceType::ElementType),
                TestCount,
                BUF_Static
                );

            UE_LOG(LogTemp,Warning, TEXT("ScanBlockCount: %d"), ScanBlockCount);

            if (SumData.Buffer->GetStride() > 0 && ScanBlockCount > 0)
            {
                const int32 SumDataCount = SumData.Buffer->GetSize() / SumData.Buffer->GetStride();

                FResourceType SumArr;
                SumArr.SetNumUninitialized(SumDataCount);

                void* SumDataPtr = RHILockStructuredBuffer(SumData.Buffer, 0, SumData.Buffer->GetSize(), RLM_ReadOnly);
                FMemory::Memcpy(SumArr.GetData(), SumDataPtr, SumData.Buffer->GetSize());
                RHIUnlockStructuredBuffer(SumData.Buffer);

                FResourceType::ElementType ScanSum = SumArr[0];

                UE_LOG(LogTemp,Warning, TEXT("SumDataCount: %d"), SumDataCount);
                UE_LOG(LogTemp,Warning, TEXT("Scan Result: %u"), ScanSum);
            }

            UE_LOG(LogTemp,Warning, TEXT("Max Value: %u"), SourceMax);
		}
    ); // RULShaderLibrary_TestReduceScan
}

void URULDebugShaderLibrary::TestReduceScan2DUint(
    UObject* WorldContextObject,
    int32 Seed,
    int32 TestCount,
    int32 MaxValueIndex,
    bool bRandomizeIndex,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

    if (! World || TestCount < 1)
    {
        return;
    }

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_TestReduceScan)(
		[Seed, TestCount, MaxValueIndex, bRandomizeIndex](FRHICommandListImmediate& RHICmdList)
		{
            check(IsInRenderingThread());

            FRULRWBufferStructured SourceData;
            FRULRWBufferStructured SumData;

            typedef TResourceArray<FRULAlignedUintPoint, VERTEXBUFFER_ALIGNMENT> FResourceType;

            FResourceType SourceArr(false);
            SourceArr.SetNumZeroed(TestCount);

            FRULAlignedUintPoint SourceMax;
            SourceMax.X = 0;
            SourceMax.Y = 0;

            if (bRandomizeIndex)
            {
                FRandomStream Rand(Seed);

                for (int32 i=0; i<TestCount; ++i)
                {
                    SourceArr[i].X = Rand.GetUnsignedInt();
                    SourceArr[i].Y = Rand.GetUnsignedInt();

                    if (SourceMax.X < SourceArr[i].X)
                    {
                        SourceMax.X = SourceArr[i].X;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.X %d: %u"), i, SourceMax.X);
                    }

                    if (SourceMax.Y < SourceArr[i].Y)
                    {                             
                        SourceMax.Y = SourceArr[i].Y;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.Y %d: %u"), i, SourceMax.Y);
                    }
                }
            }
            else
            {
                SourceMax.X = 255;
                SourceMax.Y = 255;
                SourceArr[FMath::Clamp(MaxValueIndex, 0, TestCount-1)] = SourceMax;
            }

            SourceData.Initialize(
                sizeof(FResourceType::ElementType),
                TestCount,
                &SourceArr,
                BUF_Static,
                TEXT("SourceData")
                );

            const int32 ScanBlockCount = FRULReduceScan::Reduce<FRULReduceScan::SDT_UINT2>(
                RHICmdList,
                SourceData.SRV,
                SumData,
                sizeof(FResourceType::ElementType),
                TestCount,
                BUF_Static
                );

            UE_LOG(LogTemp,Warning, TEXT("ScanBlockCount: %d"), ScanBlockCount);

            if (SumData.Buffer->GetStride() > 0 && ScanBlockCount > 0)
            {
                const int32 SumDataCount = SumData.Buffer->GetSize() / SumData.Buffer->GetStride();

                FResourceType SumArr;
                SumArr.SetNumUninitialized(SumDataCount);

                void* SumDataPtr = RHILockStructuredBuffer(SumData.Buffer, 0, SumData.Buffer->GetSize(), RLM_ReadOnly);
                FMemory::Memcpy(SumArr.GetData(), SumDataPtr, SumData.Buffer->GetSize());
                RHIUnlockStructuredBuffer(SumData.Buffer);

                FResourceType::ElementType ScanSum = SumArr[0];

                UE_LOG(LogTemp,Warning, TEXT("SumDataCount: %d"), SumDataCount);
                UE_LOG(LogTemp,Warning, TEXT("Scan Result: %u %u"), SourceMax.X, SourceMax.Y);
            }

            UE_LOG(LogTemp,Warning, TEXT("Max Value: %u, %u"), SourceMax.X, SourceMax.Y);
		}
    ); // RULShaderLibrary_TestReduceScan
}

void URULDebugShaderLibrary::TestReduceScan4DUint(
    UObject* WorldContextObject,
    int32 Seed,
    int32 TestCount,
    int32 MaxValueIndex,
    bool bRandomizeIndex,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

    if (! World || TestCount < 1)
    {
        return;
    }

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_TestReduceScan)(
		[Seed, TestCount, MaxValueIndex, bRandomizeIndex](FRHICommandListImmediate& RHICmdList)
		{
            check(IsInRenderingThread());

            FRULRWBufferStructured SourceData;
            FRULRWBufferStructured SumData;

            typedef TResourceArray<FRULAlignedUintVector4, VERTEXBUFFER_ALIGNMENT> FResourceType;

            FResourceType SourceArr(false);
            SourceArr.SetNumZeroed(TestCount);

            FRULAlignedUintVector4 SourceMax(0, 0, 0, 0);

            if (bRandomizeIndex)
            {
                FRandomStream Rand(Seed);

                for (int32 i=0; i<TestCount; ++i)
                {
                    SourceArr[i].X = Rand.GetUnsignedInt();
                    SourceArr[i].Y = Rand.GetUnsignedInt();
                    SourceArr[i].Z = Rand.GetUnsignedInt();
                    SourceArr[i].W = Rand.GetUnsignedInt();

                    if (SourceMax.X < SourceArr[i].X)
                    {
                        SourceMax.X = SourceArr[i].X;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.X %d: %u"), i, SourceMax.X);
                    }

                    if (SourceMax.Y < SourceArr[i].Y)
                    {                             
                        SourceMax.Y = SourceArr[i].Y;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.Y %d: %u"), i, SourceMax.Y);
                    }

                    if (SourceMax.Z < SourceArr[i].Z)
                    {
                        SourceMax.Z = SourceArr[i].Z;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.Z %d: %u"), i, SourceMax.Z);
                    }

                    if (SourceMax.W < SourceArr[i].W)
                    {                             
                        SourceMax.W = SourceArr[i].W;
                        UE_LOG(LogTemp,Warning, TEXT("SourceMax.W %d: %u"), i, SourceMax.W);
                    }
                }
            }
            else
            {
                SourceMax = FRULAlignedUintVector4(255, 255, 255, 255);
                SourceArr[FMath::Clamp(MaxValueIndex, 0, TestCount-1)] = SourceMax;
            }

            SourceData.Initialize(
                sizeof(FResourceType::ElementType),
                TestCount,
                &SourceArr,
                BUF_Static,
                TEXT("SourceData")
                );

            const int32 ScanBlockCount = FRULReduceScan::Reduce<FRULReduceScan::SDT_UINT4>(
                RHICmdList,
                SourceData.SRV,
                SumData,
                sizeof(FResourceType::ElementType),
                TestCount,
                BUF_Static
                );

            UE_LOG(LogTemp,Warning, TEXT("ScanBlockCount: %d"), ScanBlockCount);

            if (SumData.Buffer->GetStride() > 0 && ScanBlockCount > 0)
            {
                const int32 SumDataCount = SumData.Buffer->GetSize() / SumData.Buffer->GetStride();

                FResourceType SumArr;
                SumArr.SetNumUninitialized(SumDataCount);

                void* SumDataPtr = RHILockStructuredBuffer(SumData.Buffer, 0, SumData.Buffer->GetSize(), RLM_ReadOnly);
                FMemory::Memcpy(SumArr.GetData(), SumDataPtr, SumData.Buffer->GetSize());
                RHIUnlockStructuredBuffer(SumData.Buffer);

                FResourceType::ElementType ScanSum = SumArr[0];

                UE_LOG(LogTemp,Warning, TEXT("SumDataCount: %d"), SumDataCount);
                UE_LOG(LogTemp,Warning, TEXT("Scan Result: (X: %u, Y: %u, Z: %u, W: %u)"), ScanSum.X, ScanSum.Y, ScanSum.Z, ScanSum.W);
            }

            UE_LOG(LogTemp,Warning, TEXT("Max Value: (X: %u, Y: %u, Z: %u, W: %u)"), SourceMax.X, SourceMax.Y, SourceMax.Z, SourceMax.W);
		}
    ); // RULShaderLibrary_TestReduceScan
}
