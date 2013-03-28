#include "DXUT.h"
#include "DeferredTileComputeShader.h"

//TODO: EVERYTHING EVER

void DeferredTileComputeShader::DrawPost( ID3D11DeviceContext* pd3dDeviceContext )
{

}

HRESULT DeferredTileComputeShader::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	return S_OK;
}

void DeferredTileComputeShader::OnD3D11DestroyDevice()
{

}

DeferredTileComputeShader::DeferredTileComputeShader() : PostShader(L"DeferredTileComputeShader")
{

}

DeferredTileComputeShader::~DeferredTileComputeShader()
{
}