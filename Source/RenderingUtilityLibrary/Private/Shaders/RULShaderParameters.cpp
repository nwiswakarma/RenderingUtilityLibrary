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

#include "Shaders/RULShaderParameters.h"

#include "TextureResource.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"

bool FRULShaderTextureParameterInputResource::HasValidResource() const
{
    switch (TextureType)
    {
        case ERULShaderTextureType::RUL_STT_Texture2D:
            return Texture2DResource != nullptr;

        case ERULShaderTextureType::RUL_STT_TextureRenderTarget2D:
            return TextureRenderTarget2DResource != nullptr;
    }
    return false;
}

FTexture2DRHIParamRef FRULShaderTextureParameterInputResource::GetTextureParamRef_RT() const
{
    check(IsInRenderingThread());

    FTexture2DRHIParamRef ParamRef = nullptr;

    switch (TextureType)
    {
        case ERULShaderTextureType::RUL_STT_Texture2D:
            if (Texture2DResource)
            {
                ParamRef = Texture2DResource->GetTexture2DRHI();
            }
            break;

        case ERULShaderTextureType::RUL_STT_TextureRenderTarget2D:
            if (TextureRenderTarget2DResource)
            {
                ParamRef = TextureRenderTarget2DResource->GetTextureRHI();
            }
            break;
    }

    return ParamRef;
}

FRULShaderTextureParameterInputResource FRULShaderTextureParameterInput::GetResource_GT() const
{
    FRULShaderTextureParameterInputResource Resource;
    Resource.TextureType = TextureType;

    switch (TextureType)
    {
        case ERULShaderTextureType::RUL_STT_Texture2D:
            if (IsValid(Texture2D))
            {
                Resource.Texture2DResource = static_cast<FTexture2DResource*>(Texture2D->Resource);
            }
            break;

        case ERULShaderTextureType::RUL_STT_TextureRenderTarget2D:
            if (IsValid(TextureRenderTarget2D))
            {
                Resource.TextureRenderTarget2DResource = static_cast<FTextureRenderTarget2DResource*>(TextureRenderTarget2D->GameThread_GetRenderTargetResource());
            }
            break;
    }

    return Resource;
}
