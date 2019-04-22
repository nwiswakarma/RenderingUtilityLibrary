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
#include "Shaders/RULShaderParameters.h"
#include "Templates/RefCounting.h"
#include "RULShaderGraphTypes.generated.h"

class UMaterialInterface;
class UTexture;
class UTextureRenderTarget2D;
class URULShaderGraphTask;

class FRefCountedObject_Debug : public FRefCountedObject
{
	virtual ~FRefCountedObject_Debug()
    {
        if (!GetRefCount())
        {
            UE_LOG(LogTemp,Warning,TEXT("FRefCountedObject_Debug RELEASED"));
        }
    }
};

UENUM()
enum ERULShaderGraphConfigMethod
{
	RUL_CM_Parent,
	RUL_CM_Input,
	RUL_CM_Absolute
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderGraphTaskConfig
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRULShaderOutputConfig OutputConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRULShaderDrawConfig DrawConfig;

    FRULShaderGraphTaskConfig();
};

USTRUCT()
struct RENDERINGUTILITYLIBRARY_API FRULShaderGraphOutputRT
{
    GENERATED_USTRUCT_BODY()

    typedef FRefCountedObject_Debug FRefCountType;

    TRefCountPtr<FRefCountType> RefId;

    UPROPERTY(Transient, EditInstanceOnly)
    UTextureRenderTarget2D* RenderTarget;

    FRULShaderGraphOutputRT();
    explicit FRULShaderGraphOutputRT(UTextureRenderTarget2D* InRenderTarget);

    FORCEINLINE bool IsFree() const
    {
        return RefId.IsValid() && RefId->GetRefCount() == 1;
    }

    bool IsValidOutput() const;
    bool CompareFormat(const FRULShaderOutputConfig& OutputConfig) const;
    void ClearReferenceId();
    void CreateReferenceId();
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderGraphTextureInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture* Texture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    URULShaderGraphTask* Task;
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderGraphMaterialRef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MaterialName;
};

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderGraphTextureParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRULShaderGraphTextureInput ParameterValue;

    FRULShaderGraphTextureParameter() = default;
    FRULShaderGraphTextureParameter(FName InParameterName, const FRULShaderGraphTextureInput& InParameterValue)
        : ParameterName(InParameterName)
        , ParameterValue(InParameterValue)
    {
    }
};


USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULShaderGraphParameterNameMap
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FName> NameMap;

    FName Get(FName NameKey) const
    {
        const FName* Name = NameMap.Find(NameKey);
        return Name ? *Name : FName();
    }

    FName GetOrDefault(FName NameKey) const
    {
        const FName* Name = NameMap.Find(NameKey);
        return Name ? *Name : NameKey;
    }
};
