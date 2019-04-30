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

#include "Shaders/Graph/RULShaderGraphUtility.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Shaders/Graph/RULShaderGraphTask.h"
#include "Shaders/Graph/Tasks/RULShaderGraphTask_ApplyMaterial.h"
#include "Shaders/Graph/Tasks/RULShaderGraphTask_DrawGeometry.h"
#include "Shaders/Graph/Tasks/RULShaderGraphTask_DrawMaterialPoly.h"
#include "Shaders/Graph/Tasks/RULShaderGraphTask_DrawMaterialQuad.h"
#include "Shaders/Graph/Tasks/RULShaderGraphTask_DrawTaskToOutput.h"
#include "Shaders/Graph/Tasks/RULShaderGraphTask_DrawTaskToTexture.h"
#include "Shaders/Graph/Tasks/RULShaderGraphTask_ResolveOutput.h"

void URULShaderGraphUtility::AddTask(
    URULShaderGraph& Graph,
    URULShaderGraphTask& Task,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask
    )
{
    Task.SetTaskConfig(TaskConfig, ConfigMethod);
    Task.SetOutputTask(OutputTask);
    Graph.AddTask(&Task);
}

void URULShaderGraphUtility::AddTask(URULShaderGraph& Graph, URULShaderGraphTask& Task)
{
    Graph.AddTask(&Task);
}

URULShaderGraphTask* URULShaderGraphUtility::AddTaskByInstance(
    URULShaderGraph* Graph,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    URULShaderGraphTask* TaskInstance
    )
{
    if (IsValid(Graph) && IsValid(TaskInstance))
    {
        AddTask(*Graph, *TaskInstance, TaskConfig, ConfigMethod, OutputTask);
    }

    return TaskInstance;
}

URULShaderGraphTask_ResolveOutput* URULShaderGraphUtility::AddResolveTask(
    URULShaderGraph* Graph,
    URULShaderGraphTask* SourceTask,
    UTextureRenderTarget2D* RenderTargetTexture
    )
{
    URULShaderGraphTask_ResolveOutput* Task = nullptr;

    if (IsValid(Graph))
    {
        Task = NewObject<URULShaderGraphTask_ResolveOutput>(Graph);

        if (IsValid(Task))
        {
            Task->SourceTask = SourceTask;
            Task->RenderTargetTexture = RenderTargetTexture;
            AddTask(*Graph, *Task);
        }
    }

    return Task;
}

URULShaderGraphTask_DrawTaskToOutput* URULShaderGraphUtility::AddDrawTaskToOutputTask(
    URULShaderGraph* Graph,
    const FRULShaderGraphTaskConfig& TaskConfig,
    URULShaderGraphTask* SourceTask,
    FName OutputName
    )
{
    URULShaderGraphTask_DrawTaskToOutput* Task = nullptr;

    if (IsValid(Graph))
    {
        Task = NewObject<URULShaderGraphTask_DrawTaskToOutput>(Graph);

        if (IsValid(Task))
        {
            Task->SourceTask = SourceTask;
            Task->OutputName = OutputName;
            Task->TaskConfig.DrawConfig = TaskConfig.DrawConfig;
            AddTask(*Graph, *Task);
        }
    }

    return Task;
}

URULShaderGraphTask_DrawTaskToTexture* URULShaderGraphUtility::AddDrawTaskToTextureTask(
    URULShaderGraph* Graph,
    const FRULShaderGraphTaskConfig& TaskConfig,
    URULShaderGraphTask* SourceTask,
    UTextureRenderTarget2D* RenderTargetTexture
    )
{
    URULShaderGraphTask_DrawTaskToTexture* Task = nullptr;

    if (IsValid(Graph))
    {
        Task = NewObject<URULShaderGraphTask_DrawTaskToTexture>(Graph);

        if (IsValid(Task))
        {
            Task->SourceTask = SourceTask;
            Task->RenderTargetTexture = RenderTargetTexture;
            Task->TaskConfig.DrawConfig = TaskConfig.DrawConfig;
            AddTask(*Graph, *Task);
        }
    }

    return Task;
}

URULShaderGraphTask_DrawGeometry* URULShaderGraphUtility::AddDrawPointsTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_DrawGeometry> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const TArray<FVector2D>& Points,
    const TArray<FColor>& Colors,
    const TArray<int32>& Indices
    )
{
    URULShaderGraphTask_DrawGeometry* Task = nullptr;

    if (IsValid(Graph))
    {
        if (TaskType.Get())
        {
            Task = NewObject<URULShaderGraphTask_DrawGeometry>(Graph, TaskType);
        }
        else
        {
            Task = NewObject<URULShaderGraphTask_DrawGeometry>(Graph);
        }

        if (IsValid(Task))
        {
            TArray<FVector> Vertices;
            Vertices.Reserve(Points.Num());

            for (int32 i=0; i<Points.Num(); ++i)
            {
                Vertices.Emplace(Points[i], 1.f);
            }

            Task->Vertices = Vertices;
            Task->Colors = Colors;
            Task->Indices = Indices;

            AddTask(*Graph, *Task, TaskConfig, ConfigMethod, OutputTask);
        }
    }

    return Task;
}

