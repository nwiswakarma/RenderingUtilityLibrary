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
#include "RHIResources.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Shaders/RULShaderGeometry.h"
#include "Shaders/RULShaderParameters.h"
#include "RULShaderLibrary.generated.h"

class FGraphicsPipelineStateInitializer;
class UTexture2D;
class FTextureRenderTarget2DResource;
class UMaterialInterface;
class FMaterialRenderProxy;
class FMaterialShader;
class FSceneView;
class UGWTTickEvent;

USTRUCT(BlueprintType)
struct RENDERINGUTILITYLIBRARY_API FRULTextureValuesRef
{
    GENERATED_BODY()

    struct FValuesRef
    {
        TArray<FLinearColor> Values;
    };

    typedef TSharedPtr<FValuesRef, ESPMode::ThreadSafe> FSharedRefType;

    FSharedRefType SharedRef;

    void ClearValues()
    {
        if (SharedRef.IsValid())
        {
            SharedRef->Values.Empty();
        }
    }

    void GetValuesFromRef(TArray<FLinearColor>& Values) const
    {
        if (SharedRef.IsValid())
        {
            Values = SharedRef->Values;
        }
    }
};

UCLASS()
class RENDERINGUTILITYLIBRARY_API URULShaderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

    static FVertexBufferRHIRef& GetFilterShaderVB();
    static ERenderTargetActions GetRenderTargetActions(FRULShaderDrawConfig DrawConfig);

    static bool IsValidSwapTarget(UTextureRenderTarget2D* RTT0, UTextureRenderTarget2D* RTT1);
    static bool IsValidSwapTarget_RT(FTexture2DRHIParamRef Tex0, FTexture2DRHIParamRef Tex1);

    static void AssignBlendState(FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERULShaderDrawBlendType BlendType);

    static void SetupDefaultGraphicsPSOInit(
        FGraphicsPipelineStateInitializer& GraphicsPSOInit,
        EPrimitiveType PrimitiveType,
        const FRULShaderDrawConfig& DrawConfig
        );

    static void SetupDefaultMaterialParameters(
        FRHICommandListImmediate& RHICmdList,
        ERHIFeatureLevel::Type FeatureLevel,
        FShader* VertexShader,
        FMaterialShader* MaterialShader,
        const FMaterialRenderProxy& MaterialRenderProxy,
        FSceneView& View
        );

    static TSharedRef<FSceneView> CreateDefaultRTView(
        FRHICommandListImmediate& RHICmdList,
        FTextureRenderTarget2DResource* RTResource,
        FIntRect& ViewRect
        );

