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

#include "Shaders/RULShaderLibrary.h"

#include "BatchedElements.h"
#include "CanvasTypes.h"
#include "EngineModule.h"
#include "HitProxies.h"
#include "MeshPassProcessor.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "RHIStaticStates.h"
#include "RHIResources.h"
#include "RenderResource.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"
#include "ShaderParameterUtils.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceSupport.h"
#include "Kismet/KismetRenderingLibrary.h"

#include "GWTTickUtilities.h"

#include "RenderingUtilityLibrary.h"
#include "RHI/RULRHIUtilityLibrary.h"
#include "Shaders/RULShaderDefinitions.h"
#include "Shaders/RULPrefixSumScan.h"
#include "Shaders/RULReduceScan.h"

class FRULColorGeometryVertexDeclaration : public FRenderResource
{
public:

	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		Elements.Add(FVertexElement(0, 0, VET_Float3, 0, sizeof(FVector)));
		Elements.Add(FVertexElement(1, 0, VET_Color,  1, sizeof(FColor)));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}
	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FRULColorGeometryVertexDeclaration> GRULColorGeometryVertexDeclaration;

template<uint32 bEnableVertexColor>
class TRULShaderDrawGeometryVS : public FRULBaseVertexShader
{
public:

    typedef FRULBaseVertexShader FBaseType;

    DECLARE_SHADER_TYPE(TRULShaderDrawGeometryVS, Global);

public:

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
		return true;
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FBaseType::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.SetDefine(TEXT("RUL_ENABLE_VERTEX_COLOR"), (bEnableVertexColor>0) ? 1 : 0);
    }

    RUL_DECLARE_SHADER_CONSTRUCTOR_SERIALIZER(TRULShaderDrawGeometryVS)

    RUL_DECLARE_SHADER_PARAMETERS_0(SRV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(UAV,,)

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Value,
        FShaderParameter,
        FParameterId,
        "_DrawScaleBias" , Params_DrawScaleBias
        )
};

class FRULShaderDrawGeometryPS : public FRULBasePixelShader
{
    typedef FRULBasePixelShader FBaseType;

    RUL_DECLARE_SHADER_CONSTRUCTOR_DEFAULT_STATICS(FRULShaderDrawGeometryPS, Global, true)