URULShaderGraphTask_DrawGeometry* URULShaderGraphUtility::AddDrawGeometryTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_DrawGeometry> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const TArray<FVector>& Vertices,
    const TArray<FColor>& Colors,
    const TArray<int32>& Indices
    )
{
    URULShaderGraphTask_DrawGeometry* Task = nullptr;

    if (IsValid(Graph))
    {
        if (TaskType.Get())
        {
            Task = NewObject<URULShaderGraphTask_DrawGeometry>(Graph, TaskType);
        }
        else
        {
            Task = NewObject<URULShaderGraphTask_DrawGeometry>(Graph);
        }

        if (IsValid(Task))
        {
            Task->Vertices = Vertices;
            Task->Colors = Colors;
            Task->Indices = Indices;
            AddTask(*Graph, *Task, TaskConfig, ConfigMethod, OutputTask);
        }
    }

    return Task;
}

URULShaderGraphTask_ApplyMaterial* URULShaderGraphUtility::AddApplyMaterialTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef
    )
{
    URULShaderGraphTask_ApplyMaterial* Task = nullptr;

    if (IsValid(Graph))
    {
        if (TaskType.Get())
        {
            Task = NewObject<URULShaderGraphTask_ApplyMaterial>(Graph, TaskType);
        }
        else
        {
            Task = NewObject<URULShaderGraphTask_ApplyMaterial>(Graph);
        }

        if (IsValid(Task))
        {
            Task->MaterialRef = MaterialRef;
            AddTask(*Graph, *Task, TaskConfig, ConfigMethod, OutputTask);
        }
    }

    return Task;
}

URULShaderGraphTask_ApplyMaterial* URULShaderGraphUtility::AddApplyMaterialTaskWithParameters(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_ApplyMaterial> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FRULShaderScalarParameter>& ScalarParameters,
    const TArray<FRULShaderVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphTextureParameter>& TextureParameters
    )
{
    URULShaderGraphTask_ApplyMaterial* Task;
    Task = AddApplyMaterialTask(
        Graph,
        TaskType,
        TaskConfig,
        ConfigMethod,
        OutputTask,
        MaterialRef
        );

    if (IsValid(Task))
    {
        Task->SetParameters(ScalarParameters, VectorParameters, TextureParameters);
    }

    return Task;
}

URULShaderGraphTask_DrawMaterialQuad* URULShaderGraphUtility::AddDrawMaterialQuadTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_DrawMaterialQuad> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FGULQuadGeometryInstance>& Quads
    )
{
    URULShaderGraphTask_DrawMaterialQuad* Task = nullptr;

    if (IsValid(Graph))
    {
        if (TaskType.Get())
        {
            Task = NewObject<URULShaderGraphTask_DrawMaterialQuad>(Graph, TaskType);
        }
        else
        {
            Task = NewObject<URULShaderGraphTask_DrawMaterialQuad>(Graph);
        }

        if (IsValid(Task))
        {
            Task->MaterialRef = MaterialRef;
            Task->Quads = Quads;
            AddTask(*Graph, *Task, TaskConfig, ConfigMethod, OutputTask);
        }
    }

    return Task;
}

URULShaderGraphTask_DrawMaterialPoly* URULShaderGraphUtility::AddDrawMaterialPolyTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_DrawMaterialPoly> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FGULPolyGeometryInstance>& Polys
    )
{
    URULShaderGraphTask_DrawMaterialPoly* Task = nullptr;

    if (IsValid(Graph))
    {
        if (TaskType.Get())
        {
            Task = NewObject<URULShaderGraphTask_DrawMaterialPoly>(Graph, TaskType);
        }
        else
        {
            Task = NewObject<URULShaderGraphTask_DrawMaterialPoly>(Graph);
        }

        if (IsValid(Task))
        {
            Task->MaterialRef = MaterialRef;
            Task->Polys = Polys;
            AddTask(*Graph, *Task, TaskConfig, ConfigMethod, OutputTask);
        }
    }

    return Task;
}

URULShaderGraphTask_DrawMaterialQuad* URULShaderGraphUtility::AddDrawTextureQuadTask(
    URULShaderGraph* Graph,
    TSubclassOf<URULShaderGraphTask_DrawMaterialQuad> TaskType,
    const FRULShaderGraphTaskConfig& TaskConfig,
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod,
    URULShaderGraphTask* OutputTask,
    const FRULShaderGraphMaterialRef& MaterialRef,
    const TArray<FGULQuadGeometryInstance>& Quads,
    FName ParameterCategoryName,
    FRULShaderGraphTextureInput SourceTexture,
    float Opacity
    )
{
    URULShaderGraphTask_DrawMaterialQuad* Task;
    Task = AddDrawMaterialQuadTask(
        Graph,
        TaskType,
        TaskConfig,
        ConfigMethod,
        OutputTask,
        MaterialRef,
        Quads
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
            TEXT("DrawTextureQuad"),
            MappedScalars,
            { },
            MappedTextures
            );
    }

    return Task;
}
