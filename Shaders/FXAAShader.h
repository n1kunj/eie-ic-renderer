#include "DXUT.h"
#pragma once
#ifndef SHADERS_FXAASHADER_H
#define SHADERS_FXAASHADER_H
#include "../Utils/ShaderTools.h"
#include "../DirectXMath/DirectXMath.h"

struct FXAAConstantBuffer
{
	DirectX::XMVECTOR FXAA;
};

class FXAAShader{
private:
	boolean mCompiled;
	ID3D11VertexShader* mVertexShader;
	ID3D11PixelShader* mPixelShader;
	ID3D11Buffer* mConstantBuffer;
	ID3D11SamplerState* mSamAni;

public:
	void OnD3D11DestroyDevice()
	{
		SAFE_RELEASE(mVertexShader);
		SAFE_RELEASE(mPixelShader);
		SAFE_RELEASE(mConstantBuffer);
		SAFE_RELEASE(mSamAni);
		mCompiled = FALSE;
	}

	FXAAShader() : mCompiled(FALSE),mVertexShader(NULL),
	mPixelShader(NULL),mConstantBuffer(NULL),mSamAni(NULL){}

	~FXAAShader(){
		OnD3D11DestroyDevice();
	}

	void DrawPost(ID3D11DeviceContext* pd3dContext, ID3D11ShaderResourceView* pInputSRV);
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
};

void FXAAShader::DrawPost(ID3D11DeviceContext* pd3dContext, ID3D11ShaderResourceView* pInputSRV)
{
	if (!mCompiled) {
		return;
	}

	using namespace DirectX;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dContext->Map(mConstantBuffer,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	FXAAConstantBuffer* pFXAA = (FXAAConstantBuffer*)MappedResource.pData;
	float frameWidth = (float)DXUTGetDXGIBackBufferSurfaceDesc()->Width;
	float frameHeight = (float)DXUTGetDXGIBackBufferSurfaceDesc()->Height;

	pFXAA->FXAA = XMVectorSet(1.0f/frameWidth, 1.0f/frameHeight, 0.0f, 0.0f);
	pd3dContext->Unmap( mConstantBuffer, 0 );
	pd3dContext->VSSetConstantBuffers( 1, 1, &mConstantBuffer );
	pd3dContext->PSSetConstantBuffers( 1, 1, &mConstantBuffer );

	pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	pd3dContext->VSSetShader( mVertexShader, NULL, 0 );
	pd3dContext->PSSetShader( mPixelShader, NULL, 0 );

	pd3dContext->PSSetShaderResources( 0, 1, &pInputSRV );
	pd3dContext->PSSetSamplers( 0, 1, &mSamAni );
	pd3dContext->Draw( 4, 0 );

	//Make the runtime happy
	ID3D11ShaderResourceView* srv = NULL;
	ID3D11SamplerState* sam = NULL;
	pd3dContext->PSSetShaderResources( 0, 1, &srv );
	pd3dContext->PSSetSamplers( 0, 1, &sam );
}

HRESULT FXAAShader::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	OnD3D11DestroyDevice();
	HRESULT hr;

	//Compile VS
	ID3DBlob* pVSBlob = NULL;
	V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\FXAAShader.hlsl", "FxaaVS", "vs_5_0", &pVSBlob ));

	//Create the vertex shader
	//If fails, releases pVSBlob.
	V_RELEASE_IF_RETURN(pVSBlob,pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &mVertexShader ));

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\FXAAShader.hlsl", "FxaaPS", "ps_5_0", &pPSBlob ));

	V_RELEASE_AND_RETURN(pPSBlob, pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &mPixelShader ));

	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory( &cbDesc, sizeof(cbDesc) );
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof( FXAAConstantBuffer );

	V_RETURN( pd3dDevice->CreateBuffer( &cbDesc, NULL, &mConstantBuffer ) );

	{
		D3D11_SAMPLER_DESC desc;
		::ZeroMemory( &desc, sizeof(desc) );
		desc.MipLODBias = 0.0f;
		desc.Filter = D3D11_FILTER_ANISOTROPIC;
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.MaxAnisotropy = 4;
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		desc.MaxLOD = 0.0f;
		desc.MinLOD = 0.0f;
		V_RETURN( pd3dDevice->CreateSamplerState( &desc, &mSamAni));
	}

	mCompiled = TRUE;
	return S_OK;
}

#endif