    RUL_DECLARE_SHADER_PARAMETERS_0(SRV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(UAV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(Value,,)
};

IMPLEMENT_SHADER_TYPE(template<>, TRULShaderDrawGeometryVS<0>, TEXT("/Plugin/RenderingUtilityLibrary/Private/RULDrawGeometryVSPS.usf"), TEXT("DrawGeometryVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(template<>, TRULShaderDrawGeometryVS<1>, TEXT("/Plugin/RenderingUtilityLibrary/Private/RULDrawGeometryVSPS.usf"), TEXT("DrawGeometryVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FRULShaderDrawGeometryPS, TEXT("/Plugin/RenderingUtilityLibrary/Private/RULDrawGeometryVSPS.usf"), TEXT("DrawGeometryPS"), SF_Pixel);

class FRULShaderDrawScreenVS : public FRULBaseVertexShader
{
    typedef FRULBaseVertexShader FBaseType;

    RUL_DECLARE_SHADER_CONSTRUCTOR_DEFAULT_STATICS(FRULShaderDrawScreenVS, Global, true)

    RUL_DECLARE_SHADER_PARAMETERS_0(SRV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(UAV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(Value,,)
};

template<uint32 bEnableVertexColor>
class TRULShaderDrawScreenMS : public FRULBaseMaterialShader
{
public:

    typedef FRULBaseMaterialShader FBaseType;

    DECLARE_SHADER_TYPE(TRULShaderDrawScreenMS, Material);

public:

    static bool ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material)
    {
		return Material->IsUIMaterial() || Material->GetMaterialDomain() == MD_PostProcess;
    }

    static void ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment)
    {
        FMaterialShader::ModifyCompilationEnvironment(Platform, Material, OutEnvironment);
        OutEnvironment.SetDefine(TEXT("RUL_USE_SOURCE_MAP"), 1);
        OutEnvironment.SetDefine(TEXT("RUL_ENABLE_VERTEX_COLOR"), (bEnableVertexColor>0) ? 1 : 0);
        OutEnvironment.SetDefine(TEXT("RUL_FILTER_HAS_VALID_MATERIAL_DOMAIN"), ShouldCompilePermutation(Platform, Material) ? 1 : 0);
    }

    RUL_DECLARE_SHADER_CONSTRUCTOR_SERIALIZER_WITH_TEXTURE(TRULShaderDrawScreenMS)

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Texture,
        FShaderResourceParameter,
        FResourceId,
        "SourceMap", SourceMap
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Sampler,
        FShaderResourceParameter,
        FResourceId,
        "SourceMapSampler", SourceMapSampler
        )

    RUL_DECLARE_SHADER_PARAMETERS_0(SRV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(UAV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(Value,,)
};

IMPLEMENT_SHADER_TYPE(, FRULShaderDrawScreenVS, TEXT("/Plugin/RenderingUtilityLibrary/Private/RULDrawGeometryVSPS.usf"), TEXT("DrawScreenVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TRULShaderDrawScreenMS<0>, TEXT("/Plugin/RenderingUtilityLibrary/Private/RULDrawScreenMS.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TRULShaderDrawScreenMS<1>, TEXT("/Plugin/RenderingUtilityLibrary/Private/RULDrawScreenMS.usf"), TEXT("MainPS"), SF_Pixel);

class FRULShaderDrawQuadVS : public FRULBaseVertexShader
{
    typedef FRULBaseVertexShader FBaseType;

    RUL_DECLARE_SHADER_CONSTRUCTOR_DEFAULT_STATICS(FRULShaderDrawQuadVS, Global, true)

    RUL_DECLARE_SHADER_PARAMETERS_2(
        SRV,
        FShaderResourceParameter,
        FResourceId,
        "QuadGeomData", QuadGeomData,
        "QuadTransformData", QuadTransformData
        )

    RUL_DECLARE_SHADER_PARAMETERS_0(UAV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(Value,,)
};

IMPLEMENT_SHADER_TYPE(, FRULShaderDrawQuadVS, TEXT("/Plugin/RenderingUtilityLibrary/Private/RULDrawGeometryVSPS.usf"), TEXT("DrawQuadVS"), SF_Vertex);

class FRULShaderDrawPolyVS : public FRULBaseVertexShader
{
    typedef FRULBaseVertexShader FBaseType;

    RUL_DECLARE_SHADER_CONSTRUCTOR_DEFAULT_STATICS(FRULShaderDrawPolyVS, Global, true)

    RUL_DECLARE_SHADER_PARAMETERS_0(SRV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(UAV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(Value,,)
};

IMPLEMENT_SHADER_TYPE(, FRULShaderDrawPolyVS, TEXT("/Plugin/RenderingUtilityLibrary/Private/RULDrawGeometryVSPS.usf"), TEXT("DrawPolyVS"), SF_Vertex);

class FRULShaderAutoLevelPS : public FRULBasePixelShader
{
    typedef FRULBasePixelShader FBaseType;

    RUL_DECLARE_SHADER_CONSTRUCTOR_DEFAULT_STATICS_WITH_TEXTURE(FRULShaderAutoLevelPS, Global, true)

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Texture,
        FShaderResourceParameter,
        FResourceId,
        "SourceTexture", SourceTexture
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Sampler,
        FShaderResourceParameter,
        FResourceId,
        "SourceTextureSampler", SourceTextureSampler
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        SRV,
        FShaderResourceParameter,
        FResourceId,
        "AutoLevelData", AutoLevelData
        )

    RUL_DECLARE_SHADER_PARAMETERS_0(UAV,,)
    RUL_DECLARE_SHADER_PARAMETERS_0(Value,,)
};

IMPLEMENT_SHADER_TYPE(, FRULShaderAutoLevelPS, TEXT("/Plugin/RenderingUtilityLibrary/Private/RULAutoLevelPS.usf"), TEXT("AutoLevelPS"), SF_Pixel);

class FRULShaderGetTextureValues : public FRULBaseComputeShader<256,1,1>
{
public:

    typedef FRULBaseComputeShader<256,1,1> FBaseType;

    DECLARE_SHADER_TYPE(FRULShaderGetTextureValues, Global);

public:

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return RHISupportsComputeShaders(Parameters.Platform);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FBaseType::ModifyCompilationEnvironment(Parameters, OutEnvironment);
    }

    RUL_DECLARE_SHADER_CONSTRUCTOR_SERIALIZER_WITH_TEXTURE(FRULShaderGetTextureValues)

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Texture,
        FShaderResourceParameter,
        FResourceId,
        "SourceTexture", SourceTexture
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        Sampler,
        FShaderResourceParameter,
        FResourceId,
        "SourceTextureSampler", SourceTextureSampler
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        SRV,
        FShaderResourceParameter,
        FResourceId,
        "PointData", PointData
        )

    RUL_DECLARE_SHADER_PARAMETERS_1(
        UAV,
        FShaderResourceParameter,
        FResourceId,
        "OutValueData", OutValueData
        )

    RUL_DECLARE_SHADER_PARAMETERS_2(
        Value,
        FShaderParameter,
        FParameterId,
        "_PointScale", Params_PointScale,
        "_PointCount", Params_PointCount
        )
};

IMPLEMENT_SHADER_TYPE(, FRULShaderGetTextureValues, TEXT("/Plugin/RenderingUtilityLibrary/Private/RULTextureValues.usf"), TEXT("GetTextureValuesByPoints"), SF_Compute);

class FRULFilterShaderVertexBuffer : public FVertexBuffer
{
public:

    virtual void InitRHI() override
    {
        // Create vertex data
        TResourceArray<FVector4, VERTEXBUFFER_ALIGNMENT> Vertices;
        Vertices.SetNumUninitialized(4);
        Vertices[0] = FVector4(-1.0f,  1.0f, 0.f, 1.f);
        Vertices[1] = FVector4( 1.0f,  1.0f, 1.f, 1.f);
        Vertices[2] = FVector4(-1.0f, -1.0f, 0.f, 0.f);
        Vertices[3] = FVector4( 1.0f, -1.0f, 1.f, 0.f);
        // Create a vertex buffer
        FRHIResourceCreateInfo CreateInfo(&Vertices);
        VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
    }
};

static TGlobalResource<FRULFilterShaderVertexBuffer> GRULFilterShaderVertexBuffer;

FVertexBufferRHIRef& URULShaderLibrary::GetFilterShaderVB()
{
    return GRULFilterShaderVertexBuffer.VertexBufferRHI;
}

ERenderTargetActions URULShaderLibrary::GetRenderTargetActions(FRULShaderDrawConfig DrawConfig)
{
    if (DrawConfig.bClearRenderTarget)
    {
        return ERenderTargetActions::Clear_Store;
    }
    else
    {
        return ERenderTargetActions::Load_Store;
    }
}

bool URULShaderLibrary::IsValidSwapTarget(UTextureRenderTarget2D* RTT0, UTextureRenderTarget2D* RTT1)
{
    return (
        IsValid(RTT0) &&
        IsValid(RTT1) &&
        RTT0->SizeX == RTT1->SizeX &&
        RTT0->SizeY == RTT1->SizeY &&
        RTT0->GetFormat() == RTT1->GetFormat()
        );
}

bool URULShaderLibrary::IsValidSwapTarget_RT(FTexture2DRHIParamRef Tex0, FTexture2DRHIParamRef Tex1)
{
    return (
        Tex0 &&
        Tex1 &&
        Tex0->GetSizeX() == Tex1->GetSizeX() &&
        Tex0->GetSizeY() == Tex1->GetSizeY() &&
        Tex0->GetFormat() == Tex1->GetFormat() &&
        (Tex0->GetFlags() & TexCreate_RenderTargetable) != 0 &&
        (Tex1->GetFlags() & TexCreate_RenderTargetable) != 0 &&
        Tex0->GetNumSamples() && Tex1->GetNumSamples()
        );
}

TSharedRef<FSceneView> URULShaderLibrary::CreateDefaultRTView(
    FRHICommandListImmediate& RHICmdList,
    FTextureRenderTarget2DResource* RTResource,
    FIntRect& ViewRect
    )
{
    // Render target view rectangle

	ViewRect = FIntRect(FIntPoint(0, 0), RTResource->GetSizeXY());

	// Create a new view family

	FSceneViewFamily* ViewFamily = new FSceneViewFamily(
        FSceneViewFamily::ConstructionValues(
            RTResource,
            nullptr,
            FEngineShowFlags(ESFIM_Game)
            )
            .SetWorldTimes(0.f, 0.f, 0.f)
            .SetGammaCorrection(RTResource->GetDisplayGamma())
        );

	// Create a new view

	FSceneViewInitOptions ViewInitOptions;
	ViewInitOptions.ViewFamily = ViewFamily;
	ViewInitOptions.SetViewRectangle(ViewRect);
	ViewInitOptions.ViewOrigin = FVector::ZeroVector;
	ViewInitOptions.ViewRotationMatrix = FMatrix::Identity;
	ViewInitOptions.ProjectionMatrix = FMatrix::Identity;
	ViewInitOptions.BackgroundColor = FLinearColor::Black;
	ViewInitOptions.OverlayColor = FLinearColor::White;
	FSceneView* View = new FSceneView(ViewInitOptions);

    // Create auto-delete view reference

    TSharedRef<FSceneView> ViewRef(View, [](FSceneView* ViewToDelete){ delete ViewToDelete->Family; });

    return MoveTemp(ViewRef);
}

void URULShaderLibrary::BindMaterialInstanceParameterCollection(
    FMaterialInstanceResource& MaterialInstanceResource,
    const FRULShaderMaterialParameterCollection& ParameterCollection
    )
{
    for (const FRULShaderScalarParameter& Parameter : ParameterCollection.ScalarParameters)
    {
        FMaterialParameterInfo ParameterInfo(Parameter.ParameterName);
        MaterialInstanceResource.RenderThread_UpdateParameter(ParameterInfo, Parameter.ParameterValue);
    }

    for (const FRULShaderVectorParameter& Parameter : ParameterCollection.VectorParameters)
    {
        FMaterialParameterInfo ParameterInfo(Parameter.ParameterName);
        MaterialInstanceResource.RenderThread_UpdateParameter(ParameterInfo, Parameter.ParameterValue);
    }

    for (const FRULShaderTextureParameter& Parameter : ParameterCollection.TextureParameters)
    {
        const UTexture* TextureValue(Parameter.ParameterValue);
        FMaterialParameterInfo ParameterInfo(Parameter.ParameterName);
        MaterialInstanceResource.RenderThread_UpdateParameter(ParameterInfo, TextureValue);
    }
}

void URULShaderLibrary::ResolveMaterialInstanceTextureParameter(
    FMaterialInstanceResource& MaterialInstanceResource,
    const FRULShaderMaterialParameterCollection& ParameterCollection,
    FName ResolveName,
    const UTexture* ResolveTexture
    )
{
    for (const FRULShaderNameResolveParameter& Parameter : ParameterCollection.NamedTextures)
    {
        if (Parameter.ParameterValue == ResolveName)
        {
            FMaterialParameterInfo ParameterInfo(Parameter.ParameterName);
            MaterialInstanceResource.RenderThread_UpdateParameter(ParameterInfo, ResolveTexture);
        }
    }
}

void URULShaderLibrary::AssignBlendState(FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERULShaderDrawBlendType BlendType)
{
    switch (BlendType)
    {
        case ERULShaderDrawBlendType::DB_Opaque:
            GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
            break;

        case ERULShaderDrawBlendType::DB_Max:
            GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Max, BF_SourceAlpha, BF_One>::GetRHI();
            break;

        case ERULShaderDrawBlendType::DB_Min:
            GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Min, BF_SourceAlpha, BF_One>::GetRHI();
            break;

        case ERULShaderDrawBlendType::DB_Add:
            GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_SourceAlpha, BF_One>::GetRHI();
            break;

        case ERULShaderDrawBlendType::DB_Sub:
            GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Subtract, BF_SourceAlpha, BF_One>::GetRHI();
            break;

        case ERULShaderDrawBlendType::DB_SubRev:
            GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_ReverseSubtract, BF_SourceAlpha, BF_One>::GetRHI();
            break;
    }
}

void URULShaderLibrary::AssignBlendState(FMeshPassProcessorRenderState& RenderState, ERULShaderDrawBlendType BlendType)
{
    switch (BlendType)
    {
        case ERULShaderDrawBlendType::DB_Opaque:
            RenderState.SetBlendState(TStaticBlendState<>::GetRHI());
            break;

        case ERULShaderDrawBlendType::DB_Max:
            RenderState.SetBlendState(TStaticBlendState<CW_RGB, BO_Max, BF_SourceAlpha, BF_One>::GetRHI());
            break;

        case ERULShaderDrawBlendType::DB_Min:
            RenderState.SetBlendState(TStaticBlendState<CW_RGB, BO_Min, BF_SourceAlpha, BF_One>::GetRHI());
            break;

        case ERULShaderDrawBlendType::DB_Add:
            RenderState.SetBlendState(TStaticBlendState<CW_RGB, BO_Add, BF_SourceAlpha, BF_One>::GetRHI());
            break;

        case ERULShaderDrawBlendType::DB_Sub:
            RenderState.SetBlendState(TStaticBlendState<CW_RGB, BO_Subtract, BF_SourceAlpha, BF_One>::GetRHI());
            break;

        case ERULShaderDrawBlendType::DB_SubRev:
            RenderState.SetBlendState(TStaticBlendState<CW_RGB, BO_ReverseSubtract, BF_SourceAlpha, BF_One>::GetRHI());
            break;
    }
}

void URULShaderLibrary::SetupMaterialParameters(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    FShader* VertexShader,
    FMaterialShader* MaterialShader,
    const FMaterialRenderProxy& MaterialRenderProxy,
    FSceneView& View
    )
{
    // Bind default material shader parameters

    if (MaterialShader)
    {
        MaterialShader->SetParameters(
            RHICmdList,
            MaterialShader->GetPixelShader(),
            &MaterialRenderProxy,
            *MaterialRenderProxy.GetMaterial(FeatureLevel),
            View,
            View.ViewUniformBuffer,
            ESceneTextureSetupMode::None
            );
    }

    // Bind identity primitive shader parameters
    // To prevent potential shader error (UE-18852 ElementalDemo crashes due to nil constant buffer)

    if (VertexShader)
    {
        auto& PrimitiveVS = VertexShader->GetUniformBufferParameter<FPrimitiveUniformShaderParameters>();
        // Track down usage of the (unsupported) Primitive uniform buffer
        check(! PrimitiveVS.IsBound());
        SetUniformBufferParameter(RHICmdList, VertexShader->GetVertexShader(), PrimitiveVS, GIdentityPrimitiveUniformBuffer);
    }

    if (MaterialShader)
    {
        auto& PrimitivePS = MaterialShader->GetUniformBufferParameter<FPrimitiveUniformShaderParameters>();
        // Track down usage of the (unsupported) Primitive uniform buffer
        check(! PrimitivePS.IsBound());
        SetUniformBufferParameter(RHICmdList, MaterialShader->GetPixelShader() , PrimitivePS, GIdentityPrimitiveUniformBuffer);
    }
}

void URULShaderLibrary::SetupDefaultGraphicsPSOInit(
    FGraphicsPipelineStateInitializer& GraphicsPSOInit,
    EPrimitiveType PrimitiveType,
    const FRULShaderDrawConfig& DrawConfig
    )
{
    AssignBlendState(GraphicsPSOInit, DrawConfig.BlendType);
    GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
    GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
    GraphicsPSOInit.PrimitiveType = PrimitiveType;
}

void URULShaderLibrary::SetupDefaultRenderState(
    FMeshPassProcessorRenderState& RenderState,
    const FRULShaderDrawConfig& DrawConfig
    )
{
    AssignBlendState(RenderState, DrawConfig.BlendType);
    RenderState.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());
}

void URULShaderLibrary::CopyToResolveTarget(
    UObject* WorldContextObject,
    UTexture* SourceTexture,
    UTextureRenderTarget2D* RenderTarget,
    UGWTTickEvent* CallbackEvent
    )
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::CopyToResolveTarget() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }
    else
    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::CopyToResolveTarget() ABORTED, INVALID WORLD SCENE"));
        return;
    }
    else
    if (! IsValid(SourceTexture))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::CopyToResolveTarget() ABORTED, INVALID SOURCE TEXTURE"));
        return;
    }
    else
    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::CopyToResolveTarget() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::CopyToResolveTarget() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        FTexture* SourceTextureResource;
        FTextureRenderTarget2DResource* RenderTargetResource;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        World->Scene->GetFeatureLevel(),
        SourceTexture->Resource,
        RenderTargetResource,
        CallbackEvent
        };

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_CopyToResolveTarget)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            check(IsInRenderingThread());
            if (RenderParameter.RenderTargetResource)
            {
                // Prepare source and resolve target textures
                FTextureRHIParamRef SourceTexture = RenderParameter.SourceTextureResource->TextureRHI;
                FTextureRHIParamRef TargetTexture = RenderParameter.RenderTargetResource->TextureRHI;

                // Copy to resolve target
                if (SourceTexture && TargetTexture)
                {
                    RHICmdList.CopyToResolveTarget(
                        SourceTexture,
                        TargetTexture,
                        FResolveParams()
                        );
                }
            }
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::DrawPoints(
    UObject* WorldContextObject,
    UTextureRenderTarget2D* RenderTarget,
    FRULShaderDrawConfig DrawConfig,
    FIntPoint DrawSize,
    const TArray<FVector2D>& Points,
    const TArray<int32>& Indices,
    UGWTTickEvent* CallbackEvent
    )
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawPoints() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }

    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawPoints() ABORTED, INVALID WORLD SCENE"));
        return;
    }

    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawPoints() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    if (Points.Num() < 3)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawPoints() ABORTED, VERTEX COUNT < 3"));
        return;
    }

    if (Indices.Num() < 3)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawPoints() ABORTED, INDEX COUNT < 3"));
        return;
    }

    if (DrawSize.X <= 0 || DrawSize.Y <= 0)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawPoints() ABORTED, INVALID DRAW SIZE"));
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawPoints() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        FTextureRenderTarget2DResource* RenderTargetResource;
        FRULShaderDrawConfig DrawConfig;
        FIntPoint DrawSize;
        TArray<FVector> Vertices;
        TArray<int32> Indices;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        World->Scene->GetFeatureLevel(),
        RenderTargetResource,
        DrawConfig,
        DrawSize,
        { },
        Indices,
        CallbackEvent
        };

    // Construct vertex data, fill Z-Component with 1.f

    const int32 VertexCount = Points.Num();
    TArray<FVector>& Vertices(RenderParameter.Vertices);
    Vertices.Reserve(VertexCount);

    for (int32 i=0; i<VertexCount; ++i)
    {
        Vertices.Emplace(Points[i], 1.f);
    }

    // Construct vertex data

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_DrawPoints)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::DrawGeometry_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.RenderTargetResource,
                RenderParameter.DrawConfig,
                RenderParameter.DrawSize,
                RenderParameter.Vertices,
                RenderParameter.Indices
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::DrawGeometry(
    UObject* WorldContextObject,
    UTextureRenderTarget2D* RenderTarget,
    FRULShaderDrawConfig DrawConfig,
    FIntPoint DrawSize,
    const TArray<FVector>& Vertices,
    const TArray<int32>& Indices,
    UGWTTickEvent* CallbackEvent
    )
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometry() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }

    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometry() ABORTED, INVALID WORLD SCENE"));
        return;
    }

    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometry() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    if (Vertices.Num() < 3)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometry() ABORTED, VERTEX COUNT < 3"));
        return;
    }

    if (Indices.Num() < 3)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometry() ABORTED, INDEX COUNT < 3"));
        return;
    }

    if (DrawSize.X <= 0 || DrawSize.Y <= 0)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometry() ABORTED, INVALID DRAW SIZE"));
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometry() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        FTextureRenderTarget2DResource* RenderTargetResource;
        FRULShaderDrawConfig DrawConfig;
        FIntPoint DrawSize;
        TArray<FVector> Vertices;
        TArray<int32> Indices;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        World->Scene->GetFeatureLevel(),
        RenderTargetResource,
        DrawConfig,
        DrawSize,
        Vertices,
        Indices,
        CallbackEvent
        };

    // Construct vertex data

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_DrawGeometry)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::DrawGeometry_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.RenderTargetResource,
                RenderParameter.DrawConfig,
                RenderParameter.DrawSize,
                RenderParameter.Vertices,
                RenderParameter.Indices
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::DrawGeometryColors(
    UObject* WorldContextObject,
    UTextureRenderTarget2D* RenderTarget,
    FRULShaderDrawConfig DrawConfig,
    FIntPoint DrawSize,
    const TArray<FVector>& Vertices,
    const TArray<FColor>& Colors,
    const TArray<int32>& Indices,
    UGWTTickEvent* CallbackEvent
    )
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometryColors() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }

    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometryColors() ABORTED, INVALID WORLD SCENE"));
        return;
    }

    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometryColors() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    if (Vertices.Num() < 3)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometryColors() ABORTED, VERTEX COUNT < 3"));
        return;
    }

    if (Indices.Num() < 3)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometryColors() ABORTED, INDEX COUNT < 3"));
        return;
    }

    if (DrawSize.X <= 0 || DrawSize.Y <= 0)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometryColors() ABORTED, INVALID DRAW SIZE"));
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawGeometryColors() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        FTextureRenderTarget2DResource* RenderTargetResource;
        FRULShaderDrawConfig DrawConfig;
        FIntPoint DrawSize;
        TArray<FVector> Vertices;
        TArray<FColor> Colors;
        TArray<int32> Indices;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        World->Scene->GetFeatureLevel(),
        RenderTargetResource,
        DrawConfig,
        DrawSize,
        Vertices,
        Colors,
        Indices,
        CallbackEvent
        };

    // Construct vertex data

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_DrawGeometryColors)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::DrawGeometry_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.RenderTargetResource,
                RenderParameter.DrawConfig,
                RenderParameter.DrawSize,
                RenderParameter.Vertices,
                RenderParameter.Indices,
                &RenderParameter.Colors
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::DrawGeometry_RT(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    FTextureRenderTarget2DResource* RenderTargetResource,
    FRULShaderDrawConfig DrawConfig,
    FIntPoint DrawSize,
    const TArray<FVector>& Vertices,
    const TArray<int32>& Indices,
    const TArray<FColor>* Colors
    )
{
    check(IsInRenderingThread());

    if (! RenderTargetResource)
    {
        return;
    }

    // Prepare render target texture and resolve target
    FTextureRHIParamRef TextureRTV = RenderTargetResource->GetRenderTargetTexture();
    FTextureRHIParamRef TextureRSV = RenderTargetResource->TextureRHI;

    if (! TextureRTV || ! TextureRSV)
    {
        return;
    }

    bool bUseColorBuffer = (Colors && Colors->Num() == Vertices.Num());

    // Prepare graphics pipelane

    FRULBaseVertexShader* VSShader;
    TShaderMapRef<FRULShaderDrawGeometryPS> PSShader(GetGlobalShaderMap(FeatureLevel));

    FGraphicsPipelineStateInitializer GraphicsPSOInit;
    SetupDefaultGraphicsPSOInit(GraphicsPSOInit, PT_TriangleList, DrawConfig);
    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PSShader);

    if (bUseColorBuffer)
    {
        VSShader = *TShaderMapRef<TRULShaderDrawGeometryVS<1>>(GetGlobalShaderMap(FeatureLevel));
        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GRULColorGeometryVertexDeclaration.VertexDeclarationRHI;
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VSShader->GetVertexShader();
    }
    else
    {
        VSShader = *TShaderMapRef<TRULShaderDrawGeometryVS<0>>(GetGlobalShaderMap(FeatureLevel));
        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector3();
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VSShader->GetVertexShader();
    }

    // Render pass

    FRHIRenderPassInfo RPInfo(TextureRTV, GetRenderTargetActions(DrawConfig), TextureRSV);
    TransitionRenderPassTargets(RHICmdList, RPInfo);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("RULShaderLibrary_DrawGeometry"));
    {
        // Set graphics pipeline

        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Bind shader parameters

        FBox2D DrawBounds(FVector2D(0, 0), DrawSize);
        FVector2D DrawScale(FVector2D::UnitVector/DrawBounds.GetExtent());
        FVector2D DrawOffset(-DrawBounds.GetCenter()*DrawScale);
        FVector4 DrawScaleBias(DrawScale.X, DrawScale.Y, DrawOffset.X, DrawOffset.Y);

        VSShader->SetParameter(RHICmdList, TEXT("_DrawScaleBias"), DrawScaleBias);

        // Draw primitives

        FRULRHIUtilityLibrary::DrawTriangleList(RHICmdList, Vertices, Indices, Colors);

        VSShader->UnbindBuffers(RHICmdList);
    }
    RHICmdList.EndRenderPass();
}

