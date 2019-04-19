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
#include "Shaders/Graph/RULShaderGraphTypes.h"
#include "RULShaderGraphTask.generated.h"

class UTextureRenderTarget2D;
class URULShaderGraph;

UCLASS(Abstract, BlueprintType, Blueprintable)
class RENDERINGUTILITYLIBRARY_API URULShaderGraphTask : public UObject
{
	GENERATED_UCLASS_BODY()

protected:

    struct FDependencyData
    {
        URULShaderGraphTask* Task;
        FRULShaderGraphOutputRT Output;

        FDependencyData() : Task(nullptr) {};
        explicit FDependencyData(URULShaderGraphTask* InTask) : Task(InTask) {}
    };

    FRULShaderGraphOutputRT Output;
    TArray<FRULShaderGraphOutputRT*> DependantOutputList;
    TMap<FName, FDependencyData> DependencyMap;

    UPROPERTY()
    URULShaderGraphTask* OutputTask;

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FRULShaderGraphTaskConfig TaskConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TEnumAsByte<enum ERULShaderGraphConfigMethod> ConfigMethod;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bRequireOutput;

    UFUNCTION(BlueprintCallable)
    bool IsTaskExecutionValid(const URULShaderGraph* Graph) const;

    UFUNCTION(BlueprintCallable)
    void SetTaskConfig(const FRULShaderGraphTaskConfig& InTaskConfig, TEnumAsByte<enum ERULShaderGraphConfigMethod> InConfigMethod);

    UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="Initialize"))
    void K2_Initialize(URULShaderGraph* Graph);

    virtual void Initialize(URULShaderGraph* Graph);
    virtual void Execute(URULShaderGraph* Graph);
    virtual void PostExecute(URULShaderGraph* Graph);

    FORCEINLINE bool IsOutputRequired() const
    {
        return bRequireOutput;
    }

    FORCEINLINE URULShaderGraphTask* GetOutputTask() const
    {
        return OutputTask;
    }

    FORCEINLINE bool HasValidOutput() const
    {
        return HasValidOutputRT() && HasValidOutputRefId();
    }

    void SetOutputTask(URULShaderGraphTask* InOutputTask);
    bool HasValidOutputRT() const;
    bool HasValidOutputRefId() const;

    void GetResolvedOutputConfig(const URULShaderGraph& Graph, FRULShaderOutputConfig& OutputConfig) const;

    FRULShaderGraphOutputRT& GetOutputRef();
    void CopyOutputRef(FRULShaderGraphOutputRT& OutRef);

    void ResolveOutputDependency(const URULShaderGraph& Graph);
    void LinkOutputDependency(FRULShaderGraphOutputRT& OutRef);

    UTextureRenderTarget2D* GetOutputRTFromDependencyMap(FName OutputName) const;
};