public:

    UFUNCTION(BlueprintCallable)
    static void CopyToResolveTarget(
        UObject* WorldContextObject,
        FRULShaderTextureParameterInput SourceTexture,
        UTextureRenderTarget2D* RenderTarget,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    UFUNCTION(BlueprintCallable)
    static void DrawPoints(
        UObject* WorldContextObject,
        UTextureRenderTarget2D* RenderTarget,
        FRULShaderDrawConfig DrawConfig,
        FIntPoint DrawSize,
        const TArray<FVector2D>& Points,
        const TArray<int32>& Indices,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    UFUNCTION(BlueprintCallable)
    static void DrawGeometry(
        UObject* WorldContextObject,
        UTextureRenderTarget2D* RenderTarget,
        FRULShaderDrawConfig DrawConfig,
        FIntPoint DrawSize,
        const TArray<FVector>& Vertices,
        const TArray<int32>& Indices,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    UFUNCTION(BlueprintCallable)
    static void DrawGeometryColors(
        UObject* WorldContextObject,
        UTextureRenderTarget2D* RenderTarget,
        FRULShaderDrawConfig DrawConfig,
        FIntPoint DrawSize,
        const TArray<FVector>& Vertices,
        const TArray<FColor>& Colors,
        const TArray<int32>& Indices,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    // Draw indexed primitive to a render target
    // with vertex color stored in vertex Z component
    static void DrawGeometry_RT(
        FRHICommandListImmediate& RHICmdList,
        ERHIFeatureLevel::Type FeatureLevel,
        FTextureRenderTarget2DResource* RenderTargetResource,
        FRULShaderDrawConfig DrawConfig,
        FIntPoint DrawSize,
        const TArray<FVector>& Vertices,
        const TArray<int32>& Indices,
        const TArray<FColor>* Colors = nullptr
        );

    UFUNCTION(BlueprintCallable)
    static void ApplyMaterial(
        UObject* WorldContextObject,
        UMaterialInterface* Material,
        UTextureRenderTarget2D* RenderTarget,
        FRULShaderDrawConfig DrawConfig,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    static void ApplyMaterial_RT(
        FRHICommandListImmediate& RHICmdList,
        ERHIFeatureLevel::Type FeatureLevel,
        FTextureRenderTarget2DResource* RenderTargetResource,
        const FMaterialRenderProxy* MaterialRenderProxy,
        FRULShaderDrawConfig DrawConfig
        );

    UFUNCTION(BlueprintCallable)
    static void ApplyMaterialFilter(
        UObject* WorldContextObject,
        UMaterialInterface* Material,
        int32 RepeatCount,
        FRULShaderDrawConfig DrawConfig,
        FRULShaderTextureParameterInput SourceTexture,
        UTextureRenderTarget2D* RenderTarget,
        UTextureRenderTarget2D* SwapTarget = nullptr,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    static void ApplyMaterialFilter_RT(
        FRHICommandListImmediate& RHICmdList,
        ERHIFeatureLevel::Type FeatureLevel,
        int32 RepeatCount,
        FRULShaderDrawConfig DrawConfig,
        FRULShaderTextureParameterInputResource SourceTextureResource,
        FTextureRenderTarget2DResource* RenderTargetResource,
        FTextureRenderTarget2DResource* SwapTargetResource,
        const FMaterialRenderProxy* MaterialRenderProxy
        );

    UFUNCTION(BlueprintCallable)
    static void DrawMaterialQuad(
        UObject* WorldContextObject,
        const TArray<FRULShaderQuadGeometry>& Quads,
        UMaterialInterface* Material,
        UTextureRenderTarget2D* RenderTarget,
        FRULShaderDrawConfig DrawConfig,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    static void DrawMaterialQuad_RT(
        FRHICommandListImmediate& RHICmdList,
        ERHIFeatureLevel::Type FeatureLevel,
        const TArray<FRULShaderQuadGeometry>& Quads,
        FTextureRenderTarget2DResource* RenderTargetResource,
        const FMaterialRenderProxy* MaterialRenderProxy,
        FRULShaderDrawConfig DrawConfig
        );

    UFUNCTION(BlueprintCallable)
    static void DrawMaterialPoly(
        UObject* WorldContextObject,
        const TArray<FRULShaderPolyGeometry>& Polys,
        UMaterialInterface* Material,
        UTextureRenderTarget2D* RenderTarget,
        FRULShaderDrawConfig DrawConfig,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    static void DrawMaterialPoly_RT(
        FRHICommandListImmediate& RHICmdList,
        ERHIFeatureLevel::Type FeatureLevel,
        const TArray<FVector4>& Vertices,
        const TArray<int32>& Indices,
        FTextureRenderTarget2DResource* RenderTargetResource,
        const FMaterialRenderProxy* MaterialRenderProxy,
        FRULShaderDrawConfig DrawConfig
        );

    UFUNCTION(BlueprintCallable)
    static FRULTextureValuesRef GetTextureValuesByPoints(
        UObject* WorldContextObject,
        FRULShaderTextureParameterInput SourceTexture,
        const FVector2D ScaleDimension,
        const TArray<FVector2D>& Points,
        UGWTTickEvent* CallbackEvent = nullptr
        );

    static void GetTextureValuesByPoints_RT(
        FRHICommandListImmediate& RHICmdList,
        ERHIFeatureLevel::Type FeatureLevel,
        FRULShaderTextureParameterInputResource TextureResource,
        const FVector2D ScaleDimension,
        const TArray<FVector2D>& Points,
        FRULTextureValuesRef::FSharedRefType ValuesRef
        );

    UFUNCTION(BlueprintCallable)
    static TArray<FLinearColor> GetTextureValuesOutput(const FRULTextureValuesRef& ValuesRef);

    UFUNCTION(BlueprintCallable)
    static void GetTextureValuesOutputByReference(const FRULTextureValuesRef& ValuesRef, UPARAM(ref) TArray<FLinearColor>& Values);

    UFUNCTION(BlueprintCallable)
    static void ClearTextureValuesOutput(UPARAM(ref) FRULTextureValuesRef& ValuesRef);

    UFUNCTION(BlueprintCallable)
    static void TestGPUCompute(
        UObject* WorldContextObject,
        int32 TestCount,
        UGWTTickEvent* CallbackEvent = nullptr
        );
};