void URULShaderLibrary::DrawTexture(
    UObject* WorldContextObject,
    UTexture* SourceTexture,
    UTextureRenderTarget2D* RenderTarget,
    FRULShaderDrawConfig DrawConfig,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawTexture() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }
    else
    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawTexture() ABORTED, INVALID WORLD SCENE"));
        return;
    }
    else
    if (! IsValid(SourceTexture))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawTexture() ABORTED, INVALID TEXTURE"));
        return;
    }
    else
    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawTexture() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawTexture() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    World->SendAllEndOfFrameUpdates();

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        FTexture* SourceTexture;
        FTextureRenderTarget2DResource* RenderTargetResource;
        FRULShaderDrawConfig DrawConfig;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        World->Scene->GetFeatureLevel(),
        SourceTexture->Resource,
        RenderTargetResource,
        DrawConfig,
        CallbackEvent
        };

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_DrawTexture)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::DrawTexture_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.SourceTexture,
                RenderParameter.RenderTargetResource,
                RenderParameter.DrawConfig
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::DrawTexture_RT(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    FTexture* SourceTexture,
    FTextureRenderTarget2DResource* RenderTargetResource,
    FRULShaderDrawConfig DrawConfig
    )
{
    check(IsInRenderingThread());

    if (! RenderTargetResource || ! SourceTexture)
    {
        return;
    }

    // Prepare render target texture and resolve target
    FTextureRHIParamRef TextureRTV = RenderTargetResource->GetRenderTargetTexture();

    if (! TextureRTV)
    {
        return;
    }

    // Setup viewport
    FIntPoint SizeXY(RenderTargetResource->GetSizeXY());
	FIntRect ViewRect(FIntPoint(0, 0), SizeXY);
    RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

    // Safe check render pass
    GetRendererModule().InitializeSystemTextures(RHICmdList);
    check(RHICmdList.IsOutsideRenderPass());

    // Render pass with the specified render target
    FRHIRenderPassInfo RPInfo(TextureRTV, ERenderTargetActions::Load_Store);
    TransitionRenderPassTargets(RHICmdList, RPInfo);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("RULShaderLibrary_DrawTexture"));
    {
		float Gamma = 1.0f / RenderTargetResource->GetDisplayGamma();
		bool bNeedsToSwitchVerticalAxis = !RHINeedsToSwitchVerticalAxis(GShaderPlatformForFeatureLevel[FeatureLevel]);

        // Prepare view
        FSceneView SceneView(FBatchedElements::CreateProxySceneView(FMatrix::Identity, ViewRect));
        FMeshPassProcessorRenderState DrawRenderState(SceneView);

        // Set blend state and disable depth test & writes
        SetupDefaultRenderState(DrawRenderState, DrawConfig);

        // Construct and draw batched elements

        FBatchedElements BatchedElements;

        {
            float L = -1.f;
            float T = -1.f;
            float R =  1.f;
            float B =  1.f;
            FVector2D UV0(0.f, 0.f);
            FVector2D UV1(1.f, 1.f);
            int32 V00 = BatchedElements.AddVertex(
                FVector4(L, T, 0.f, 1.f),
                FVector2D(UV0.X, UV0.Y),
                FLinearColor::White,
                FHitProxyId::InvisibleHitProxyId );
            int32 V10 = BatchedElements.AddVertex(
                FVector4(R, T, 0.f, 1.f),
                FVector2D(UV1.X, UV0.Y),
                FLinearColor::White,
                FHitProxyId::InvisibleHitProxyId );
            int32 V01 = BatchedElements.AddVertex(
                FVector4(L, B, 0.f, 1.f),
                FVector2D(UV0.X, UV1.Y),		
                FLinearColor::White,
                FHitProxyId::InvisibleHitProxyId );
            int32 V11 = BatchedElements.AddVertex(
                FVector4(R,	B, 0.f, 1.f),
                FVector2D(UV1.X, UV1.Y),
                FLinearColor::White,
                FHitProxyId::InvisibleHitProxyId );

            BatchedElements.AddTriangle(V00, V10, V11, SourceTexture, SE_BLEND_Opaque);
            BatchedElements.AddTriangle(V00, V11, V01, SourceTexture, SE_BLEND_Opaque);
        }

        BatchedElements.Draw(
            RHICmdList,
            DrawRenderState,
            FeatureLevel,
            bNeedsToSwitchVerticalAxis,
            SceneView,
            false,
            Gamma
            );
    }
    RHICmdList.EndRenderPass();
}

