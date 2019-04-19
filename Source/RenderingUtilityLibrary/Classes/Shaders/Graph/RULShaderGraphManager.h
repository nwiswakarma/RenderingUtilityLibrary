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
#include "RULShaderGraphManager.generated.h"

class URULShaderGraph;

UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class RENDERINGUTILITYLIBRARY_API URULShaderGraphManager : public UActorComponent
{
	GENERATED_BODY()

    UPROPERTY()
    URULShaderGraph* Graph;

    UPROPERTY(EditAnywhere)
    TArray<FRULShaderGraphOutputRT> RenderTargetPool;

    UPROPERTY()
    TMap<UMaterialInterface*, UMaterialInstanceDynamic*> BasedMIDCacheMap;

    UPROPERTY()
    TMap<FName, UMaterialInstanceDynamic*> NamedMIDCacheMap;

    int32 FindFreeRTIndex(const FRULShaderOutputConfig& OutputConfig);

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DisplayName="Shader Graph Type"))
    TSubclassOf<URULShaderGraph> GraphType;

#if WITH_EDITOR
	UFUNCTION(meta=(CallInEditor="true"))
    void CheckCallInEditor();
#endif

    UFUNCTION(BlueprintCallable, meta=(DisplayName="Execute Graph"))
    void K2_ExecuteGraph(URULShaderGraph* OptGraph);

    UFUNCTION(BlueprintCallable, meta=(DisplayName="Clear Outputs"))
    void K2_ClearOutputs();

    void Reset();
    void Initialize(URULShaderGraph* OptGraph);
    void Execute();

    void FindFreeOutputRT(const FRULShaderOutputConfig& OutputConfig, FRULShaderGraphOutputRT& OutputRT);
    void ClearOutputRTs();

    UMaterialInstanceDynamic* GetCachedMID(UMaterialInterface* BaseMaterial, bool bClearParameterValues = false);
    UMaterialInstanceDynamic* GetCachedMID(FName MaterialName, bool bClearParameterValues = false);
};
