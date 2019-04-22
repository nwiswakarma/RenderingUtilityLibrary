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
#include "RULShaderParameters.generated.h"

class UTexture;
class UTexture2D;
class UTextureRenderTarget2D;
class FTexture2DResource;
class FTextureRenderTarget2DResource;

UENUM(BlueprintType)
enum class ERULShaderDrawBlendType : uint8
{
	DB_Opaque = 0,
	DB_Max    = 1,
	DB_Min    = 2,
	DB_Add    = 3,
	DB_Sub    = 4,
	DB_SubRev = 5
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderOutputConfig
{
	GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(UIMin=0, ClampMin=0))
    int32 SizeX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(UIMin=0, ClampMin=0))
    int32 SizeY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<enum ETextureRenderTargetFormat> Format;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bForceLinearGamma;

    FRULShaderOutputConfig();

    FORCEINLINE FIntPoint GetDimension() const
    {
        return { SizeX, SizeY };
    }

    FORCEINLINE bool Compare(const FRULShaderOutputConfig& Other) const
    {
        return (
            SizeX == Other.SizeX &&
            SizeX == Other.SizeY &&
            Format == Other.Format &&
            bForceLinearGamma == Other.bForceLinearGamma
            );
    }
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderDrawConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERULShaderDrawBlendType BlendType = ERULShaderDrawBlendType::DB_Opaque;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bClearRenderTarget = false;
};

UENUM(BlueprintType)
enum class ERULShaderTextureType : uint8
{
    RUL_STT_Texture2D,
    RUL_STT_TextureRenderTarget2D,
    RUL_STT_Unknown
};

struct RENDERINGUTILITYLIBRARY_API FRULShaderTextureParameterInputResource
{
    ERULShaderTextureType           TextureType = ERULShaderTextureType::RUL_STT_Unknown;
    FTexture2DResource*             Texture2DResource = nullptr;
    FTextureRenderTarget2DResource* TextureRenderTarget2DResource = nullptr;

    bool HasValidResource() const;
    FTexture2DRHIParamRef GetTextureParamRef_RT() const;
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderTextureParameterInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERULShaderTextureType TextureType = ERULShaderTextureType::RUL_STT_Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* Texture2D = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTextureRenderTarget2D* TextureRenderTarget2D = nullptr;

    FRULShaderTextureParameterInput() = default;

    explicit FRULShaderTextureParameterInput(UTexture2D* InTexture2D)
        : TextureType(ERULShaderTextureType::RUL_STT_Texture2D)
        , Texture2D(InTexture2D)
        , TextureRenderTarget2D(nullptr)
    {
    }

    explicit FRULShaderTextureParameterInput(UTextureRenderTarget2D* InTextureRenderTarget2D)
        : TextureType(ERULShaderTextureType::RUL_STT_TextureRenderTarget2D)
        , Texture2D(nullptr)
        , TextureRenderTarget2D(InTextureRenderTarget2D)
    {
    }

    FRULShaderTextureParameterInputResource GetResource_GT() const;
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderScalarParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ParameterValue;

    FRULShaderScalarParameter() = default;
    FRULShaderScalarParameter(FName InParameterName, float InParameterValue)
        : ParameterName(InParameterName)
        , ParameterValue(InParameterValue)
    {
    }
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderVectorParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ParameterValue;

    FRULShaderVectorParameter() = default;
    FRULShaderVectorParameter(FName InParameterName, const FLinearColor& InParameterValue)
        : ParameterName(InParameterName)
        , ParameterValue(InParameterValue)
    {
    }
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderTextureParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture* ParameterValue;

    FRULShaderTextureParameter() = default;
    FRULShaderTextureParameter(FName InParameterName, UTexture* InParameterValue)
        : ParameterName(InParameterName)
        , ParameterValue(InParameterValue)
    {
    }
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderNameResolveParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ParameterValue;

    FRULShaderNameResolveParameter() = default;
    FRULShaderNameResolveParameter(FName InParameterName, FName InParameterValue)
        : ParameterName(InParameterName)
        , ParameterValue(InParameterValue)
    {
    }
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderMaterialParameterCollection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FRULShaderScalarParameter> ScalarParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FRULShaderVectorParameter> VectorParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FRULShaderTextureParameter> TextureParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FRULShaderNameResolveParameter> NamedTextures;
};
