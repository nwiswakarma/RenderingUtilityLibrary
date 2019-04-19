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
#include "Shaders/Graph/RULShaderGraphTypes.h"
#include "RULShaderGraphBaseMaterialLibrary.generated.h"

class UMaterialInterface;
class URULShaderGraphTask;
class URULShaderGraphTask_ApplyMaterial;

UCLASS()
class RENDERINGUTILITYLIBRARY_API URULShaderGraphBaseMaterialLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

    typedef FRULShaderGraphParameterNameMap FParamNameMap;
    typedef FRULShaderGraphMaterialScalarParameter FScalarParam;
    typedef FRULShaderGraphMaterialVectorParameter FVectorParam;
    typedef FRULShaderGraphMaterialTextureParameter FTextureParam;

public:

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Levels", AutoCreateRefTerm="TaskConfig,MaterialRef,ScalarParameters,VectorParameters,TextureParameters", AdvancedDisplay="Graph,TaskType,TaskConfig,ConfigMethod,OutputTask,ScalarParameters,VectorParameters,TextureParameters,ParameterCategoryName"))
    static URULShaderGraphTask_ApplyMaterial* AddLevelsTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderGraphMaterialScalarParameter>& ScalarParameters,
        const TArray<FRULShaderGraphMaterialVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphMaterialTextureParameter>& TextureParameters,
        FName ParameterCategoryName,
        FRULShaderGraphTaskTextureInput SourceTexture,
        float InRemapValueLo = 0.f,
        float InRemapValueHi = 1.f,
        float OutRemapValueLo = 0.f,
        float OutRemapValueHi = 1.f,
        float MidPoint = .5f
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Levels Manual", AutoCreateRefTerm="TaskConfig,MaterialRef,ScalarParameters,VectorParameters,TextureParameters", AdvancedDisplay="Graph,TaskType,TaskConfig,ConfigMethod,OutputTask,ScalarParameters,VectorParameters,TextureParameters,ParameterCategoryName"))
    static URULShaderGraphTask_ApplyMaterial* AddLevelsManualTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderGraphMaterialScalarParameter>& ScalarParameters,
        const TArray<FRULShaderGraphMaterialVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphMaterialTextureParameter>& TextureParameters,
        FName ParameterCategoryName,
        FRULShaderGraphTaskTextureInput SourceTexture,
        float InRemapValueLo = 0.f,
        float InRemapValueHi = 1.f,
        float OutRemapValueLo = 0.f,
        float OutRemapValueHi = 1.f,
        float LevelsLo = 0.0f,
        float LevelsMi = 0.5f,
        float LevelsHi = 1.0f,
        float MidPoint = .5f
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Blend Target", AutoCreateRefTerm="TaskConfig,MaterialRef,ScalarParameters,VectorParameters,TextureParameters", AdvancedDisplay="Graph,TaskType,ConfigMethod,ScalarParameters,VectorParameters,TextureParameters,ParameterCategoryName"))
    static URULShaderGraphTask_ApplyMaterial* AddBlendTargetTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderGraphMaterialScalarParameter>& ScalarParameters,
        const TArray<FRULShaderGraphMaterialVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphMaterialTextureParameter>& TextureParameters,
        FName ParameterCategoryName,
        FRULShaderGraphTaskTextureInput SourceTexture,
        float Opacity
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Blend", AutoCreateRefTerm="TaskConfig,MaterialRef,ScalarParameters,VectorParameters,TextureParameters", AdvancedDisplay="Graph,TaskType,TaskConfig,ConfigMethod,OutputTask,ScalarParameters,VectorParameters,TextureParameters,ParameterCategoryName"))
    static URULShaderGraphTask_ApplyMaterial* AddBlendTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderGraphMaterialScalarParameter>& ScalarParameters,
        const TArray<FRULShaderGraphMaterialVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphMaterialTextureParameter>& TextureParameters,
        FName ParameterCategoryName,
        FRULShaderGraphTaskTextureInput BackgroundTexture,
        FRULShaderGraphTaskTextureInput ForegroundTexture,
        float Opacity
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Blend Masked", AutoCreateRefTerm="TaskConfig,MaterialRef,ScalarParameters,VectorParameters,TextureParameters", AdvancedDisplay="Graph,TaskType,TaskConfig,ConfigMethod,OutputTask,ScalarParameters,VectorParameters,TextureParameters,ParameterCategoryName"))
    static URULShaderGraphTask_ApplyMaterial* AddBlendMaskedTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderGraphMaterialScalarParameter>& ScalarParameters,
        const TArray<FRULShaderGraphMaterialVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphMaterialTextureParameter>& TextureParameters,
        FName ParameterCategoryName,
        FRULShaderGraphTaskTextureInput BackgroundTexture,
        FRULShaderGraphTaskTextureInput ForegroundTexture,
        FRULShaderGraphTaskTextureInput MaskTexture,
        float Opacity
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Blur Filter 1D", AutoCreateRefTerm="TaskConfig,MaterialRef,ScalarParameters,VectorParameters,TextureParameters", AdvancedDisplay="Graph,TaskType,TaskConfig,ConfigMethod,OutputTask,ScalarParameters,VectorParameters,TextureParameters,ParameterCategoryName"))
    static URULShaderGraphTask_BlurFilter1D* AddBlurFilter1DTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_BlurFilter1D> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderGraphMaterialScalarParameter>& ScalarParameters,
        const TArray<FRULShaderGraphMaterialVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphMaterialTextureParameter>& TextureParameters,
        FName ParameterCategoryName,
        FRULShaderGraphTaskTextureInput SourceTexture,
        float BlurSampleCount = 1.f
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Distance Filter", AutoCreateRefTerm="TaskConfig,MaterialRef,ScalarParameters,VectorParameters,TextureParameters", AdvancedDisplay="Graph,TaskType,TaskConfig,ConfigMethod,OutputTask,ScalarParameters,VectorParameters,TextureParameters,ParameterCategoryName"))
    static URULShaderGraphTask_ApplyMaterial* AddDistanceFilterTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderGraphMaterialScalarParameter>& ScalarParameters,
        const TArray<FRULShaderGraphMaterialVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphMaterialTextureParameter>& TextureParameters,
        FName ParameterCategoryName,
        FRULShaderGraphTaskTextureInput SourceTexture,
        float DistanceSteps = 1.f
        );
};
