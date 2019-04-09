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
#include "RULShaderGeometry.generated.h"

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderQuadGeometry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Origin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Size = FVector2D::UnitVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AngleRadian = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Luminosity = 1.f;
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderPolyGeometry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Origin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Size = FVector2D::UnitVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Sides = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AngleRadian = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Luminosity = 1.f;
};
