#pragma once
#ifndef SHADERS_DISTANTGBUFFERSHADER_H
#define SHADERS_DISTANTGBUFFERSHADER_H
#include "DXUT.h"
#include "../DrawableShader.h"
#include "../DirectXMath/DirectXMath.h"
#include "../DrawableMesh.h"
#include "../DrawableState.h"
#include "../Camera.h"
#include "../Utils/ShaderTools.h"
#include "../Procedural/Generator.h"

__declspec(align(16)) struct DistantGBufferVSCB
{
	DirectX::XMMATRIX Model;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX VP;
	DirectX::XMMATRIX MV;
	DirectX::XMMATRIX MVP;
	DirectX::XMINT3 Offset;
	FLOAT tessAmount;
};

class DistantGBufferShader : public DrawableShader {
private:
	boolean mCompiled;
	ID3D11InputLayout* mVertexLayout;
	ID3D11VertexShader* mVertexShader;
	ID3D11HullShader* mHullShader;
	ID3D11DomainShader* mDomainShader;
	ID3D11PixelShader* mPixelShader;
	ID3D11Buffer* mDSConstantBuffer;
	ID3D11SamplerState* mDefaultSampler;
	ID3D11SamplerState* mAnisotropicSampler;
	FLOAT* mTessAmount;

public:
	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mVertexShader);
		SAFE_RELEASE(mPixelShader);
		SAFE_RELEASE(mVertexLayout);
		SAFE_RELEASE(mHullShader);
		SAFE_RELEASE(mDomainShader);
		SAFE_RELEASE(mDSConstantBuffer);
		SAFE_RELEASE(mDefaultSampler);
		SAFE_RELEASE(mAnisotropicSampler);
		mCompiled = FALSE;
	}

	DistantGBufferShader(FLOAT* pTessAmount) : DrawableShader(L"DistantGBufferShader"),mCompiled(FALSE),mVertexLayout(NULL),
		mVertexShader(NULL),mPixelShader(NULL),mDSConstantBuffer(NULL),mHullShader(NULL), mDomainShader(NULL), mDefaultSampler(NULL), mAnisotropicSampler(NULL), mTessAmount(pTessAmount) {}

	~DistantGBufferShader()
	{
		OnD3D11DestroyDevice();
	}

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice) {
		//Make sure everything is clean before we start
		OnD3D11DestroyDevice();

		HRESULT hr;

		//Compile VS
		{
			ID3DBlob* pVSBlob = NULL;
			V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\DistantGBufferShader.fx", "VS", "vs_5_0", &pVSBlob ));

			//Create the vertex shader
			//If fails, releases pVSBlob.
			V_RELEASE_IF_RETURN(pVSBlob,pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &mVertexShader ));

			//Create the input layout
			V_RELEASE_AND_RETURN(pVSBlob,pd3dDevice->CreateInputLayout( VertexLayout, numLayoutElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &mVertexLayout ));
		}

		// Compile the hull shader
		{
			ID3DBlob* blob = NULL;
			V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\DistantGBufferShader.fx", "HS", "hs_5_0", &blob ));

			V_RELEASE_AND_RETURN(blob, pd3dDevice->CreateHullShader( blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &mHullShader ));
		}

		// Compile the domain shader
		{
			ID3DBlob* blob = NULL;
			V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\DistantGBufferShader.fx", "DS", "ds_5_0", &blob ));

			V_RELEASE_AND_RETURN(blob, pd3dDevice->CreateDomainShader( blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &mDomainShader ));
		}

		// Compile the pixel shader
		{
			ID3DBlob* blob = NULL;
			V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\DistantGBufferShader.fx", "PS", "ps_5_0", &blob ));

			V_RELEASE_AND_RETURN(blob, pd3dDevice->CreatePixelShader( blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &mPixelShader ));
		}

		// Create the constant buffer
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory( &bd, sizeof(bd) );
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.ByteWidth = sizeof(DistantGBufferVSCB);
			V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &mDSConstantBuffer ));
		}

		//Create the sampler state
		{
			CD3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
			pd3dDevice->CreateSamplerState(&desc,&mDefaultSampler);

			desc.Filter = D3D11_FILTER_ANISOTROPIC;
			desc.MaxAnisotropy = 8;
			pd3dDevice->CreateSamplerState(&desc,&mAnisotropicSampler);
		}

		mCompiled = TRUE;
		return S_OK;
	}

	void DrawMesh(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, const DrawableState* pState, const Camera* pCamera) {
		assert(pMesh!= NULL);
		assert(pMesh->mInitialised == TRUE);
		assert(pState != NULL);
		assert(pCamera != NULL);

		if (!mCompiled) {
			return;
		}

		setupShader(pd3dContext,pMesh,pState,pCamera);

		pd3dContext->DrawIndexed( pMesh->mNumIndices, 0, 0 );

		cleanupShader(pd3dContext);
	}

