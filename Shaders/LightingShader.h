#include "DXUT.h"
#pragma once
#ifndef SHADERS_LIGHTINGSHADER_H
#define SHADERS_LIGHTINGSHADER_H
#include "../Utils/ShaderTools.h"
#include "../DirectXMath/DirectXMath.h"

class LightingShader{
private:
	boolean mCompiled;
	ID3D11VertexShader* mVertexShader;
	ID3D11PixelShader* mPixelShader;
public:
	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mVertexShader);
		SAFE_RELEASE(mPixelShader);
		mCompiled = FALSE;
	}

	LightingShader() : mCompiled(FALSE),mPixelShader(NULL),
		mVertexShader(NULL)	{}

	~LightingShader() {
		OnD3D11DestroyDevice();
	}

	void DrawPost(ID3D11DeviceContext* pd3dContext,ID3D11ShaderResourceView* pSRV[3])
	{
		if (!mCompiled) {
			return;
		}

		pd3dContext->PSSetShaderResources(0,3,pSRV);

		pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
		pd3dContext->VSSetShader( mVertexShader, NULL, 0 );
		pd3dContext->PSSetShader( mPixelShader, NULL, 0 );
		pd3dContext->Draw( 4, 0 );

		ID3D11ShaderResourceView* srvs[3] = {NULL,NULL,NULL};
		pd3dContext->PSSetShaderResources(0,3,srvs);
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

		mCompiled = TRUE;
		return S_OK;
	}
};

#endif