void URULShaderLibrary::ApplyMaterial(
    UObject* WorldContextObject,
    UMaterialInterface* Material,
    UTextureRenderTarget2D* RenderTarget,
    FRULShaderDrawConfig DrawConfig,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterial() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }

    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterial() ABORTED, INVALID WORLD SCENE"));
        return;
    }

    if (! IsValid(Material))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterial() ABORTED, INVALID MATERIAL"));
        return;
    }

    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterial() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterial() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    World->SendAllEndOfFrameUpdates();

    ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
    const FMaterialRenderProxy* MaterialRenderProxy = Material->GetRenderProxy();

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        FTextureRenderTarget2DResource* RenderTargetResource;
        const FMaterialRenderProxy* MaterialRenderProxy;
        FRULShaderDrawConfig DrawConfig;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        FeatureLevel,
        RenderTargetResource,
        MaterialRenderProxy,
        DrawConfig,
        CallbackEvent
        };

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_ApplyMaterial)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::ApplyMaterial_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.RenderTargetResource,
                RenderParameter.MaterialRenderProxy,
                RenderParameter.DrawConfig
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::ApplyMaterial_RT(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    FTextureRenderTarget2DResource* RenderTargetResource,
    const FMaterialRenderProxy* MaterialRenderProxy,
    FRULShaderDrawConfig DrawConfig
    )
{
    check(IsInRenderingThread());

    const FMaterial* MaterialResource = MaterialRenderProxy->GetMaterial(FeatureLevel);

    if (! RenderTargetResource || ! MaterialRenderProxy || ! MaterialResource)
    {
        return;
    }

    // Prepare render target texture and resolve target
    FTextureRHIParamRef TextureRTV = RenderTargetResource->GetRenderTargetTexture();
    FTextureRHIParamRef TextureRSV = RenderTargetResource->TextureRHI;

    if (! TextureRTV || ! TextureRSV)
    {
        return;
    }

    // Update deferred expression cache

    MaterialRenderProxy->UpdateUniformExpressionCacheIfNeeded(FeatureLevel);
    FMaterialRenderProxy::UpdateDeferredCachedUniformExpressions();

	// Create default render target view

	FIntRect ViewRect;
    TSharedRef<FSceneView> View(CreateDefaultRTView(RHICmdList, RenderTargetResource, ViewRect));

    // Setup viewport

    RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

    // Prepare graphics pipelane

    TShaderMapRef<FRULShaderDrawScreenVS> VSShader(GetGlobalShaderMap(FeatureLevel));

	const FMaterialShaderMap* MaterialShaderMap = MaterialResource->GetRenderingThreadShaderMap();
	FRULBaseMaterialShader* PSShader = MaterialShaderMap->GetShader<TRULShaderDrawScreenMS<0>>();

    FGraphicsPipelineStateInitializer GraphicsPSOInit;
    SetupDefaultGraphicsPSOInit(GraphicsPSOInit, PT_TriangleStrip, DrawConfig);
    GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSShader);
    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PSShader->GetPixelShader();

    // Render pass

    FRHIRenderPassInfo RPInfo(TextureRTV, GetRenderTargetActions(DrawConfig));
    TransitionRenderPassTargets(RHICmdList, RPInfo);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("RULShaderLibrary_ApplyMaterial"));
    {
        // Set graphics pipeline

        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Bind shader parameters

        SetupMaterialParameters(RHICmdList, FeatureLevel, *VSShader, PSShader, *MaterialRenderProxy, *View);

        // Draw primitives

        RHICmdList.SetStreamSource(0, GetFilterShaderVB(), 0);
        RHICmdList.DrawPrimitive(0, 2, 1);

        // Unbind shader parameters

        VSShader->UnbindBuffers(RHICmdList);
        PSShader->UnbindBuffers(RHICmdList);
    }
    RHICmdList.EndRenderPass();
}

