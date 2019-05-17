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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RULDebugShaderLibrary.generated.h"

class UGWTTickEvent;

UCLASS()
class RENDERINGUTILITYLIBRARY_API URULDebugShaderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
    static void TestPrefixSumScan(
        UObject* WorldContextObject,
        int32 TestCount,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    UFUNCTION(BlueprintCallable)
    static void TestReduceScan1D(
        UObject* WorldContextObject,
        int32 Seed,
        int32 TestCount,
        float MaxValue,
        int32 MaxValueIndex,
        bool bRandomizeIndex,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    UFUNCTION(BlueprintCallable)
    static void TestReduceScan2D(
        UObject* WorldContextObject,
        int32 Seed,
        int32 TestCount,
        float MaxValue,
        int32 MaxValueIndex,
        bool bRandomizeIndex,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    UFUNCTION(BlueprintCallable)
    static void TestReduceScan4D(
        UObject* WorldContextObject,
        int32 Seed,
        int32 TestCount,
        float MaxValue,
        int32 MaxValueIndex,
        bool bRandomizeIndex,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    UFUNCTION(BlueprintCallable)
    static void TestReduceScan1DUint(
        UObject* WorldContextObject,
        int32 Seed,
        int32 TestCount,
        int32 MaxValueIndex,
        bool bRandomizeIndex,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    UFUNCTION(BlueprintCallable)
    static void TestReduceScan2DUint(
        UObject* WorldContextObject,
        int32 Seed,
        int32 TestCount,
        int32 MaxValueIndex,
        bool bRandomizeIndex,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    UFUNCTION(BlueprintCallable)
    static void TestReduceScan4DUint(
        UObject* WorldContextObject,
        int32 Seed,
        int32 TestCount,
        int32 MaxValueIndex,
        bool bRandomizeIndex,
        UGWTTickEvent* CallbackEvent = nullptr
        );
};
