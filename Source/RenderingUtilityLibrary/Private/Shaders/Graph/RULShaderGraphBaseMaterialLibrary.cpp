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

#include "Shaders/Graph/RULShaderGraphBaseMaterialLibrary.h"

#include "Shaders/Graph/RULShaderGraph.h"
#include "Shaders/Graph/RULShaderGraphUtility.h"
#include "Shaders/Graph/Tasks/Materials/RULShaderGraphTask_BlurFilter1D.h"

URULShaderGraphTask_ApplyMaterial* URULShaderGraphBaseMaterialLibrary::AddLevelsTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FRULShaderScalarParameter>& ScalarParameters,
    const TArray<FRULShaderVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphTextureParameter>& TextureParameters,
    FName ParameterCategoryName,
    FRULShaderGraphTextureInput SourceTexture,
    float InRemapValueLo,
    float InRemapValueHi,
    float OutRemapValueLo,
    float OutRemapValueHi,
    float MidPoint
    )
{
    URULShaderGraphTask_ApplyMaterial* Task;
    Task = URULShaderGraphUtility::AddApplyMaterialTaskWithParameters(
        Graph,
        TaskType,
        TaskConfig,
        ConfigMethod,
        OutputTask,
        MaterialRef,
        ScalarParameters,
        VectorParameters,
        TextureParameters
        );

    if (IsValid(Task))
    {
        TArray<FScalarParam> MappedScalars;
        TArray<FTextureParam> MappedTextures;

        MappedScalars.Emplace(TEXT("InRemapValueLo"), InRemapValueLo);
        MappedScalars.Emplace(TEXT("InRemapValueHi"), InRemapValueHi);
        MappedScalars.Emplace(TEXT("OutRemapValueLo"), OutRemapValueLo);
        MappedScalars.Emplace(TEXT("OutRemapValueHi"), OutRemapValueHi);
        MappedScalars.Emplace(TEXT("MidPoint"), MidPoint);
        MappedTextures.Emplace(TEXT("SourceTexture"), SourceTexture);

        Task->SetParameters(
            *Graph,
            ParameterCategoryName,
            TEXT("Levels"),
            MappedScalars,
            { },
            MappedTextures
            );
    }

    return Task;
}

URULShaderGraphTask_ApplyMaterial* URULShaderGraphBaseMaterialLibrary::AddLevelsManualTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FRULShaderScalarParameter>& ScalarParameters,
    const TArray<FRULShaderVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphTextureParameter>& TextureParameters,
    FName ParameterCategoryName,
    FRULShaderGraphTextureInput SourceTexture,
    float InRemapValueLo,
    float InRemapValueHi,
    float OutRemapValueLo,
    float OutRemapValueHi,
    float LevelsLo,
    float LevelsMi,
    float LevelsHi,
    float MidPoint
    )
{
    URULShaderGraphTask_ApplyMaterial* Task;
    Task = URULShaderGraphUtility::AddApplyMaterialTaskWithParameters(
        Graph,
        TaskType,
        TaskConfig,
        ConfigMethod,
        OutputTask,
        MaterialRef,
        ScalarParameters,
        VectorParameters,
        TextureParameters
        );

    if (IsValid(Task))
    {
        TArray<FScalarParam> MappedScalars;
        TArray<FTextureParam> MappedTextures;

        MappedScalars.Emplace(TEXT("InRemapValueLo"), InRemapValueLo);
        MappedScalars.Emplace(TEXT("InRemapValueHi"), InRemapValueHi);
        MappedScalars.Emplace(TEXT("OutRemapValueLo"), OutRemapValueLo);
        MappedScalars.Emplace(TEXT("OutRemapValueHi"), OutRemapValueHi);
        MappedScalars.Emplace(TEXT("LevelsLo"), LevelsLo);
        MappedScalars.Emplace(TEXT("LevelsMi"), LevelsMi);
        MappedScalars.Emplace(TEXT("LevelsHi"), LevelsHi);
        MappedScalars.Emplace(TEXT("MidPoint"), MidPoint);
        MappedTextures.Emplace(TEXT("SourceTexture"), SourceTexture);

        Task->SetParameters(
            *Graph,
            ParameterCategoryName,
            TEXT("LevelsManual"),
            MappedScalars,
            { },
            MappedTextures
            );
    }

    return Task;
}