void URULShaderLibrary::ApplyMaterialFilter(
    UObject* WorldContextObject,
    UMaterialInterface* Material,
    int32 RepeatCount,
    FRULShaderDrawConfig DrawConfig,
    FRULShaderTextureParameterInput SourceTexture,
    UTextureRenderTarget2D* RenderTarget,
    UTextureRenderTarget2D* SwapTarget,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterialFilter() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }

    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterialFilter() ABORTED, INVALID WORLD SCENE"));
        return;
    }

    if (! IsValid(Material))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterialFilter() ABORTED, INVALID MATERIAL"));
        return;
    }

    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterialFilter() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterialFilter() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    // Get swap render target resource if present

    FTextureRenderTarget2DResource* SwapTargetResource = nullptr;

    if (IsValid(SwapTarget))
    {
        if (IsValidSwapTarget(RenderTarget, SwapTarget))
        {
            SwapTargetResource = static_cast<FTextureRenderTarget2DResource*>(SwapTarget->GameThread_GetRenderTargetResource());
        }
        else
        {
            UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMaterialFilter() INVALID SWAP RENDER TARGET DIMENSION / FORMAT"));
        }
    }

    World->SendAllEndOfFrameUpdates();

    ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
    const FMaterialRenderProxy* MaterialRenderProxy = Material->GetRenderProxy();

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        int32 RepeatCount;
        FRULShaderDrawConfig DrawConfig;
        FRULShaderTextureParameterInputResource SourceTextureResource;
        FTextureRenderTarget2DResource* RenderTargetResource;
        FTextureRenderTarget2DResource* SwapTargetResource;
        const FMaterialRenderProxy* MaterialRenderProxy;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        FeatureLevel,
        RepeatCount,
        DrawConfig,
        SourceTexture.GetResource_GT(),
        RenderTargetResource,
        SwapTargetResource,
        MaterialRenderProxy,
        CallbackEvent
        };

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_ApplyMaterialFilter)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::ApplyMaterialFilter_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.RepeatCount,
                RenderParameter.DrawConfig,
                RenderParameter.SourceTextureResource,
                RenderParameter.RenderTargetResource,
                RenderParameter.SwapTargetResource,
                RenderParameter.MaterialRenderProxy
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::ApplyMaterialFilter_RT(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    int32 RepeatCount,
    FRULShaderDrawConfig DrawConfig,
    FRULShaderTextureParameterInputResource SourceTextureResource,
    FTextureRenderTarget2DResource* RenderTargetResource,
    FTextureRenderTarget2DResource* SwapTargetResource,
    const FMaterialRenderProxy* MaterialRenderProxy
    )
{
    check(IsInRenderingThread());

    const FMaterial* MaterialResource = MaterialRenderProxy->GetMaterial(FeatureLevel);

    if (! RenderTargetResource || ! MaterialRenderProxy || ! MaterialResource)
    {
        return;
    }

    FTexture2DRHIRef SwapTextureRTV;
    FTexture2DRHIRef SwapTextureRSV;
    FTexture2DRHIRef SourceTexture = SourceTextureResource.GetTextureParamRef_RT();
    FTexture2DRHIRef TargetTexture = RenderTargetResource->GetRenderTargetTexture();

    if (! TargetTexture.IsValid())
    {
        return;
    }

    // Update deferred expression cache

    MaterialRenderProxy->UpdateUniformExpressionCacheIfNeeded(FeatureLevel);
    FMaterialRenderProxy::UpdateDeferredCachedUniformExpressions();

    const bool bIsMultiPass = RepeatCount > 0;

    // Prepare swap texture for multi-pass filter
    if (bIsMultiPass)
    {
        // Get render resource texture if swap texture is supplied
        if (SwapTargetResource)
        {
            if (IsValidSwapTarget_RT(TargetTexture, SwapTargetResource->GetRenderTargetTexture()))
            {
                SwapTextureRTV = SwapTargetResource->GetRenderTargetTexture();
            }
            else
            {
                UE_LOG(LogRUL,Warning, TEXT("SUPPLIED SWAP RENDER TARGET IS NOT SWAP CAPABLE (RT)"));
                FDebug::DumpStackTraceToLog();
            }
        }

        // Create new swap texture if no swap texture is supplied
        if (! SwapTextureRTV.IsValid())
        {
            const uint32 FlagsMask = ~(TexCreate_RenderTargetable | TexCreate_ResolveTargetable | TexCreate_ShaderResource);
            FRHIResourceCreateInfo CreateInfo(TargetTexture->GetClearBinding());
            RHICreateTargetableShaderResource2D(
                TargetTexture->GetSizeX(),
                TargetTexture->GetSizeY(),
                TargetTexture->GetFormat(),
                1,
                TargetTexture->GetFlags() & FlagsMask,
                TexCreate_RenderTargetable,
                false,
                CreateInfo,
                SwapTextureRTV,
                SwapTextureRSV,
                TargetTexture->GetNumSamples()
                );
        }
    }

    // Prepare render target texture and resolve target
    FTextureRHIParamRef TextureRTV = TargetTexture;
    FTextureRHIParamRef TextureRSV = RenderTargetResource->TextureRHI;
    FSamplerStateRHIParamRef TextureSampler = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();

	// Create default render target view

	FIntRect ViewRect;
    TSharedRef<FSceneView> View(CreateDefaultRTView(RHICmdList, RenderTargetResource, ViewRect));

    // Setup viewport

    RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

    // Prepare graphics pipelane

    TShaderMapRef<FRULShaderDrawScreenVS> VSShader(GetGlobalShaderMap(FeatureLevel));

	const FMaterialShaderMap* MaterialShaderMap = MaterialResource->GetRenderingThreadShaderMap();
	FRULBaseMaterialShader* PSShader = MaterialShaderMap->GetShader<TRULShaderDrawScreenMS<0>>();

    FGraphicsPipelineStateInitializer GraphicsPSOInit;
    SetupDefaultGraphicsPSOInit(GraphicsPSOInit, PT_TriangleStrip, DrawConfig);
    GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSShader);
    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PSShader->GetPixelShader();

    // Render pass

    FRHIRenderPassInfo RPInfo(TextureRTV, GetRenderTargetActions(DrawConfig));
    TransitionRenderPassTargets(RHICmdList, RPInfo);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("RULShaderLibrary_ApplyMaterialFilter"));
    {
        // Set graphics pipeline

        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Bind shader parameters

        SetupMaterialParameters(RHICmdList, FeatureLevel, *VSShader, PSShader, *MaterialRenderProxy, *View);
        PSShader->BindTexture(RHICmdList, TEXT("SourceMap"), TEXT("SourceMapSampler"), SourceTexture, TextureSampler);

        // Draw primitives

        RHICmdList.SetStreamSource(0, GetFilterShaderVB(), 0);
        RHICmdList.DrawPrimitive(0, 2, 1);

        // Unbind shader parameters
        PSShader->UnbindBuffers(RHICmdList);
    }
    RHICmdList.EndRenderPass();

    // Draw multi-pass if required
    if (bIsMultiPass)
    {
        FTextureRHIParamRef TextureRTV0 = TextureRTV;
        FTextureRHIParamRef TextureRTV1 = SwapTextureRTV;

        check(TextureRTV0 != nullptr);
        check(TextureRTV1 != nullptr);

        for (int32 i=0; i<RepeatCount; ++i)
        {
            // Render pass
            FRHIRenderPassInfo SwapRPInfo(TextureRTV1, ERenderTargetActions::Load_Store);
            // Apply render pass target transition only on the first iteration
            if (i == 1)
            {
                TransitionRenderPassTargets(RHICmdList, RPInfo);
            }
            RHICmdList.BeginRenderPass(SwapRPInfo, TEXT("RULShaderLibrary_ApplyMaterialFilter"));
            {
                // Bind swap texture
                PSShader->BindTexture(RHICmdList, TEXT("SourceMap"), TEXT("SourceMapSampler"), TextureRTV0, TextureSampler);
                // Draw primitives
                RHICmdList.DrawPrimitive(0, 2, 1);
                // Unbind shader parameters
                PSShader->UnbindBuffers(RHICmdList);
            }
            RHICmdList.EndRenderPass();

            // Swap RTTs
            Swap(TextureRTV0, TextureRTV1);
        }

        // Unbind vertex shader parameters
        VSShader->UnbindBuffers(RHICmdList);

        // Make sure TextureRTV has the last drawn render target
        if (TextureRTV0 != TextureRTV)
        {
            RHICmdList.CopyToResolveTarget(
                TextureRTV0,
                TextureRSV,
                FResolveParams()
                );
        }
    }
}

void URULShaderLibrary::ApplyMultiParametersMaterial(
    UObject* WorldContextObject,
    UMaterialInstanceDynamic* Material,
    const TArray<FRULShaderMaterialParameterCollection>& ParameterCollections,
    FRULShaderDrawConfig DrawConfig,
    UTextureRenderTarget2D* RenderTarget,
    UTextureRenderTarget2D* SwapTarget,
    int32 ParameterCollectionStartIndex,
    int32 ParameterCollectionRepeatCount,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    // Empty parameter collection, abort without warning
    if (ParameterCollections.Num() < 1)
    {
        return;
    }

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMultiParametersMaterial() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }

    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMultiParametersMaterial() ABORTED, INVALID WORLD SCENE"));
        return;
    }

    if (! IsValid(Material))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMultiParametersMaterial() ABORTED, INVALID MATERIAL"));
        return;
    }

    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMultiParametersMaterial() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMultiParametersMaterial() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    // Get swap render target resource if present

    FTextureRenderTarget2DResource* SwapTargetResource = nullptr;

    if (IsValid(SwapTarget))
    {
        if (IsValidSwapTarget(RenderTarget, SwapTarget))
        {
            SwapTargetResource = static_cast<FTextureRenderTarget2DResource*>(SwapTarget->GameThread_GetRenderTargetResource());
        }
        else
        {
            UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyMultiParametersMaterial() INVALID SWAP RENDER TARGET DIMENSION / FORMAT"));
        }
    }

    World->SendAllEndOfFrameUpdates();

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        FRULShaderDrawConfig DrawConfig;
        FTextureRenderTarget2DResource* RenderTargetResource;
        FTextureRenderTarget2DResource* SwapTargetResource;
        FMaterialInstanceResource* MaterialInstanceResource;
        TArray<FRULShaderMaterialParameterCollection> ParameterCollections;
        TArray<UTexture*> ResolveTextures;
        int32 ParameterCollectionStartIndex;
        int32 ParameterCollectionRepeatCount;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        World->Scene->GetFeatureLevel(),
        DrawConfig,
        RenderTargetResource,
        SwapTargetResource,
        Material->Resource,
        ParameterCollections,
        { RenderTarget, SwapTarget },
        ParameterCollectionStartIndex,
        FMath::Max(0, ParameterCollectionRepeatCount),
        CallbackEvent
        };

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_ApplyMultiParametersMaterial)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::ApplyMultiParametersMaterial_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.DrawConfig,
                RenderParameter.RenderTargetResource,
                RenderParameter.SwapTargetResource,
                RenderParameter.MaterialInstanceResource,
                RenderParameter.ParameterCollections,
                RenderParameter.ResolveTextures,
                RenderParameter.ParameterCollectionStartIndex,
                RenderParameter.ParameterCollectionRepeatCount
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::ApplyMultiParametersMaterial_RT(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    FRULShaderDrawConfig DrawConfig,
    FTextureRenderTarget2DResource* RenderTargetResource,
    FTextureRenderTarget2DResource* SwapTargetResource,
    FMaterialInstanceResource* MIResource,
    const TArray<FRULShaderMaterialParameterCollection>& ParameterCollections,
    const TArray<UTexture*>& ResolveTextures,
    int32 ParameterCollectionStartIndex,
    int32 ParameterCollectionRepeatCount
    )
{
    check(IsInRenderingThread());
    check(MIResource != nullptr);
    check(ParameterCollectionStartIndex > 0 || ParameterCollections.Num() > 0);

    const FMaterial* MaterialResource = MIResource->GetMaterial(FeatureLevel);

    if (! RenderTargetResource || ! MaterialResource)
    {
        return;
    }

    FTexture2DRHIRef SwapTextureRTV;
    FTexture2DRHIRef SwapTextureRSV;
    FTexture2DRHIRef TargetTexture = RenderTargetResource->GetRenderTargetTexture();

    if (! TargetTexture.IsValid())
    {
        return;
    }

    const int32 ParameterCollectionCount = ParameterCollections.Num();
    const int32 ParameterCollectionDrawCount = ParameterCollectionCount + (ParameterCollectionCount * ParameterCollectionRepeatCount);
    const int32 TotalDrawCount = ParameterCollectionDrawCount + ParameterCollectionStartIndex;
    const bool bIsMultiPass = TotalDrawCount > 1;

    // Prepare swap texture for multi-pass filter
    if (bIsMultiPass)
    {
        // Get render resource texture if swap texture is supplied
        if (SwapTargetResource)
        {
            if (IsValidSwapTarget_RT(TargetTexture, SwapTargetResource->GetRenderTargetTexture()))
            {
                SwapTextureRTV = SwapTargetResource->GetRenderTargetTexture();
            }
            else
            {
                UE_LOG(LogRUL,Warning, TEXT("SUPPLIED SWAP RENDER TARGET IS NOT SWAP CAPABLE (RT)"));
                FDebug::DumpStackTraceToLog();
            }
        }

        // Create new swap texture if no swap texture is supplied
        if (! SwapTextureRTV.IsValid())
        {
            const uint32 FlagsMask = ~(TexCreate_RenderTargetable | TexCreate_ResolveTargetable | TexCreate_ShaderResource);
            FRHIResourceCreateInfo CreateInfo(TargetTexture->GetClearBinding());
            RHICreateTargetableShaderResource2D(
                TargetTexture->GetSizeX(),
                TargetTexture->GetSizeY(),
                TargetTexture->GetFormat(),
                1,
                TargetTexture->GetFlags() & FlagsMask,
                TexCreate_RenderTargetable,
                false,
                CreateInfo,
                SwapTextureRTV,
                SwapTextureRSV,
                TargetTexture->GetNumSamples()
                );
        }
    }

    // Prepare render target texture and resolve target
    FTextureRHIParamRef TextureRTV = TargetTexture;
    FTextureRHIParamRef TextureRSV = RenderTargetResource->TextureRHI;
    FSamplerStateRHIParamRef TextureSampler = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();

	// Create default render target view

	FIntRect ViewRect;
    TSharedRef<FSceneView> View(CreateDefaultRTView(RHICmdList, RenderTargetResource, ViewRect));

    // Setup viewport

    RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

    // Prepare graphics pipelane

    TShaderMapRef<FRULShaderDrawScreenVS> VSShader(GetGlobalShaderMap(FeatureLevel));

	const FMaterialShaderMap* MaterialShaderMap = MaterialResource->GetRenderingThreadShaderMap();
	FRULBaseMaterialShader* PSShader = MaterialShaderMap->GetShader<TRULShaderDrawScreenMS<0>>();

    FGraphicsPipelineStateInitializer GraphicsPSOInit;
    SetupDefaultGraphicsPSOInit(GraphicsPSOInit, PT_TriangleStrip, DrawConfig);
    GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSShader);
    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PSShader->GetPixelShader();

    // Render pass

    FRHIRenderPassInfo RPInfo(TextureRTV, GetRenderTargetActions(DrawConfig));
    TransitionRenderPassTargets(RHICmdList, RPInfo);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("RULShaderLibrary_ApplyMultiParametersMaterial"));
    {
        // Set graphics pipeline

        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Bind shader parameters

        if (ParameterCollectionStartIndex <= 0)
        {
            BindMaterialInstanceParameterCollection(*MIResource, ParameterCollections[0]);
            MIResource->CacheUniformExpressions(false);
        }

        SetupMaterialParameters(RHICmdList, FeatureLevel, *VSShader, PSShader, *MIResource, *View);

        // Draw primitives

        RHICmdList.SetStreamSource(0, GetFilterShaderVB(), 0);
        RHICmdList.DrawPrimitive(0, 2, 1);

        // Unbind shader parameters
        PSShader->UnbindBuffers(RHICmdList);
    }
    RHICmdList.EndRenderPass();

    // Draw multi-pass if required
    if (bIsMultiPass)
    {
        FTextureRHIParamRef TextureRTV0 = TextureRTV;
        FTextureRHIParamRef TextureRTV1 = SwapTextureRTV;

        check(TextureRTV0 != nullptr);
        check(TextureRTV1 != nullptr);

        for (int32 i=1; i<TotalDrawCount; ++i)
        {
            // Render pass
            FRHIRenderPassInfo SwapRPInfo(TextureRTV1, ERenderTargetActions::Load_Store);
            // Apply render pass target transition only on the first iteration
            if (i == 1)
            {
                TransitionRenderPassTargets(RHICmdList, RPInfo);
            }
            RHICmdList.BeginRenderPass(SwapRPInfo, TEXT("RULShaderLibrary_ApplyMultiParametersMaterial"));
            {
                // Bind shader parameters

                if (i >= ParameterCollectionStartIndex)
                {
                    // Bind parameters
                    int32 ParamId = (i - ParameterCollectionStartIndex) % ParameterCollectionCount;
                    const FRULShaderMaterialParameterCollection& Parameters(ParameterCollections[ParamId]);
                    BindMaterialInstanceParameterCollection(*MIResource, Parameters);

                    // Resolve named textures
                    int32 ResolveTextureId = (TextureRTV0 == TargetTexture) ? 0 : 1;
                    UTexture* ResolveTexture = ResolveTextures[ResolveTextureId];
                    ResolveMaterialInstanceTextureParameter(*MIResource, Parameters, TEXT("__SWAP_TEXTURE__"), ResolveTexture);

                    MIResource->CacheUniformExpressions(false);
                    SetupMaterialParameters(RHICmdList, FeatureLevel, *VSShader, PSShader, *MIResource, *View);
                }

                // Draw primitives
                RHICmdList.DrawPrimitive(0, 2, 1);

                // Unbind shader parameters
                PSShader->UnbindBuffers(RHICmdList);
            }
            RHICmdList.EndRenderPass();

            // Swap RTTs
            Swap(TextureRTV0, TextureRTV1);
        }

        // Unbind vertex shader parameters
        VSShader->UnbindBuffers(RHICmdList);

        // Copy final drawn texture to resolve target
        RHICmdList.CopyToResolveTarget(
            TextureRTV0,
            TextureRSV,
            FResolveParams()
            );
    }
    else
    {
        // Copy to resolve target
        RHICmdList.CopyToResolveTarget(
            TextureRTV,
            TextureRSV,
            FResolveParams()
            );
    }
}

