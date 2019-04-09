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
#include "ShaderParameterMacros.h"

struct FRULUintPoint
{
    uint32 X;
    uint32 Y;
};

// 1D Data
typedef TShaderParameterTypeInfo<uint32>::TAlignedType      FRULAlignedUint;

// 2D Data
typedef TShaderParameterTypeInfo<FVector2D>::TAlignedType   FRULAlignedVector2D;
typedef TAlignedTypedef<FRULUintPoint,8>::Type              FRULAlignedUintPoint;

// 4D Data
typedef TShaderParameterTypeInfo<FIntVector4>::TAlignedType FRULAlignedIntVector4;
typedef TAlignedTypedef<FUintVector4,16>::Type              FRULAlignedUintVector4;

