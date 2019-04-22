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
#include "Shaders/RULShaderGeometry.h"
#include "Shaders/RULShaderParameters.h"
#include "Shaders/Graph/RULShaderGraphTypes.h"
#include "RULShaderGraphUtility.generated.h"

class UMaterialInterface;
class UTextureRenderTarget2D;
class URULShaderGraphTask;
class URULShaderGraphTask_ResolveOutput;
class URULShaderGraphTask_DrawGeometry;
class URULShaderGraphTask_DrawMaterialPoly;
class URULShaderGraphTask_DrawMaterialQuad;
class URULShaderGraphTask_ApplyMaterial;

UCLASS()
class RENDERINGUTILITYLIBRARY_API URULShaderGraphUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

    typedef FRULShaderGraphParameterNameMap FParamNameMap;
    typedef FRULShaderScalarParameter FScalarParam;
    typedef FRULShaderVectorParameter FVectorParam;
    typedef FRULShaderGraphTextureParameter FTextureParam;

public:

    static void AddTask(
        URULShaderGraph& Graph,
        URULShaderGraphTask& Task,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask
        );

    static void AddTask(URULShaderGraph& Graph, URULShaderGraphTask& Task);

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", AutoCreateRefTerm="TaskConfig", AdvancedDisplay="Graph,TaskConfig,ConfigMethod,OutputTask"))
    static URULShaderGraphTask* AddTaskByInstance(
        URULShaderGraph* Graph,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        URULShaderGraphTask* TaskInstance
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Resolve Output To Render Target", AutoCreateRefTerm="TaskConfig", AdvancedDisplay="Graph"))
    static URULShaderGraphTask_ResolveOutput* AddResolveTask(
        URULShaderGraph* Graph,
        URULShaderGraphTask* SourceTask,
        UTextureRenderTarget2D* RenderTargetTexture
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Draw Points", AutoCreateRefTerm="TaskConfig,Colors", AdvancedDisplay="Graph,TaskType,TaskConfig,ConfigMethod,OutputTask"))
    static URULShaderGraphTask_DrawGeometry* AddDrawPointsTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_DrawGeometry> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const TArray<FVector2D>& Points,
        const TArray<FColor>& Colors,
        const TArray<int32>& Indices
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Draw Geometry", AutoCreateRefTerm="TaskConfig,Colors", AdvancedDisplay="Graph,TaskType,TaskConfig,ConfigMethod,OutputTask"))
    static URULShaderGraphTask_DrawGeometry* AddDrawGeometryTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_DrawGeometry> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const TArray<FVector>& Vertices,
        const TArray<FColor>& Colors,
        const TArray<int32>& Indices
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", AutoCreateRefTerm="TaskConfig,MaterialRef", AdvancedDisplay="Graph,TaskType,TaskConfig,ConfigMethod,OutputTask"))
    static URULShaderGraphTask_ApplyMaterial* AddApplyMaterialTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", AutoCreateRefTerm="TaskConfig,MaterialRef,ScalarParameters,VectorParameters,TextureParameters", AdvancedDisplay="Graph,TaskType,TaskConfig,ConfigMethod,OutputTask"))
    static URULShaderGraphTask_ApplyMaterial* AddApplyMaterialTaskWithParameters(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderScalarParameter>& ScalarParameters,
        const TArray<FRULShaderVectorParameter>& VectorParameters,
        const TArray<FRULShaderGraphTextureParameter>& TextureParameters
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Draw Material Quad", AutoCreateRefTerm="TaskConfig,MaterialRef", AdvancedDisplay="Graph,TaskType,ConfigMethod,OutputTask"))
    static URULShaderGraphTask_DrawMaterialQuad* AddDrawMaterialQuadTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_DrawMaterialQuad> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderQuadGeometry>& Quads
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Draw Material Poly", AutoCreateRefTerm="TaskConfig,MaterialRef", AdvancedDisplay="Graph,TaskType,ConfigMethod,OutputTask"))
    static URULShaderGraphTask_DrawMaterialPoly* AddDrawMaterialPolyTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_DrawMaterialPoly> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderPolyGeometry>& Polys
        );

    UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Graph", DisplayName="Draw Texture Quad", AutoCreateRefTerm="TaskConfig,MaterialRef", AdvancedDisplay="Graph,TaskType,ConfigMethod,OutputTask,ParameterCategoryName"))
    static URULShaderGraphTask_DrawMaterialQuad* AddDrawTextureQuadTask(
        URULShaderGraph* Graph,
        TSubclassOf<URULShaderGraphTask_DrawMaterialQuad> TaskType,
        const FRULShaderGraphTaskConfig& TaskConfig,
        TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
        URULShaderGraphTask* OutputTask,
        const FRULShaderGraphMaterialRef& MaterialRef,
        const TArray<FRULShaderQuadGeometry>& Quads,
        FName ParameterCategoryName,
        FRULShaderGraphTextureInput SourceTexture,
        float Opacity = 1.f
        );
};
