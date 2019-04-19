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

#include "Shaders/Graph/RULShaderGraphTask.h"
#include "Shaders/Graph/RULShaderGraph.h"

URULShaderGraphTask::URULShaderGraphTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
    , ConfigMethod(RUL_CM_Parent)
    , bRequireOutput(true)
{
}

bool URULShaderGraphTask::IsTaskExecutionValid(const URULShaderGraph* Graph) const
{
    return IsValid(Graph) && Graph->HasGraphManager();
}

void URULShaderGraphTask::SetTaskConfig(const FRULShaderGraphTaskConfig& InTaskConfig, TEnumAsByte<enum ERULShaderGraphConfigMethod> InConfigMethod)
{
    TaskConfig = InTaskConfig;
    ConfigMethod = InConfigMethod;
}

void URULShaderGraphTask::Initialize(URULShaderGraph* Graph)
{
    // Blank implementation
}

void URULShaderGraphTask::Execute(URULShaderGraph* Graph)
{
    // Blank implementation
}

void URULShaderGraphTask::PostExecute(URULShaderGraph* Graph)
{
    check(IsValid(Graph));

    // Resolve output to dependant tasks
    for (FRULShaderGraphOutputRT* DependantOutput : DependantOutputList)
    {
        if (DependantOutput)
        {
            CopyOutputRef(*DependantOutput);
        }
    }
    DependantOutputList.Reset();

    // Clear dependency map
    DependencyMap.Reset();

    // Clear output RT
    Output = FRULShaderGraphOutputRT();
}

void URULShaderGraphTask::SetOutputTask(URULShaderGraphTask* InOutputTask)
{
    if (this != InOutputTask)
    {
        OutputTask = InOutputTask;
    }
}

bool URULShaderGraphTask::HasValidOutputRT() const
{
    return IsValid(Output.RenderTarget);
}

bool URULShaderGraphTask::HasValidOutputRefId() const
{
    return Output.RefId.IsValid();
}

void URULShaderGraphTask::GetResolvedOutputConfig(const URULShaderGraph& Graph, FRULShaderOutputConfig& OutputConfig) const
{
    Graph.ResolveTaskOutputConfig(*this, OutputConfig);
}

FRULShaderGraphOutputRT& URULShaderGraphTask::GetOutputRef()
{
    return Output;
}

void URULShaderGraphTask::CopyOutputRef(FRULShaderGraphOutputRT& OutRef)
{
    OutRef = Output;
}

void URULShaderGraphTask::ResolveOutputDependency(const URULShaderGraph& Graph)
{
    // Resolve dependency map
    for (auto& Dependency : DependencyMap)
    {
        FDependencyData& DependencyData(Dependency.Value);
        URULShaderGraphTask* Task(DependencyData.Task);

        if (IsValid(Task))
        {
            Task->LinkOutputDependency(DependencyData.Output);
        }
    }

    // Resolve assigned output task dependency
    if (IsValid(OutputTask))
    {
        FRULShaderOutputConfig SrcResolvedOutputConfig;
        FRULShaderOutputConfig DstResolvedOutputConfig;

        GetResolvedOutputConfig(Graph, SrcResolvedOutputConfig);
        OutputTask->GetResolvedOutputConfig(Graph, DstResolvedOutputConfig);

        if (SrcResolvedOutputConfig.Compare(DstResolvedOutputConfig))
        {
            OutputTask->LinkOutputDependency(Output);
        }
    }
}

void URULShaderGraphTask::LinkOutputDependency(FRULShaderGraphOutputRT& OutRef)
{
    DependantOutputList.Emplace(&OutRef);
}

UTextureRenderTarget2D* URULShaderGraphTask::GetOutputRTFromDependencyMap(FName OutputName) const
{
    const FDependencyData* DependencyData = DependencyMap.Find(OutputName);
    return DependencyData ? DependencyData->Output.RenderTarget : nullptr;
}