void URULShaderLibrary::DrawMaterialQuad(
    UObject* WorldContextObject,
    const TArray<FGULQuadGeometryInstance>& Quads,
    UMaterialInterface* Material,
    UTextureRenderTarget2D* RenderTarget,
    FRULShaderDrawConfig DrawConfig,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawMaterialQuad() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }

    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawMaterialQuad() ABORTED, INVALID WORLD SCENE"));
        return;
    }

    if (! IsValid(Material))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawMaterialQuad() ABORTED, INVALID MATERIAL"));
        return;
    }

    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawMaterialQuad() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    if (Quads.Num() < 1)
    {
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawMaterialQuad() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    World->SendAllEndOfFrameUpdates();

    ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
    const FMaterialRenderProxy* MaterialRenderProxy = Material->GetRenderProxy();

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        const TArray<FGULQuadGeometryInstance> Quads;
        FTextureRenderTarget2DResource* RenderTargetResource;
        const FMaterialRenderProxy* MaterialRenderProxy;
        FRULShaderDrawConfig DrawConfig;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        FeatureLevel,
        Quads,
        RenderTargetResource,
        MaterialRenderProxy,
        DrawConfig,
        CallbackEvent
        };

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_DrawMaterialQuad)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::DrawMaterialQuad_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.Quads,
                RenderParameter.RenderTargetResource,
                RenderParameter.MaterialRenderProxy,
                RenderParameter.DrawConfig
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::DrawMaterialQuad_RT(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    const TArray<FGULQuadGeometryInstance>& Quads,
    FTextureRenderTarget2DResource* RenderTargetResource,
    const FMaterialRenderProxy* MaterialRenderProxy,
    FRULShaderDrawConfig DrawConfig
    )
{
    check(IsInRenderingThread());

    const FMaterial* MaterialResource = MaterialRenderProxy->GetMaterial(FeatureLevel);

    if (! RenderTargetResource || ! MaterialRenderProxy || ! MaterialResource)
    {
        return;
    }

    // Prepare render target texture and resolve target
    FTextureRHIParamRef TextureRTV = RenderTargetResource->GetRenderTargetTexture();
    FTextureRHIParamRef TextureRSV = RenderTargetResource->TextureRHI;

    if (! TextureRTV || ! TextureRSV)
    {
        return;
    }

    // Update deferred expression cache

    MaterialRenderProxy->UpdateUniformExpressionCacheIfNeeded(FeatureLevel);
    FMaterialRenderProxy::UpdateDeferredCachedUniformExpressions();

    // Create quad data SRV

    const int32 QuadCount = Quads.Num();

    TResourceArray<FVector4, VERTEXBUFFER_ALIGNMENT> QuadGeomArr;
    TResourceArray<FVector4, VERTEXBUFFER_ALIGNMENT> QuadTransformArr;

    QuadGeomArr.Reserve(QuadCount);
    QuadTransformArr.Reserve(QuadCount);

    for (const FGULQuadGeometryInstance& Quad : Quads)
    {
        const FVector2D& Origin(Quad.Origin);
        const FVector2D& Size(Quad.Size);

        QuadGeomArr.Emplace(Origin.X, Origin.Y, Size.X, Size.Y);
        QuadTransformArr.Emplace(Quad.Scale, Quad.Angle, Quad.Value, 0.f);
    }
    
    FRULReadBuffer QuadGeomData;
    FRULReadBuffer QuadTransformData;

    QuadGeomData.Initialize(
        QuadGeomArr.GetTypeSize(),
        QuadCount,
        PF_A32B32G32R32F,
        &QuadGeomArr,
        BUF_Static,
        TEXT("QuadGeomData")
        );

    QuadTransformData.Initialize(
        QuadTransformArr.GetTypeSize(),
        QuadCount,
        PF_A32B32G32R32F,
        &QuadTransformArr,
        BUF_Static,
        TEXT("QuadTransformData")
        );

	// Create default render target view

	FIntRect ViewRect;
    TSharedRef<FSceneView> View(CreateDefaultRTView(RHICmdList, RenderTargetResource, ViewRect));

    // Setup viewport

    RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

    // Prepare graphics pipelane

    TShaderMapRef<FRULShaderDrawQuadVS> VSShader(GetGlobalShaderMap(FeatureLevel));

	const FMaterialShaderMap* MaterialShaderMap = MaterialResource->GetRenderingThreadShaderMap();
	FRULBaseMaterialShader* PSShader = MaterialShaderMap->GetShader<TRULShaderDrawScreenMS<1>>();

    FGraphicsPipelineStateInitializer GraphicsPSOInit;
    SetupDefaultGraphicsPSOInit(GraphicsPSOInit, PT_TriangleStrip, DrawConfig);
    GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSShader);
    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PSShader->GetPixelShader();

    // Render pass

    FRHIRenderPassInfo RPInfo(TextureRTV, GetRenderTargetActions(DrawConfig));
    TransitionRenderPassTargets(RHICmdList, RPInfo);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("RULShaderLibrary_DrawMaterialQuad"));
    {
        // Set graphics pipeline

        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Bind shader parameters

        SetupMaterialParameters(RHICmdList, FeatureLevel, *VSShader, PSShader, *MaterialRenderProxy, *View);

        VSShader->BindSRV(RHICmdList, TEXT("QuadGeomData"), QuadGeomData.SRV);
        VSShader->BindSRV(RHICmdList, TEXT("QuadTransformData"), QuadTransformData.SRV);

        // Draw primitives

        RHICmdList.SetStreamSource(0, GetFilterShaderVB(), 0);
        RHICmdList.DrawPrimitive(0, 2, QuadCount);

        // Unbind shader parameters

        VSShader->UnbindBuffers(RHICmdList);
        PSShader->UnbindBuffers(RHICmdList);
    }
    RHICmdList.EndRenderPass();
}