private:
	void setupShader(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, const DrawableState* pState, const Camera* pCamera) {
		using namespace DirectX;

		// Update constant buffer
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		pd3dContext->Map(mDSConstantBuffer,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		DistantGBufferVSCB* vscb = (DistantGBufferVSCB*)MappedResource.pData;
		vscb->Model = XMMatrixTranspose(pState->mModelMatrix);
		vscb->View = XMMatrixTranspose(pCamera->mViewMatrix);
		vscb->VP = XMMatrixTranspose(pCamera->mViewProjectionMatrix);
		vscb->MV = XMMatrixMultiplyTranspose(pState->mModelMatrix,pCamera->mViewMatrix);
		vscb->MVP = XMMatrixMultiplyTranspose(pState->mModelMatrix,pCamera->mViewProjectionMatrix);
		const XMINT3& mCrds = pState->mCoords;
		const XMINT3& cCrds = pCamera->mCoords;
		vscb->Offset = XMINT3(mCrds.x - cCrds.x,
			mCrds.y - cCrds.y,
			mCrds.z - cCrds.z);
		vscb->tessAmount = *mTessAmount;
		pd3dContext->Unmap(mDSConstantBuffer,0);

		pd3dContext->HSSetConstantBuffers( 0, 1, &mDSConstantBuffer );
		pd3dContext->DSSetConstantBuffers( 0, 1, &mDSConstantBuffer );
		pd3dContext->PSSetConstantBuffers( 0, 1, &mDSConstantBuffer );
		pd3dContext->PSSetSamplers(0,1,&mDefaultSampler);
		pd3dContext->PSSetSamplers(1,1,&mAnisotropicSampler);
		pd3dContext->DSSetSamplers(0,1,&mDefaultSampler);

		//Set vertex layout and bind buffers
		pd3dContext->IASetInputLayout( mVertexLayout );

		UINT offset = 0;
		pd3dContext->IASetVertexBuffers( 0, 1, &pMesh->mVertexBuffer, &VertexDataStride, &offset );

		pd3dContext->IASetIndexBuffer( pMesh->mIndexBuffer, pMesh->mIndexBufferFormat, 0 );
		pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST );

		pd3dContext->DSSetShaderResources(0,1,&pState->mDistantTile->mHeightMap.mSRV);

		ID3D11ShaderResourceView* pssrvs[2] = {pState->mDistantTile->mAlbedoMap.mSRV,pState->mDistantTile->mNormalMap.mSRV};

		pd3dContext->PSSetShaderResources(0,2,pssrvs);
		//Set shaders
		pd3dContext->VSSetShader( mVertexShader, NULL, 0 );
		pd3dContext->HSSetShader( mHullShader, NULL, 0 );
		pd3dContext->DSSetShader( mDomainShader, NULL, 0 );
		pd3dContext->PSSetShader( mPixelShader, NULL, 0 );
	}

	void cleanupShader(ID3D11DeviceContext* pd3dContext) {
		pd3dContext->HSSetShader(NULL,NULL,0);
		pd3dContext->DSSetShader(NULL,NULL,0);
		ID3D11ShaderResourceView* nullsrvs[2] = {NULL,NULL};
		pd3dContext->PSSetShaderResources(0,2,nullsrvs);
		pd3dContext->DSSetShaderResources(0,1,nullsrvs);
	}
};

#endif