URULShaderGraphTask_ApplyMaterial* URULShaderGraphBaseMaterialLibrary::AddBlendTargetTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FRULShaderScalarParameter>& ScalarParameters,
    const TArray<FRULShaderVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphTextureParameter>& TextureParameters,
    FName ParameterCategoryName,
    FRULShaderGraphTextureInput SourceTexture,
    float Opacity
    )
{
    URULShaderGraphTask_ApplyMaterial* Task;
    Task = URULShaderGraphUtility::AddApplyMaterialTaskWithParameters(
        Graph,
        TaskType,
        TaskConfig,
        ConfigMethod,
        OutputTask,
        MaterialRef,
        ScalarParameters,
        VectorParameters,
        TextureParameters
        );

    if (IsValid(Task))
    {
        TArray<FScalarParam> MappedScalars;
        TArray<FTextureParam> MappedTextures;

        MappedScalars.Emplace(TEXT("Opacity"), Opacity);
        MappedTextures.Emplace(TEXT("SourceTexture"), SourceTexture);

        Task->SetParameters(
            *Graph,
            ParameterCategoryName,
            TEXT("BlendTarget"),
            MappedScalars,
            { },
            MappedTextures
            );
    }

    return Task;
}

URULShaderGraphTask_ApplyMaterial* URULShaderGraphBaseMaterialLibrary::AddBlendTargetMaskedTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FRULShaderScalarParameter>& ScalarParameters,
    const TArray<FRULShaderVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphTextureParameter>& TextureParameters,
    FName ParameterCategoryName,
    FRULShaderGraphTextureInput SourceTexture,
    FRULShaderGraphTextureInput MaskTexture,
    float Opacity
    )
{
    URULShaderGraphTask_ApplyMaterial* Task;
    Task = URULShaderGraphUtility::AddApplyMaterialTaskWithParameters(
        Graph,
        TaskType,
        TaskConfig,
        ConfigMethod,
        OutputTask,
        MaterialRef,
        ScalarParameters,
        VectorParameters,
        TextureParameters
        );

    if (IsValid(Task))
    {
        TArray<FScalarParam> MappedScalars;
        TArray<FTextureParam> MappedTextures;

        MappedScalars.Emplace(TEXT("Opacity"), Opacity);
        MappedTextures.Emplace(TEXT("SourceTexture"), SourceTexture);
        MappedTextures.Emplace(TEXT("MaskTexture"), MaskTexture);

        Task->SetParameters(
            *Graph,
            ParameterCategoryName,
            TEXT("BlendTargetMasked"),
            MappedScalars,
            { },
            MappedTextures
            );
    }

    return Task;
}

URULShaderGraphTask_ApplyMaterial* URULShaderGraphBaseMaterialLibrary::AddBlendTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FRULShaderScalarParameter>& ScalarParameters,
    const TArray<FRULShaderVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphTextureParameter>& TextureParameters,
    FName ParameterCategoryName,
    FRULShaderGraphTextureInput BackgroundTexture,
    FRULShaderGraphTextureInput ForegroundTexture,
    float Opacity
    )
{
    URULShaderGraphTask_ApplyMaterial* Task;
    Task = URULShaderGraphUtility::AddApplyMaterialTaskWithParameters(
        Graph,
        TaskType,
        TaskConfig,
        ConfigMethod,
        OutputTask,
        MaterialRef,
        ScalarParameters,
        VectorParameters,
        TextureParameters
        );

    if (IsValid(Task))
    {
        TArray<FScalarParam> MappedScalars;
        TArray<FTextureParam> MappedTextures;

        MappedScalars.Emplace(TEXT("Opacity"), Opacity);
        MappedTextures.Emplace(TEXT("BackgroundTexture"), BackgroundTexture);
        MappedTextures.Emplace(TEXT("ForegroundTexture"), ForegroundTexture);

        Task->SetParameters(
            *Graph,
            ParameterCategoryName,
            TEXT("Blend"),
            MappedScalars,
            { },
            MappedTextures
            );
    }

    return Task;
}