void URULShaderLibrary::DrawMaterialPoly(
    UObject* WorldContextObject,
    const TArray<FGULPolyGeometryInstance>& Polys,
    UMaterialInterface* Material,
    UTextureRenderTarget2D* RenderTarget,
    FRULShaderDrawConfig DrawConfig,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawMaterialPoly() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }

    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawMaterialPoly() ABORTED, INVALID WORLD SCENE"));
        return;
    }

    if (! IsValid(Material))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawMaterialPoly() ABORTED, INVALID MATERIAL"));
        return;
    }

    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawMaterialPoly() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    if (Polys.Num() < 1)
    {
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::DrawMaterialPoly() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    World->SendAllEndOfFrameUpdates();

    ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
    const FMaterialRenderProxy* MaterialRenderProxy = Material->GetRenderProxy();

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        TArray<FVector4> Vertices;
        TArray<int32> Indices;
        FTextureRenderTarget2DResource* RenderTargetResource;
        const FMaterialRenderProxy* MaterialRenderProxy;
        FRULShaderDrawConfig DrawConfig;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        FeatureLevel,
        {},
        {},
        RenderTargetResource,
        MaterialRenderProxy,
        DrawConfig,
        CallbackEvent
        };

    for (const FGULPolyGeometryInstance& Poly : Polys)
    {
        const int32 Sides = FMath::Max(3, Poly.Sides);

        TArray<FVector4>& Vertices(RenderParameter.Vertices);
        TArray<int32>& Indices(RenderParameter.Indices);

        const FVector2D& Origin(Poly.Origin);
        const FVector2D& Size(Poly.Size);
        const float Value = Poly.Value;
        const float Scale = Poly.Scale;
        const float UnitAngle = 2*PI / Sides;
        int32 OriginIndex = Vertices.Num();
        int32 IndexOffset = OriginIndex+1;

        Vertices.Emplace(Origin.X, Origin.Y, Value, 1.f);

        for (int32 i=0; i<Sides; ++i)
        {
            float ExtentS, ExtentC;
            float AngleDeg = FMath::RadiansToDegrees(Poly.Angle);
            FMath::SinCos(&ExtentS, &ExtentC, i*UnitAngle);

            FVector2D Extent(ExtentC, ExtentS);
            FVector2D Vertex(Origin + (Extent * Size * Scale).GetRotated(AngleDeg));

            Vertices.Emplace(Vertex.X, Vertex.Y, Value, 0.f);

            Indices.Emplace(OriginIndex  );
            Indices.Emplace(IndexOffset+i);
            Indices.Emplace(IndexOffset+(i+1)%Sides);
        }
    }

    ENQUEUE_RENDER_COMMAND(RULUtilityShaderLibrary_DrawMaterialPoly)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::DrawMaterialPoly_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.Vertices,
                RenderParameter.Indices,
                RenderParameter.RenderTargetResource,
                RenderParameter.MaterialRenderProxy,
                RenderParameter.DrawConfig
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::DrawMaterialPoly_RT(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    const TArray<FVector4>& Vertices,
    const TArray<int32>& Indices,
    FTextureRenderTarget2DResource* RenderTargetResource,
    const FMaterialRenderProxy* MaterialRenderProxy,
    FRULShaderDrawConfig DrawConfig
    )
{
    check(IsInRenderingThread());

    const FMaterial* MaterialResource = MaterialRenderProxy->GetMaterial(FeatureLevel);

    if (! RenderTargetResource || ! MaterialRenderProxy || ! MaterialResource)
    {
        return;
    }

    // Prepare render target texture and resolve target
    FTextureRHIParamRef TextureRTV = RenderTargetResource->GetRenderTargetTexture();
    FTextureRHIParamRef TextureRSV = RenderTargetResource->TextureRHI;

    if (! TextureRTV || ! TextureRSV)
    {
        return;
    }

    // Update deferred expression cache

    MaterialRenderProxy->UpdateUniformExpressionCacheIfNeeded(FeatureLevel);
    FMaterialRenderProxy::UpdateDeferredCachedUniformExpressions();

	// Create default render target view

	FIntRect ViewRect;
    TSharedRef<FSceneView> View(CreateDefaultRTView(RHICmdList, RenderTargetResource, ViewRect));

    // Setup viewport

    RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

    // Prepare graphics pipelane

    TShaderMapRef<FRULShaderDrawPolyVS> VSShader(GetGlobalShaderMap(FeatureLevel));

	const FMaterialShaderMap* MaterialShaderMap = MaterialResource->GetRenderingThreadShaderMap();
	FRULBaseMaterialShader* PSShader = MaterialShaderMap->GetShader<TRULShaderDrawScreenMS<1>>();

    FGraphicsPipelineStateInitializer GraphicsPSOInit;
    SetupDefaultGraphicsPSOInit(GraphicsPSOInit, PT_TriangleList, DrawConfig);
    GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSShader);
    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PSShader->GetPixelShader();

    // Render pass

    FRHIRenderPassInfo RPInfo(TextureRTV, GetRenderTargetActions(DrawConfig));
    TransitionRenderPassTargets(RHICmdList, RPInfo);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("RULShaderLibrary_DrawMaterialPoly"));
    {
        // Set graphics pipeline

        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Bind shader parameters

        SetupMaterialParameters(RHICmdList, FeatureLevel, *VSShader, PSShader, *MaterialRenderProxy, *View);

        // Draw primitives

        FRULRHIUtilityLibrary::DrawTriangleList(RHICmdList, Vertices, Indices);

        // Unbind shader parameters

        VSShader->UnbindBuffers(RHICmdList);
        PSShader->UnbindBuffers(RHICmdList);
    }
    RHICmdList.EndRenderPass();
}

void URULShaderLibrary::ApplyAutoLevels(
    UObject* WorldContextObject,
    UTexture* SourceTexture,
    UTextureRenderTarget2D* RenderTarget,
    FRULShaderDrawConfig DrawConfig,
    bool bApplyLevelMin,
    bool bApplyLevelMax,
    UGWTTickEvent* CallbackEvent
    )
{
    // No level operation specified, silent abort
    if (! bApplyLevelMin && ! bApplyLevelMax)
    {
        return;
    }

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FTextureRenderTarget2DResource* RenderTargetResource = nullptr;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyAutoLevels() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return;
    }
    else
    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyAutoLevels() ABORTED, INVALID WORLD SCENE"));
        return;
    }
    else
    if (! IsValid(SourceTexture))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyAutoLevels() ABORTED, INVALID TEXTURE"));
        return;
    }
    else
    if (! IsValid(RenderTarget))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyAutoLevels() ABORTED, INVALID RENDER TARGET"));
        return;
    }

    RenderTargetResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget->GameThread_GetRenderTargetResource());

    if (! RenderTargetResource)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::ApplyAutoLevels() ABORTED, INVALID RENDER TARGET TEXTURE RESOURCE"));
        return;
    }

    World->SendAllEndOfFrameUpdates();

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        FTexture* SourceTexture;
        FTextureRenderTarget2DResource* RenderTargetResource;
        FRULShaderDrawConfig DrawConfig;
        bool bApplyLevelMin;
        bool bApplyLevelMax;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        World->Scene->GetFeatureLevel(),
        SourceTexture->Resource,
        RenderTargetResource,
        DrawConfig,
        bApplyLevelMin,
        bApplyLevelMax,
        CallbackEvent
        };

    ENQUEUE_RENDER_COMMAND(RULShaderLibrary_ApplyAutoLevels)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::ApplyAutoLevels_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.SourceTexture,
                RenderParameter.RenderTargetResource,
                RenderParameter.DrawConfig,
                RenderParameter.bApplyLevelMin,
                RenderParameter.bApplyLevelMax
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );
}

