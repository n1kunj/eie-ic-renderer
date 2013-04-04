#include "DXUT.h"
#pragma once
#ifndef SHADERS_LIGHTINGCOMPUTE_H
#define SHADERS_LIGHTINGCOMPUTE_H
#include "../Utils/ShaderTools.h"
#include "../DirectXMath/DirectXMath.h"
#include "../Camera.h"
#include "../Texture2D.h"

#define COMPUTE_SHADER_TILE_GROUP_DIM 16

__declspec(align(16)) struct LightingCSCB
{
	DirectX::XMUINT2 bufferDim;
};

struct LightingCSFB {
	UINT rg;
	UINT ba;
};

class LightingCompute{
private:
	boolean mCompiled;
	ID3D11ComputeShader* mCS;
	ID3D11Buffer* mCSCB;
public:
	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mCS);
		SAFE_RELEASE(mCSCB);
		mCompiled = FALSE;
	}

	LightingCompute() : mCompiled(FALSE),mCS(NULL),mCSCB(NULL){}

	~LightingCompute() {
		OnD3D11DestroyDevice();
	}

	void Compute(ID3D11DeviceContext* pd3dContext, ID3D11ShaderResourceView* pSRV[3], StructuredBuffer<LightingCSFB>* pSB)
	{
		if (!mCompiled) {
			return;
		}

		pd3dContext->CSSetShaderResources(0,3,pSRV);

		UINT width = DXUTGetDXGIBackBufferSurfaceDesc()->Width;
		UINT height = DXUTGetDXGIBackBufferSurfaceDesc()->Height;

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		pd3dContext->Map(mCSCB,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		LightingCSCB* cscb = (LightingCSCB*)MappedResource.pData;
		cscb->bufferDim = DirectX::XMUINT2(width,height);
		pd3dContext->Unmap(mCSCB,0);

		pd3dContext->CSSetConstantBuffers(0,1,&mCSCB);

		pd3dContext->CSSetUnorderedAccessViews(0,1,&pSB->mUAV,0);
		pd3dContext->CSSetShader(mCS,0,0);

		unsigned int dispatchWidth = (width + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
		unsigned int dispatchHeight = (height + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;

		pd3dContext->Dispatch(dispatchWidth, dispatchHeight, 1);

		ID3D11ShaderResourceView* srvs[3] = {NULL,NULL,NULL};
		pd3dContext->CSSetShaderResources(0,3,srvs);
		ID3D11UnorderedAccessView* uavs[1] = {NULL};
		pd3dContext->CSSetUnorderedAccessViews(0,1,uavs,0);
	}

	HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
	{
		OnD3D11DestroyDevice();
		HRESULT hr;

		//Compile CS
		ID3DBlob* pCSBlob = NULL;
		V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\LightingCompute.fx", "LightingCS", "cs_5_0", &pCSBlob ));

		//Create the compute shader
		//If fails, releases pVSBlob.
		V_RELEASE_IF_RETURN(pCSBlob,pd3dDevice->CreateComputeShader( pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), NULL, &mCS ));

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.ByteWidth = sizeof(LightingCSCB);
		V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &mCSCB ));

		mCompiled = TRUE;
		return S_OK;
	}
};

#endif