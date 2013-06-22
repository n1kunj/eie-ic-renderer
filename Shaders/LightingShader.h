#include "DXUT.h"
#pragma once
#ifndef SHADERS_LIGHTINGSHADER_H
#define SHADERS_LIGHTINGSHADER_H
#include "../Utils/ShaderTools.h"
#include "../DirectXMath/DirectXMath.h"
#include "../Camera.h"

__declspec(align(16)) struct LightingPSCB
{
	DirectX::XMMATRIX Projection;
	DirectX::XMUINT2 bufferDim;
	FLOAT yHeight;
	FLOAT padding;
};

class LightingShader{
private:
	boolean mCompiled;
	ID3D11VertexShader* mVertexShader;
	ID3D11PixelShader* mPixelShader;
	ID3D11Buffer* mPSCB;
public:
	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mVertexShader);
		SAFE_RELEASE(mPixelShader);
		SAFE_RELEASE(mPSCB);
		mCompiled = FALSE;
	}

	LightingShader() : mCompiled(FALSE),mPixelShader(NULL),
		mVertexShader(NULL),mPSCB(NULL)	{}

	~LightingShader() {
		OnD3D11DestroyDevice();
	}

	void DrawPost(ID3D11DeviceContext* pd3dContext,ID3D11ShaderResourceView* pSRV[2], const Camera* pCamera)
	{
		if (!mCompiled) {
			return;
		}

		UINT width = DXUTGetDXGIBackBufferSurfaceDesc()->Width;
		UINT height = DXUTGetDXGIBackBufferSurfaceDesc()->Height;

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		pd3dContext->Map(mPSCB,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		LightingPSCB* pscb = (LightingPSCB*)MappedResource.pData;
		pscb->Projection = XMMatrixTranspose(pCamera->mProjectionMatrix);
		pscb->bufferDim = DirectX::XMUINT2(width,height);
		pscb->yHeight = (FLOAT)pCamera->getEyeY();
		pd3dContext->Unmap(mPSCB,0);

		pd3dContext->PSSetConstantBuffers(0,1,&mPSCB);

		pd3dContext->PSSetShaderResources(0,2,pSRV);

		pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
		pd3dContext->VSSetShader( mVertexShader, NULL, 0 );
		pd3dContext->PSSetShader( mPixelShader, NULL, 0 );
		pd3dContext->Draw( 4, 0 );

		ID3D11ShaderResourceView* srvs[2] = {NULL,NULL};
		pd3dContext->PSSetShaderResources(0,2,srvs);
	}

	HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
	{
		OnD3D11DestroyDevice();
		HRESULT hr;

		//Compile VS
		ID3DBlob* pVSBlob = NULL;
		V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\LightingShader.fx", "LightingVS", "vs_5_0", &pVSBlob ));

		//Create the vertex shader
		//If fails, releases pVSBlob.
		V_RELEASE_IF_RETURN(pVSBlob,pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &mVertexShader ));

		// Compile the pixel shader
		ID3DBlob* pPSBlob = NULL;
		V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\LightingShader.fx", "LightingPS", "ps_5_0", &pPSBlob ));

		V_RELEASE_AND_RETURN(pPSBlob, pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &mPixelShader ));

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.ByteWidth = sizeof(LightingPSCB);
		V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &mPSCB ));

		mCompiled = TRUE;
		return S_OK;
	}
};

#endif