void URULShaderLibrary::ApplyAutoLevels_RT(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    FTexture* SourceTexture,
    FTextureRenderTarget2DResource* RenderTargetResource,
    FRULShaderDrawConfig DrawConfig,
    bool bApplyLevelMin,
    bool bApplyLevelMax
    )
{
    check(IsInRenderingThread());

    if (! SourceTexture || ! RenderTargetResource)
    {
        return;
    }

    FTextureRHIParamRef TextureRTV = RenderTargetResource->GetRenderTargetTexture();
    FTextureRHIParamRef SourceTextureRHI = SourceTexture->TextureRHI;

    if (! TextureRTV || ! SourceTextureRHI)
    {
        return;
    }

    FIntVector DimensionXYZ = SourceTextureRHI->GetSizeXYZ();
    FIntPoint Dimension(DimensionXYZ.X, DimensionXYZ.Y);

    TResourceArray<FVector4, VERTEXBUFFER_ALIGNMENT> DefaultSumData(false);
    DefaultSumData.SetNumUninitialized(2);
    DefaultSumData[0] = FVector4(0.f,0.f,0.f,0.f);
    DefaultSumData[1] = FVector4(1.f,1.f,1.f,1.f);

    FRULRWBufferStructured SumData;
    SumData.Initialize(sizeof(FVector4), 2, &DefaultSumData, BUF_Static);

    int32 ScanBlockCountMin = 0;
    int32 ScanBlockCountMax = 0;

    if (bApplyLevelMin)
    {
        ScanBlockCountMin = FRULReduceScan::ReduceTexture<FRULReduceScan::SOT_Min>(
            RHICmdList,
            SourceTextureRHI,
            SumData,
            Dimension,
            0,
            false,
            BUF_Static
            );
    }

    if (bApplyLevelMax)
    {
        ScanBlockCountMax = FRULReduceScan::ReduceTexture<FRULReduceScan::SOT_Max>(
            RHICmdList,
            SourceTextureRHI,
            SumData,
            Dimension,
            1,
            false,
            BUF_Static
            );
    }

#if 0
    UE_LOG(LogTemp,Warning, TEXT("ScanBlockCountMin: %d"), ScanBlockCountMin);
    UE_LOG(LogTemp,Warning, TEXT("ScanBlockCountMax: %d"), ScanBlockCountMax);

    if (SumData.Buffer->GetStride() > 0 && (ScanBlockCountMin > 0 || ScanBlockCountMax > 0))
    {
        const int32 SumDataCount = SumData.Buffer->GetSize() / SumData.Buffer->GetStride();

        TArray<FVector4> SumArr;
        SumArr.SetNumUninitialized(SumDataCount);

        void* SumDataPtr = RHILockStructuredBuffer(SumData.Buffer, 0, SumData.Buffer->GetSize(), RLM_ReadOnly);
        FMemory::Memcpy(SumArr.GetData(), SumDataPtr, SumData.Buffer->GetSize());
        RHIUnlockStructuredBuffer(SumData.Buffer);

        UE_LOG(LogTemp,Warning, TEXT("SumDataCount: %d"), SumDataCount);

        for (int32 i=0; i<SumDataCount; ++i)
        {
            UE_LOG(LogTemp,Warning, TEXT("Scan Result [%d]: %s"), i, *SumArr[i].ToString());
        }
    }
#endif

    // Prepare graphics pipelane

    TShaderMapRef<FRULShaderDrawScreenVS> VSShader(GetGlobalShaderMap(FeatureLevel));
    TShaderMapRef<FRULShaderAutoLevelPS> PSShader(GetGlobalShaderMap(FeatureLevel));

    FGraphicsPipelineStateInitializer GraphicsPSOInit;
    SetupDefaultGraphicsPSOInit(GraphicsPSOInit, PT_TriangleStrip, DrawConfig);
    GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSShader);
    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PSShader->GetPixelShader();

    // Render pass

    FRHIRenderPassInfo RPInfo(TextureRTV, GetRenderTargetActions(DrawConfig));
    TransitionRenderPassTargets(RHICmdList, RPInfo);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("RULShaderLibrary_AutoLevel"));
    {
        // Set graphics pipeline

        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Bind shader parameters

        PSShader->BindTexture(RHICmdList, TEXT("SourceTexture"), SourceTextureRHI);
        PSShader->BindSRV(RHICmdList, TEXT("AutoLevelData"), SumData.SRV);

        // Draw primitives

        RHICmdList.SetStreamSource(0, GetFilterShaderVB(), 0);
        RHICmdList.DrawPrimitive(0, 2, 1);

        // Unbind shader parameters

        VSShader->UnbindBuffers(RHICmdList);
        PSShader->UnbindBuffers(RHICmdList);
    }
    RHICmdList.EndRenderPass();
}

FRULTextureValuesRef URULShaderLibrary::GetTextureValuesByPoints(
    UObject* WorldContextObject,
    FRULShaderTextureParameterInput SourceTexture,
    const FVector2D ScaleDimension,
    const TArray<FVector2D>& Points,
    UGWTTickEvent* CallbackEvent
    )
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    FRULShaderTextureParameterInputResource TextureResource(SourceTexture.GetResource_GT());
    FRULTextureValuesRef ValuesRef;

    if (! IsValid(World))
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::GetTextureValuesByPoints() ABORTED, INVALID WORLD CONTEXT OBJECT"));
        return ValuesRef;
    }

    if (! World->Scene)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::GetTextureValuesByPoints() ABORTED, INVALID WORLD SCENE"));
        return ValuesRef;
    }

    if (Points.Num() < 1)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::GetTextureValuesByPoints() ABORTED, EMPTY POINTS"));
        return ValuesRef;
    }

    if (ScaleDimension.X <= 0 || ScaleDimension.Y <= 0)
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::GetTextureValuesByPoints() ABORTED, INVALID SCALE SIZE"));
        return ValuesRef;
    }

    if (! TextureResource.HasValidResource())
    {
        UE_LOG(LogRUL,Warning, TEXT("URULShaderLibrary::GetTextureValuesByPoints() ABORTED, INVALID TEXTURE INPUT"));
        return ValuesRef;
    }

    ValuesRef.SharedRef = FRULTextureValuesRef::FSharedRefType(new FRULTextureValuesRef::FValuesRef);
    ValuesRef.SharedRef->Values.SetNumZeroed(Points.Num());

    struct FRenderParameter
    {
        ERHIFeatureLevel::Type FeatureLevel;
        FRULShaderTextureParameterInputResource TextureResource;
        FVector2D ScaleDimension;
        const TArray<FVector2D> Points;
        FRULTextureValuesRef::FSharedRefType ValuesRef;
        UGWTTickEvent* CallbackEvent;
    };

    FRenderParameter RenderParameter = {
        World->Scene->GetFeatureLevel(),
        TextureResource,
        ScaleDimension,
        Points,
        ValuesRef.SharedRef,
        CallbackEvent
        };

    // Enqueue render command

    ENQUEUE_RENDER_COMMAND(RULUtilityShaderLibrary_GetTextureValuesByPoints)(
        [RenderParameter](FRHICommandListImmediate& RHICmdList)
        {
            URULShaderLibrary::GetTextureValuesByPoints_RT(
                RHICmdList,
                RenderParameter.FeatureLevel,
                RenderParameter.TextureResource,
                RenderParameter.ScaleDimension,
                RenderParameter.Points,
                RenderParameter.ValuesRef
                );
            FGWTTickEventRef(RenderParameter.CallbackEvent).EnqueueCallback();
        }
    );

    return ValuesRef;
}

void URULShaderLibrary::GetTextureValuesByPoints_RT(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    FRULShaderTextureParameterInputResource TextureResource,
    const FVector2D ScaleDimension,
    const TArray<FVector2D>& Points,
    FRULTextureValuesRef::FSharedRefType ValuesRef
    )
{
    check(IsInRenderingThread());
    check(ScaleDimension.X > 0.f);
    check(ScaleDimension.Y > 0.f);
    check(ValuesRef.IsValid());

    FTexture2DRHIParamRef SourceTexture = TextureResource.GetTextureParamRef_RT();

    if (! SourceTexture)
    {
        return;
    }

    const FVector2D PointScale = FVector2D::UnitVector / ScaleDimension;
    const int32 PointCount = Points.Num();

    typedef TResourceArray<FRULAlignedVector2D, VERTEXBUFFER_ALIGNMENT> FPointData;

    FPointData PointArr(false);
    PointArr.SetNumUninitialized(PointCount);

    for (int32 i=0; i<PointCount; ++i)
    {
        PointArr[i] = Points[i];
    }
    
    FRULRWBufferStructured PointData;
    PointData.Initialize(
        sizeof(FPointData::ElementType),
        PointCount,
        &PointArr,
        BUF_Static,
        TEXT("PointData")
        );
    
    FRULRWBufferStructured ValueData;
    ValueData.Initialize(
        sizeof(FLinearColor),
        PointCount,
        BUF_Static,
        TEXT("ValueData")
        );

    RHICmdList.BeginComputePass(TEXT("GetTextureValuesByPoints"));
    {
        FSamplerStateRHIParamRef TextureSampler = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();

        TShaderMapRef<FRULShaderGetTextureValues> ComputeShader(GetGlobalShaderMap(FeatureLevel));
        ComputeShader->SetShader(RHICmdList);
        ComputeShader->BindTexture(RHICmdList, TEXT("SourceTexture"), TEXT("SourceTextureSampler"), SourceTexture, TextureSampler);
        ComputeShader->BindSRV(RHICmdList, TEXT("PointData"), PointData.SRV);
        ComputeShader->BindUAV(RHICmdList, TEXT("OutValueData"), ValueData.UAV);
        ComputeShader->SetParameter(RHICmdList, TEXT("_PointScale"), PointScale);
        ComputeShader->SetParameter(RHICmdList, TEXT("_PointCount"), PointCount);
        ComputeShader->DispatchAndClear(RHICmdList, PointCount, 1, 1);

        TArray<FLinearColor>& Values(ValuesRef->Values);

        // Resize output value count if required
        if (Values.Num() != PointCount)
        {
            Values.SetNumUninitialized(PointCount, true);
        }

        // Copy values
        void* ValueDataPtr = RHILockStructuredBuffer(ValueData.Buffer, 0, ValueData.Buffer->GetSize(), RLM_ReadOnly);
        FMemory::Memcpy(Values.GetData(), ValueDataPtr, ValueData.Buffer->GetSize());
        RHIUnlockStructuredBuffer(ValueData.Buffer);

#if 0
        for (int32 i=0; i<PointCount; ++i)
        {
            UE_LOG(LogTemp,Warning, TEXT("Values[%d]: %s"), i, *Values[i].ToString());
        }
#endif
    }
    RHICmdList.EndComputePass();
}

void URULShaderLibrary::GetTextureValuesOutput(const FRULTextureValuesRef& ValuesRef, TArray<FLinearColor>& Values)
{
    ValuesRef.GetValuesFromRef(Values);
}

void URULShaderLibrary::ClearTextureValuesOutput(FRULTextureValuesRef& ValuesRef)
{
    ValuesRef.ClearValues();
}

void URULShaderLibrary::ConvertPointsToScreenCoordinates(const TArray<FVector2D>& Points, TArray<FVector2D>& ScreenPoints, int32 DrawSizeX, int32 DrawSizeY)
{
    FBox2D DrawBounds(FVector2D::ZeroVector, FVector2D(DrawSizeX, DrawSizeY));
    FVector2D DrawScale(FVector2D::UnitVector/DrawBounds.GetExtent());
    FVector2D DrawOffset(-DrawBounds.GetCenter()*DrawScale);

    ScreenPoints.SetNumUninitialized(Points.Num());

    for (int32 i=0; i<Points.Num(); ++i)
    {
        const FVector2D& DrawPoint(Points[i]);
        ScreenPoints[i] = DrawOffset + DrawPoint*DrawScale;
    }
}

void URULShaderLibrary::ConvertPointToScreenCoordinate(FVector2D Point, FVector2D& ScreenPoint, int32 DrawSizeX, int32 DrawSizeY)
{
    FBox2D DrawBounds(FVector2D::ZeroVector, FVector2D(DrawSizeX, DrawSizeY));
    FVector2D DrawScale(FVector2D::UnitVector/DrawBounds.GetExtent());
    FVector2D DrawOffset(-DrawBounds.GetCenter()*DrawScale);

    ScreenPoint = DrawOffset + Point*DrawScale;
}
