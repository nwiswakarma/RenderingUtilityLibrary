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
#include "Shaders/Graph/RULShaderGraphTask.h"
#include "Shaders/Graph/RULShaderGraphTypes.h"
#include "RULShaderGraphTask_ApplyMaterial.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class URULShaderGraph;

UCLASS()
class RENDERINGUTILITYLIBRARY_API URULShaderGraphTask_ApplyMaterial : public URULShaderGraphTask
{
	GENERATED_BODY()

protected:

    UPROPERTY(Transient)
    TMap<FName, UTexture*> ResolvedTextureInputMap;

    virtual void ExecuteMaterialFunction(URULShaderGraph& Graph, UMaterialInstanceDynamic& MID);

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FRULShaderGraphMaterialRef MaterialRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ScalarInputMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FLinearColor> VectorInputMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FRULShaderGraphTextureInput> TextureInputMap;

    virtual void Initialize(URULShaderGraph* Graph) override;
    virtual void Execute(URULShaderGraph* Graph) override;

    UFUNCTION(BlueprintCallable)
    void SetScalarParameterValue(FName ParameterName, float ParameterValue);

    UFUNCTION(BlueprintCallable)
    void SetVectorParameterValue(FName ParameterName, FLinearColor ParameterValue);

    UFUNCTION(BlueprintCallable)
    void SetTextureParameterValue(FName ParameterName, FRULShaderGraphTextureInput ParameterValue);

    UFUNCTION(BlueprintCallable)
    void SetScalarParameter(const FRULShaderScalarParameter& Parameter);

    UFUNCTION(BlueprintCallable)
    void SetVectorParameter(const FRULShaderVectorParameter& Parameter);

    UFUNCTION(BlueprintCallable)
    void SetTextureParameter(const FRULShaderGraphTextureParameter& Parameter);

    void SetParameters(
        const TArray<FRULShaderScalarParameter>& ScalarParameters,
        const TArray<FRULShaderVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphTextureParameter>& TextureParameters
        );

    void SetParameters(
        const FRULShaderGraphParameterNameMap& ParameterNameMap,
        const TArray<FRULShaderScalarParameter>& ScalarParameters,
        const TArray<FRULShaderVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphTextureParameter>& TextureParameters
        );

    void SetParameters(
        URULShaderGraph& Graph,
        FName ParameterCategoryName,
        FName ParameterCategoryDefaultName,
        const TArray<FRULShaderScalarParameter>& ScalarParameters,
        const TArray<FRULShaderVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphTextureParameter>& TextureParameters
        );

    void ResolveTaskInputMap();
    void ApplyMaterialParameters(UMaterialInstanceDynamic& MID);
};
