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

#include "Shaders/Graph/Tasks/RULShaderGraphTask_ApplyMaterial.h"
#include "Shaders/RULShaderLibrary.h"
#include "Shaders/Graph/RULShaderGraph.h"

void URULShaderGraphTask_ApplyMaterial::Initialize(URULShaderGraph* Graph)
{
    check(IsValid(Graph));

    // Register task inputs to dependency map

    for (const auto& InputPair : TextureInputMap)
    {
        const FName& InputKey(InputPair.Key);
        const FRULShaderGraphTaskTextureInput& InputValue(InputPair.Value);

        UTexture* Texture(InputValue.Texture);
        URULShaderGraphTask* Task(InputValue.Task);

        if (IsValid(Texture))
        {
            ResolvedTextureInputMap.Emplace(InputKey, Texture);
        }
        else
        if (IsValid(Task))
        {
            DependencyMap.Emplace(InputKey, Task);
        }
    }
}

void URULShaderGraphTask_ApplyMaterial::Execute(URULShaderGraph* Graph)
{
    check(IsValid(Graph));

    // Resolve task output as texture input
    ResolveTaskInputMap();

    // Resolve material instance to apply

    UMaterialInstanceDynamic* MID = nullptr;

    // Create new MID from material interface
    if (MaterialRef.Material)
    {
        MID = Graph->GetCachedMID(MaterialRef.Material, true);
    }
    // Use cached MID with the specified material name
    else
    {
        MID = Graph->GetCachedMID(MaterialRef.MaterialName, true);
    }

    if (IsValid(MID))
    {
        ExecuteMaterialFunction(*Graph, *MID);
    }
}

void URULShaderGraphTask_ApplyMaterial::ExecuteMaterialFunction(URULShaderGraph& Graph, UMaterialInstanceDynamic& MID)
{
    ApplyMaterialParameters(MID);
    URULShaderLibrary::ApplyMaterial(
        Graph.GetGraphManager(),
        &MID,
        Output.RenderTarget,
        TaskConfig.DrawConfig
        );
}

void URULShaderGraphTask_ApplyMaterial::SetScalarParameterValue(FName ParameterName, float ParameterValue)
{
    ScalarInputMap.Emplace(ParameterName, ParameterValue);
}

void URULShaderGraphTask_ApplyMaterial::SetVectorParameterValue(FName ParameterName, FLinearColor ParameterValue)
{
    VectorInputMap.Emplace(ParameterName, ParameterValue);
}

void URULShaderGraphTask_ApplyMaterial::SetTextureParameterValue(FName ParameterName, FRULShaderGraphTaskTextureInput ParameterValue)
{
    TextureInputMap.Emplace(ParameterName, ParameterValue);
}

void URULShaderGraphTask_ApplyMaterial::SetScalarParameter(const FRULShaderGraphMaterialScalarParameter& Parameter)
{
    ScalarInputMap.Emplace(Parameter.ParameterName, Parameter.ParameterValue);
}

void URULShaderGraphTask_ApplyMaterial::SetVectorParameter(const FRULShaderGraphMaterialVectorParameter& Parameter)
{
    VectorInputMap.Emplace(Parameter.ParameterName, Parameter.ParameterValue);
}

void URULShaderGraphTask_ApplyMaterial::SetTextureParameter(const FRULShaderGraphMaterialTextureParameter& Parameter)
{
    TextureInputMap.Emplace(Parameter.ParameterName, Parameter.ParameterValue);
}

void URULShaderGraphTask_ApplyMaterial::SetParameters(
    const TArray<FRULShaderGraphMaterialScalarParameter>& ScalarParameters,
    const TArray<FRULShaderGraphMaterialVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphMaterialTextureParameter>& TextureParameters
    )
{
    for (const auto& ScalarParameter : ScalarParameters)
    {
        SetScalarParameter(ScalarParameter);
    }

    for (const auto& VectorParameter : VectorParameters)
    {
        SetVectorParameter(VectorParameter);
    }

    for (const auto& TextureParameter : TextureParameters)
    {
        SetTextureParameter(TextureParameter);
    }
}

void URULShaderGraphTask_ApplyMaterial::SetParameters(
    const FRULShaderGraphParameterNameMap& ParameterNameMap,
    const TArray<FRULShaderGraphMaterialScalarParameter>& ScalarParameters,
    const TArray<FRULShaderGraphMaterialVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphMaterialTextureParameter>& TextureParameters
    )
{
    for (const auto& Param : ScalarParameters)
    {
        FName ParameterName(ParameterNameMap.GetOrDefault(Param.ParameterName));
        if (ParameterName.IsValid())
        {
            SetScalarParameterValue(ParameterName, Param.ParameterValue);
        }
    }

    for (const auto& Param : VectorParameters)
    {
        FName ParameterName(ParameterNameMap.GetOrDefault(Param.ParameterName));
        if (ParameterName.IsValid())
        {
            SetVectorParameterValue(ParameterName, Param.ParameterValue);
        }
    }

    for (const auto& Param : TextureParameters)
    {
        FName ParameterName(ParameterNameMap.GetOrDefault(Param.ParameterName));
        if (ParameterName.IsValid())
        {
            SetTextureParameterValue(ParameterName, Param.ParameterValue);
        }
    }
}

void URULShaderGraphTask_ApplyMaterial::SetParameters(
    URULShaderGraph& Graph,
    FName ParameterCategoryName,
    FName ParameterCategoryDefaultName,
    const TArray<FRULShaderGraphMaterialScalarParameter>& ScalarParameters,
    const TArray<FRULShaderGraphMaterialVectorParameter>& VectorParameters,
    const TArray<FRULShaderGraphMaterialTextureParameter>& TextureParameters
    )
{
    if (! ParameterCategoryName.IsValid())
    {
        ParameterCategoryName = ParameterCategoryDefaultName;
    }

    FRULShaderGraphParameterNameMap DefaultNameMap;
    const FRULShaderGraphParameterNameMap* ParamNameMapPtr = Graph.GetParameterNameMap(ParameterCategoryName);
    const FRULShaderGraphParameterNameMap& ParamNameMap(ParamNameMapPtr ? *ParamNameMapPtr : DefaultNameMap);

    SetParameters(
        ParamNameMap,
        ScalarParameters,
        VectorParameters,
        TextureParameters
        );
}

void URULShaderGraphTask_ApplyMaterial::ResolveTaskInputMap()
{
    for (const auto& DataPair : DependencyMap)
    {
        FName Key(DataPair.Key);
        UTexture* Texture = DataPair.Value.Output.RenderTarget;

        if (IsValid(Texture))
        {
            ResolvedTextureInputMap.Emplace(Key, Texture);
        }
    }
}

void URULShaderGraphTask_ApplyMaterial::ApplyMaterialParameters(UMaterialInstanceDynamic& MID)
{
    // Apply scalar parameters
    for (const auto& ParameterData : ScalarInputMap)
    {
        MID.SetScalarParameterValue(ParameterData.Key, ParameterData.Value);
    }

    // Apply vector parameters
    for (const auto& ParameterData : VectorInputMap)
    {
        MID.SetVectorParameterValue(ParameterData.Key, ParameterData.Value);
    }

    // Apply texture parameters
    for (const auto& ParameterData : ResolvedTextureInputMap)
    {
        MID.SetTextureParameterValue(ParameterData.Key, ParameterData.Value);
    }
}
