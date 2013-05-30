#pragma once
#ifndef UTILS_SHADERTOOLS_H
#define UTILS_SHADERTOOLS_H

#include "DXUT.h"

namespace ShaderTools {
	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, D3D_SHADER_MACRO* pDefines);
}

#endif