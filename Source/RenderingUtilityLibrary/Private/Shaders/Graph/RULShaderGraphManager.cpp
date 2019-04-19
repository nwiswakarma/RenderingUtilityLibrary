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

#include "Shaders/Graph/RULShaderGraphManager.h"
#include "Kismet/KismetRenderingLibrary.h"

#if WITH_EDITOR
void URULShaderGraphManager::CheckCallInEditor()
{
}
#endif

void URULShaderGraphManager::K2_ExecuteGraph(URULShaderGraph* OptGraph)
{
    if (IsValid(Graph) && Graph->IsExecutionInProgress())
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderGraphUtility::K2_ExecuteGraph() ABORTED, GRAPH EXECUTION IS IN PROGRESS"));
        return;
    }

    Initialize(OptGraph);
    Execute();
}

void URULShaderGraphManager::K2_ClearOutputs()
{
    ClearOutputRTs();
}

void URULShaderGraphManager::Reset()
{
    Graph = nullptr;
}

void URULShaderGraphManager::Initialize(URULShaderGraph* OptGraph)
{
    check(! IsValid(Graph) || ! Graph->IsExecutionInProgress());

    if (IsValid(OptGraph))
    {
        Graph = OptGraph;
    }
    else
    {
        UClass* GraphClass = *GraphType;

        if (! IsValid(GraphClass))
        {
            UE_LOG(LogRUL,Warning, TEXT("URULShaderGraphUtility::Initialize() ABORTED, INVALID GRAPH TYPE"));
            return;
        }

        if (! IsValid(Graph) || Graph->GetClass() != GraphClass)
        {
            Graph = NewObject<URULShaderGraph>(this, GraphClass);
        }
    }
}

void URULShaderGraphManager::Execute()
{
    if (IsValid(Graph))
    {
        check(! Graph->IsExecutionInProgress());
        Graph->K2_PrepareGraph(this);
        Graph->ExecuteGraph(this);
    }
}

int32 URULShaderGraphManager::FindFreeRTIndex(const FRULShaderOutputConfig& OutputConfig)
{
    int32 OutputIndex = RenderTargetPool.IndexOfByPredicate(
        [&OutputConfig](const FRULShaderGraphOutputRT& Output)
        {
            return Output.IsValidOutput() && Output.CompareFormat(OutputConfig);
        } );
    return OutputIndex;
}

void URULShaderGraphManager::FindFreeOutputRT(const FRULShaderOutputConfig& OutputConfig, FRULShaderGraphOutputRT& OutputRT)
{
    int32 OutputIndex = FindFreeRTIndex(OutputConfig);

    // Reuse free output
    if (OutputIndex >= 0)
    {
        check(RenderTargetPool[OutputIndex].CompareFormat(OutputConfig));
        check(RenderTargetPool[OutputIndex].IsValidOutput());

        OutputRT = RenderTargetPool[OutputIndex];
    }
    // Create new output
    else
    {
        UTextureRenderTarget2D* NewRenderTarget;
        NewRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(
            this,
            OutputConfig.SizeX,
            OutputConfig.SizeY,
            OutputConfig.Format
            );

        if (IsValid(NewRenderTarget))
        {
            OutputIndex = RenderTargetPool.Num();
            RenderTargetPool.Emplace(NewRenderTarget);

            OutputRT = RenderTargetPool[OutputIndex];
        }
    }
}

void URULShaderGraphManager::ClearOutputRTs()
{
    RenderTargetPool.Empty();
}

UMaterialInstanceDynamic* URULShaderGraphManager::GetCachedMID(UMaterialInterface* BaseMaterial, bool bClearParameterValues)
{
    auto* MIDPtr = BasedMIDCacheMap.Find(BaseMaterial);
    UMaterialInstanceDynamic* MID = nullptr;

    if (MIDPtr)
    {
        MID = *MIDPtr;

        if (IsValid(MID) && bClearParameterValues)
        {
            MID->ClearParameterValues();
        }
    }
    else
    {
        if (IsValid(BaseMaterial))
        {
            MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);

            if (IsValid(MID))
            {
                BasedMIDCacheMap.Emplace(BaseMaterial, MID);
            }
        }
        else
        {
            UE_LOG(LogRUL,Warning, TEXT("URULShaderGraphUtility::GetCachedMID() ABORTED, INVALID MATERIAL"));
        }
    }

    return MID;
}

UMaterialInstanceDynamic* URULShaderGraphManager::GetCachedMID(FName MaterialName, bool bClearParameterValues)
{
    auto* MIDPtr = NamedMIDCacheMap.Find(MaterialName);
    UMaterialInstanceDynamic* MID = nullptr;

    if (MIDPtr)
    {
        MID = *MIDPtr;

        if (IsValid(MID) && bClearParameterValues)
        {
            MID->ClearParameterValues();
        }
    }
    else
    {
        UMaterialInterface* Material = nullptr;

        const URULShaderMaterialLibrary* Library = URenderingUtilitySettings::GetMaterialLibrary();

        if (IsValid(Library))
        {
            Material = Library->GetMaterial(MaterialName);

            if (IsValid(Material))
            {
                MID = UMaterialInstanceDynamic::Create(Material, this);

                if (IsValid(MID))
                {
                    NamedMIDCacheMap.Emplace(MaterialName, MID);
                }
            }
            else
            {
                UE_LOG(LogRUL,Warning, TEXT("URULShaderGraphUtility::GetCachedMID() ABORTED, SOURCE MATERIAL '%s' NOT FOUND"), *MaterialName.ToString());
            }
        }
        else
        {
            UE_LOG(LogRUL,Warning, TEXT("URULShaderGraphUtility::GetCachedMID() ABORTED, MATERIAL LIBRARY NOT FOUND! (Check Rendering Utility Settings project settings)"));
        }
    }

    return MID;
}