URULShaderGraphTask_ApplyMaterial* URULShaderGraphBaseMaterialLibrary::AddBlendMaskedTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FRULShaderScalarParameter>& ScalarParameters,
    const TArray<FRULShaderVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphTextureParameter>& TextureParameters,
    FName ParameterCategoryName,
    FRULShaderGraphTextureInput BackgroundTexture,
    FRULShaderGraphTextureInput ForegroundTexture,
    FRULShaderGraphTextureInput MaskTexture,
    float Opacity
    )
{
    URULShaderGraphTask_ApplyMaterial* Task;
    Task = URULShaderGraphUtility::AddApplyMaterialTaskWithParameters(
        Graph,
        TaskType,
        TaskConfig,
        ConfigMethod,
        OutputTask,
        MaterialRef,
        ScalarParameters,
        VectorParameters,
        TextureParameters
        );

    if (IsValid(Task))
    {
        TArray<FScalarParam> MappedScalars;
        TArray<FTextureParam> MappedTextures;

        MappedScalars.Emplace(TEXT("Opacity"), Opacity);
        MappedTextures.Emplace(TEXT("BackgroundTexture"), BackgroundTexture);
        MappedTextures.Emplace(TEXT("ForegroundTexture"), ForegroundTexture);
        MappedTextures.Emplace(TEXT("MaskTexture"), MaskTexture);

        Task->SetParameters(
            *Graph,
            ParameterCategoryName,
            TEXT("BlendMasked"),
            MappedScalars,
            { },
            MappedTextures
            );
    }

    return Task;
}

URULShaderGraphTask_BlurFilter1D* URULShaderGraphBaseMaterialLibrary::AddBlurFilter1DTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_BlurFilter1D> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FRULShaderScalarParameter>& ScalarParameters,
    const TArray<FRULShaderVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphTextureParameter>& TextureParameters,
    FName ParameterCategoryName,
    FRULShaderGraphTextureInput SourceTexture,
    float BlurSampleCount
    )
{
    URULShaderGraphTask_BlurFilter1D* Task = nullptr;

    if (IsValid(Graph))
    {
        if (TaskType.Get())
        {
            Task = NewObject<URULShaderGraphTask_BlurFilter1D>(Graph, TaskType);
        }
        else
        {
            Task = NewObject<URULShaderGraphTask_BlurFilter1D>(Graph);
        }

        if (IsValid(Task))
        {
            TArray<FScalarParam> MappedScalars;
            TArray<FTextureParam> MappedTextures;

            MappedScalars.Emplace(TEXT("BlurSampleCount"), BlurSampleCount);
            MappedTextures.Emplace(TEXT("SourceTexture"), SourceTexture);

            Task->MaterialRef = MaterialRef;

            Task->DirectionXParameterName = Graph->GetParameterNameFromCategory(
                ParameterCategoryName,
                TEXT("BlurFilter1D"),
                TEXT("BlurDirectionX")
                );

            Task->DirectionYParameterName = Graph->GetParameterNameFromCategory(
                ParameterCategoryName,
                TEXT("BlurFilter1D"),
                TEXT("BlurDirectionY")
                );

            Task->SourceTextureParameterName = Graph->GetParameterNameFromCategory(
                ParameterCategoryName,
                TEXT("BlurFilter1D"),
                TEXT("SourceTexture")
                );

            Task->SetParameters(
                ScalarParameters,
                VectorParameters,
                TextureParameters
                );

            Task->SetParameters(
                *Graph,
                ParameterCategoryName,
                TEXT("BlurFilter1D"),
                MappedScalars,
                { },
                MappedTextures
                );

            URULShaderGraphUtility::AddTask(
                *Graph,
                *Task,
                TaskConfig,
                ConfigMethod,
                OutputTask
                );
        }
    }

    return Task;
}

URULShaderGraphTask_ApplyMaterial* URULShaderGraphBaseMaterialLibrary::AddDistanceFilterTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FRULShaderScalarParameter>& ScalarParameters,
    const TArray<FRULShaderVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphTextureParameter>& TextureParameters,
    FName ParameterCategoryName,
    FRULShaderGraphTextureInput SourceTexture,
    float DistanceSteps
    )
{
    URULShaderGraphTask_ApplyMaterial* Task;
    Task = URULShaderGraphUtility::AddApplyMaterialTaskWithParameters(
        Graph,
        TaskType,
        TaskConfig,
        ConfigMethod,
        OutputTask,
        MaterialRef,
        ScalarParameters,
        VectorParameters,
        TextureParameters
        );

    if (IsValid(Task))
    {
        TArray<FScalarParam> MappedScalars;
        TArray<FTextureParam> MappedTextures;

        MappedScalars.Emplace(TEXT("DistanceSteps"), DistanceSteps);
        MappedTextures.Emplace(TEXT("SourceTexture"), SourceTexture);

        check(Graph != nullptr);

        Task->SetParameters(
            *Graph,
            ParameterCategoryName,
            TEXT("DistanceFilter"),
            MappedScalars,
            { },
            MappedTextures
            );
    }

    return Task;
}
